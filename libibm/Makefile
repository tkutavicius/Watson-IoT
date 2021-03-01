include $(TOPDIR)/rules.mk

PKG_NAME:=libibm
PKG_RELEASE:=1
PKG_VERSION:=1.0.0
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ibm-watson-iot/iot-c
PKG_SOURCE_VERSION:=afec329eda0caa917a0f4fd5262de17aa3f00996

include $(INCLUDE_DIR)/package.mk

define Package/libibm
	CATEGORY:=Base system
	TITLE:=libibm
	DEPENDS:=+libopenssl +libpthread
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/paho.mqtt.c/build/output/*.so* $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/paho.mqtt.c/build/output/paho_c_version $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/paho.mqtt.c/src/*.h $(1)/usr/include/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/*.so* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/iotp_version.h $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/wiotp/sdk/*.h* $(1)/usr/include/

endef

define Package/libibm/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/paho.mqtt.c/build/output/*.so* $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/paho.mqtt.c/build/output/paho_c_version $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/*.so* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/iotp_version.h $(1)/usr/lib
endef

$(eval $(call BuildPackage,libibm))