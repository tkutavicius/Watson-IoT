map = Map("ibm_cloud")

section = map:section(NamedSection, "cloud_sct", "ibm_cloud", "General")
flag = section:option(Flag, "enable", "Enable", "Enable program")
orgID = section:option(Value, "orgID", "Organization ID")
orgID.datatype = "rangelength(1,6)"
typeID = section:option(Value, "typeID", "Device Type")
typeID.datatype = "minlength(1)"
deviceID = section:option(Value, "deviceID", "Device ID")
deviceID.datatype = "minlength(1)"
authToken = section:option(Value, "authToken", "Authorization Token")
authToken.datatype = "rangelength(1,18)"

return map