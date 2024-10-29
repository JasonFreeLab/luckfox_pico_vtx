#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
objects := rtl8812cu_monitor
objects_clean := $(addsuffix _clean,$(objects))

INSTALL_DIR := $(INSTALL_DIR)/usr/kmod

MODULE_PATH := rtl8812cu_monitor/8812cu.ko

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
.PHONY: $(objects) $(objects_clean)

$(objects):
	sed -i "s/CONFIG_PLATFORM_I386_PC = y/CONFIG_PLATFORM_I386_PC = n/g" $@/Makefile && \
	sed -i "s/CONFIG_PLATFORM_ARM_RK1106 = n/CONFIG_PLATFORM_ARM_RK1106 = y/g" $@/Makefile && \
	make -C $@ ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) KSRC=$(KDIR) -j16

$(objects_clean):
	make -C $(patsubst %_clean,%,$@) clean
	sed -i "s/CONFIG_PLATFORM_I386_PC = n/CONFIG_PLATFORM_I386_PC = y/g" $(patsubst %_clean,%,$@)/Makefile && \
	sed -i "s/CONFIG_PLATFORM_ARM_RK1106 = y/CONFIG_PLATFORM_ARM_RK1106 = n/g" $(patsubst %_clean,%,$@)/Makefile
