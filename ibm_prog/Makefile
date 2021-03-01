include $(TOPDIR)/rules.mk

PKG_NAME:=ibm_prog
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/ibm_prog
	CATEGORY:=Base system
	TITLE:=ibm_prog
	DEPENDS:=+libibm +libuci +libubus +libubox +libblobmsg-json
endef

define Package/ibm_prog/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ibm_prog $(1)/usr/bin
	$(INSTALL_BIN) ./files/ibm_prog.init $(1)/etc/init.d/ibm_prog
	$(INSTALL_CONF) ./files/ibm_cloud.config $(1)/etc/config/ibm_cloud
endef

$(eval $(call BuildPackage,ibm_prog))