#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
objects := src
objects_clean := $(addsuffix _clean,$(objects))

INSTALL_DIR := $(INSTALL_DIR)/vtx/bin

MODULE_PATH := $(OUT_DIR)/src/luckfox_pico_rtp/_install/luckfox_pico_rtp

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean  install uninstall

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
	@echo -e "\033[32m""Build luckfox_pico_rtp ...""\033[00m"
	make -C $@ -j16 CC=$(CMAKE_C_COMPILER)

$(objects_clean):
	make -C $(patsubst %_clean,%,$@) clean
