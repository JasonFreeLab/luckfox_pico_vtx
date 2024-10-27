#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BUILD_DIR := $(OUT_DIR)/src/libnl
LOCAL_PREFIX :=  $(BUILD_DIR)/_install

INSTALL_DIR := $(INSTALL_DIR)/usr/lib

HOST := $(HOST)
CROSS_COMPILE := $(HOST)-

MODULE_SRC := libnl-3.5.0
MODULE := libnl-3.so.200.26.0 \
	libnl-3.so.200 \
	libnl-genl-3.so.200.26.0 \
	libnl-genl-3.so.200 \
	libnl-route-3.so.200.26.0 \
	libnl-route-3.so.200
MODULE_PATH := $(BUILD_DIR)/$(MODULE_SRC)/lib/.libs/libnl-3.so.200.26.0 \
	$(BUILD_DIR)/$(MODULE_SRC)/lib/.libs/libnl-3.so.200 \
	$(BUILD_DIR)/$(MODULE_SRC)/lib/.libs/libnl-genl-3.so.200.26.0 \
	$(BUILD_DIR)/$(MODULE_SRC)/lib/.libs/libnl-genl-3.so.200 \
	$(BUILD_DIR)/$(MODULE_SRC)/lib/.libs/libnl-route-3.so.200.26.0 \
	$(BUILD_DIR)/$(MODULE_SRC)/lib/.libs/libnl-route-3.so.200

BUILD_MODULES = $(patsubst %,%_build,${MODULE_SRC})
UNINSTALL_MODULES = $(patsubst %,%_uninstall,${MODULE})

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean install uninstall
all: $(BUILD_MODULES)
	@test -d $(INSTALL_DIR) || mkdir -p $(INSTALL_DIR)
	@cp -v $(MODULE_PATH) $(INSTALL_DIR)/

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(LOCAL_PREFIX)

install: all

uninstall: $(UNINSTALL_MODULES)

#==================================================================
#                          modules
#==================================================================
%_build: $(BUILD_DIR)
	@echo -e "\033[31m""Build $(patsubst %_build,%,$@) ...""\033[00m"
	@test -d $(BUILD_DIR)/$(patsubst %_build,%,$@) \
		|| tar -zxf $(patsubst %_build,%,$@).tar.gz -C $(BUILD_DIR)
	cd $(BUILD_DIR)/$(patsubst %_build,%,$@) && \
	./configure \
		--host=$(HOST) \
		--prefix=$(LOCAL_PREFIX) && \
	make -j16 && \
	make install && \
	cd -

$(BUILD_DIR):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

%_uninstall:
	@rm -f $(INSTALL_DIR)/$(patsubst %_uninstall,%,$@)
