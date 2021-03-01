module("luci.controller.ibm_cloud_controller", package.seeall)

function index()
    entry({"admin", "services", "ibm_cloud_model"}, cbi("ibm_cloud_model"), _("IBM Cloud"),105)
end