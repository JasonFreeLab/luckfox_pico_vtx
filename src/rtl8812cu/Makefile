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

#==================================================================
#                          modules
#==================================================================
.PHONY: $(objects) $(objects_clean)

$(objects):
	make -C $@ ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) KSRC=$(KDIR) -j16

$(objects_clean):
	make -C $(patsubst %_clean,%,$@) clean
