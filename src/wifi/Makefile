#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ifeq ($(CFG_WIFI_SUPPORT_RTL8812AU),y)
objects += rtl8812au_monitor
MODULE_PATH += rtl8812au_monitor/rtl8812au_monitor.ko
endif

ifeq ($(CFG_WIFI_SUPPORT_RTL8812CU),y)
objects += rtl8812cu_monitor
MODULE_PATH += rtl8812cu_monitor/rtl8812cu_monitor.ko
endif

ifeq ($(CFG_WIFI_SUPPORT_RTL8812EU),y)
objects += rtl8812eu_monitor
MODULE_PATH += rtl8812eu_monitor/rtl8812eu_monitor.ko
endif

objects_clean := $(addsuffix _clean,$(objects))

INSTALL_DIR := $(INSTALL_DIR)/vtx/kmod

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean

all: $(objects)
	@test -d $(INSTALL_DIR) || mkdir -p $(INSTALL_DIR)
	@cp -v $(MODULE_PATH) $(INSTALL_DIR)/

clean: $(objects_clean)

install: all

uninstall: 
	@rm -rf $(INSTALL_DIR)

#==================================================================
#                          modules
#==================================================================
.PHONY: $(objects) $(objects_clean) $(MODULE)

$(objects):
	sed -i "s/CONFIG_PLATFORM_I386_PC = y/CONFIG_PLATFORM_I386_PC = n/g" $@/Makefile && \
	sed -i "s/CONFIG_PLATFORM_ARM_RK1106 = n/CONFIG_PLATFORM_ARM_RK1106 = y/g" $@/Makefile && \
	make -C $@ ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) KSRC=$(KDIR) USER_MODULE_NAME=$@ -j16

$(objects_clean):
	make -C $(patsubst %_clean,%,$@) clean
	sed -i "s/CONFIG_PLATFORM_I386_PC = n/CONFIG_PLATFORM_I386_PC = y/g" $(patsubst %_clean,%,$@)/Makefile && \
	sed -i "s/CONFIG_PLATFORM_ARM_RK1106 = y/CONFIG_PLATFORM_ARM_RK1106 = n/g" $(patsubst %_clean,%,$@)/Makefile
