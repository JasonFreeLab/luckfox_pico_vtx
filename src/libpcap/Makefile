#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#===============================================================================
#   variable
#===============================================================================
BUILD_DIR := $(OUT_DIR)/src/libpcap
LOCAL_PREFIX := $(BUILD_DIR)/_install
LIBNL_DIR := $(OUT_DIR)/src/libnl/_install

PKG_CONFIG_PATH := $(LIBNL_DIR)/lib/pkgconfig

INSTALL_DIR := $(INSTALL_DIR)/usr/lib

HOST := $(HOST)
CROSS_COMPILE := $(HOST)-

MODULE_SRC := libpcap-1.10.5
MODULE := libpcap.so.1.10.5
MODULE_PATH := $(LOCAL_PREFIX)/lib/libpcap.so.1.10.5

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
	@rm -rf $(BUILD_DIR)/../tcpdump/$(MODULE_SRC)
	@rm -rf $(BUILD_DIR)
	@rm -rf $(LOCAL_PREFIX)

install: all

uninstall: $(UNINSTALL_MODULES)

#==================================================================
#                          modules LDFLAGS=-static 
#==================================================================
%_build: $(BUILD_DIR)
	@echo -e "\033[31m""Build $(patsubst %_build,%,$@) ...""\033[00m"
	@test -d $(BUILD_DIR)/$(patsubst %_build,%,$@) \
		|| tar Jxvf $(patsubst %_build,%,$@).tar.xz -C $(BUILD_DIR)
	cd $(BUILD_DIR)/$(patsubst %_build,%,$@) && \
	export PKG_CONFIG_PATH="$(PKG_CONFIG_PATH):$$PKG_CONFIG_PATH" && \
	./configure \
		--host=$(HOST) \
		--prefix=$(LOCAL_PREFIX) \
		--with-pcap=linux && \
	make -j16 && \
	make install && \
	cd -
#	@test -d $(BUILD_DIR)/../tcpdump || mkdir $(BUILD_DIR)/../tcpdump
#	@test -d $(BUILD_DIR)/../tcpdump/$(patsubst %_build,%,$@) \
		|| ln -s $(BUILD_DIR)/$(patsubst %_build,%,$@) $(BUILD_DIR)/../tcpdump/$(patsubst %_build,%,$@)

$(BUILD_DIR):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

%_uninstall:
	@rm -f $(INSTALL_DIR)/$(patsubst %_uninstall,%,$@)