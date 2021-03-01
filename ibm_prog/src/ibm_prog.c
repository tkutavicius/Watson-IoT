#include <stdio.h>
#include <signal.h>
#include <iotp_device.h>
#include <uci.h>
#include <libubox/blobmsg_json.h>
#include "libubus.h"

volatile sig_atomic_t deamonize = 1;
int rc = 0;
struct uci_context *c;
struct ubus_context *ctx;
IoTPDevice *device = NULL;
IoTPConfig *config = NULL;

void term_proc(int sigterm)
{
    deamonize = 0;
}

enum
{
    FREE_MEMORY,
    __MEMORY_MAX,
};

enum
{
    MEMORY_DATA,
    __INFO_MAX,
};

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
    [FREE_MEMORY] = {.name = "free", .type = BLOBMSG_TYPE_INT64},
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
    [MEMORY_DATA] = {.name = "memory", .type = BLOBMSG_TYPE_TABLE},
};

static void data_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *tb[__INFO_MAX];
    struct blob_attr *memory[__MEMORY_MAX];
    uint64_t *load = (uint64_t *)req->priv;

    blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

    if (!tb[MEMORY_DATA])
    {
        fprintf(stderr, "Empty memory data from ubus parser!\n");
        rc = -1;
        return;
    }

    blobmsg_parse(memory_policy, __MEMORY_MAX, memory,
                  blobmsg_data(tb[MEMORY_DATA]), blobmsg_data_len(tb[MEMORY_DATA]));

    *load = blobmsg_get_u64(memory[FREE_MEMORY]);
}

char *uci_show_value(struct uci_option *o)
{
    struct uci_element *e;

    switch (o->type)
    {
    case UCI_TYPE_STRING:
        return (o->v.string);
        break;
    default:
        return "<unknown>";
        break;
    }
}

char *show_config_entry(char *path)
{
    struct uci_ptr ptr;

    c = uci_alloc_context();
    if (uci_lookup_ptr(c, &ptr, path, true) != UCI_OK)
    {
        uci_perror(c, "get_config_entry Error");
        return "1";
    }

    return (uci_show_value(ptr.o));
}

int connectDevice()
{
    char path[] = "ibm_cloud.cloud_sct.orgID";
    const char *orgID = show_config_entry(path);
    strcpy(path, "ibm_cloud.cloud_sct.typeID");
    const char *typeID = show_config_entry(path);
    strcpy(path, "ibm_cloud.cloud_sct.deviceID");
    const char *devID = show_config_entry(path);
    strcpy(path, "ibm_cloud.cloud_sct.authToken");
    const char *authToken = show_config_entry(path);
    uci_free_context(c);

    rc = IoTPConfig_setLogHandler(IoTPLog_FileDescriptor, stdout);
    if (rc != 0)
    {
        fprintf(stderr, "Failed to set IoTP Client log handler: rc=%d\n", rc);
        return rc;
    }

    rc = IoTPConfig_create(&config, NULL);
    if (rc != 0)
    {
        fprintf(stderr, "Failed to initialize configuration: rc=%d\n", rc);
        return rc;
    }

    IoTPConfig_setProperty(config, "identity.orgId", orgID);
    IoTPConfig_setProperty(config, "identity.typeId", typeID);
    IoTPConfig_setProperty(config, "identity.deviceId", devID);
    IoTPConfig_setProperty(config, "auth.token", authToken);

    rc = IoTPDevice_create(&device, config);
    if (rc != 0)
    {
        fprintf(stderr, "Failed to configure IoTP device: rc=%d\n", rc);
        return rc;
    }

    rc = IoTPDevice_setMQTTLogHandler(device, NULL);
    if (rc != 0)
    {
        fprintf(stderr, "Failed to set MQTT Trace handler: rc=%d\n", rc);
        return rc;
    }

    rc = IoTPDevice_connect(device);
    if (rc != 0)
    {
        fprintf(stderr, "Failed to connect to Watson IoT Platform: rc=%d\n", rc);
        fprintf(stderr, "Returned error reason: %s\n", IOTPRC_toString(rc));
        return rc;
    }

    return rc;
}

int requestMemory(struct ubus_context *ctx, uint64_t *load)
{
    uint32_t id;
    static struct blob_buf b;
    *load = -1;
    if (ubus_lookup_id(ctx, "system", &id))
    {
        fprintf(stderr, "Failed to lookup file\n");
        return -1;
    }
    blob_buf_init(&b, 0);
    if (ubus_invoke(ctx, id, "info", b.head, data_cb, load, 5000))
    {
        fprintf(stderr, "Cannot request section\n");
        return -1;
    }
    if (*load < 0)
    {
        fprintf(stderr, "Failed to get load data\n");
        return -1;
    }
    blob_buf_free(&b);
    return 0;
}

int main(int argc, char *argv[])
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term_proc;
    sigaction(SIGTERM, &action, NULL);

    uint64_t load = -1;

    if (connectDevice(&device, &config) != 0)
    {
        fprintf(stderr, "Failed to connect device!\n");
        goto failed_connect_cloud;
    }
    else
    {
        fprintf(stdout, "Device connected to cloud!\n");
    }

    ctx = ubus_connect(NULL);
    if (!ctx)
    {
        fprintf(stderr, "Failed to connect to ubus\n");
        goto failed_connect_ubus;
    }

    while (deamonize)
    {
        if (requestMemory(ctx, &load) == 0)
        {
            char data[50];
            snprintf(data, 50, "{\"d\" : {\"SensorID\": \"freeRAM\", \"Value\": %.2f}}", (float)load / 1000000);

            rc = IoTPDevice_sendEvent(device, "status", data, "json", QoS0, NULL);
            if (rc != IOTPRC_SUCCESS)
            {
                fprintf(stderr, "Failed to publish data. rc=%d\n", rc);
            }
        }
        sleep(10);
    }

    failed_connect_cloud:
        rc = IoTPDevice_disconnect(device);
        if (rc == IOTPRC_SUCCESS)
        {
            rc = IoTPDevice_destroy(device);
            if (rc == IOTPRC_SUCCESS)
            {
                rc = IoTPConfig_clear(config);
            }
        }
        if (rc != IOTPRC_SUCCESS)
        {
            fprintf(stderr, "Failed to disconnect or clean handles. rc=%d reason:%s\n",
                    rc, IOTPRC_toString(rc));
        }

    failed_connect_ubus:
        ubus_free(ctx);

    return rc;
}