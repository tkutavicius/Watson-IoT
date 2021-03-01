BIN:=ibm_prog
CFLAGS:=-liotp-as-device -luci -lubus -lubox -lblobmsg_json

.PHONY: all clean

all: $(BIN)

$(BIN): ibm_prog.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(BIN)
