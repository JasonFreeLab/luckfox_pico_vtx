#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#===============================================================================
#   variable
#===============================================================================
BUILD_DIR := $(OUT_DIR)/src/wfb-ng
LIBSODIUM_DIR := $(OUT_DIR)/src/libsodium/_install
LIBNL_DIR := $(OUT_DIR)/src/libnl/_install
LIBPCAP_DIR := $(OUT_DIR)/src/libpcap/_install

INSTALL_DIR := $(INSTALL_DIR)/usr/bin

MODULE_SRC := wfb-ng-wfb-ng-24.08.2
MODULE := wfb_rx wfb_tx wfb_keygen wfb_tx_cmd
MODULE_PATH := $(BUILD_DIR)/$(MODULE_SRC)/wfb_rx \
	$(BUILD_DIR)/$(MODULE_SRC)/wfb_rx \
	$(BUILD_DIR)/$(MODULE_SRC)/wfb_tx \
	$(BUILD_DIR)/$(MODULE_SRC)/wfb_keygen \
	$(BUILD_DIR)/$(MODULE_SRC)/wfb_tx_cmd

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

install: all

uninstall: $(UNINSTALL_MODULES)

#==================================================================
#                          modules LDFLAGS=-static 
#==================================================================
%_build: $(BUILD_DIR)
	@echo -e "\033[31m""Build $(patsubst %_build,%,$@) ...""\033[00m"
	@test -d $(BUILD_DIR)/$(patsubst %_build,%,$@) \
		|| tar -zxf $(patsubst %_build,%,$@).tar.gz -C $(BUILD_DIR) \
		&& cp wfb-ng-wfb-ng-24.08.2.patch $(BUILD_DIR)
	cd $(BUILD_DIR)/$(patsubst %_build,%,$@) && \
	patch -Np1 -i ../wfb-ng-wfb-ng-24.08.2.patch && \
	make all_bin -j16 \
		ARCH=$(CFG_HI_CPU_ARCH) \
		CC=$(CMAKE_C_COMPILER) \
		CXX=$(CMAKE_CXX_COMPILER) \
		CFLAGS+="-I$(LIBNL_DIR)/include/libnl3 \
			-I$(LIBPCAP_DIR)/include \
			-I$(LIBSODIUM_DIR)/include" \
		LDFLAGS+="-L$(LIBNL_DIR)/lib -lnl-genl-3 -lnl-3 \
			-L$(LIBPCAP_DIR)/lib \
			-L$(LIBSODIUM_DIR)/lib" && \
	cd -

$(BUILD_DIR):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

%_uninstall:
	@rm -f $(INSTALL_DIR)/$(patsubst %_uninstall,%,$@)