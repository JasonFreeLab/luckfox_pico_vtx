#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(CURDIR)

include base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
objects := src
objects_clean := $(addsuffix _clean,$(objects))
objects_uninstall := $(addsuffix _uninstall,$(objects))

MODULE_PATH := scripts
SCRIPTS_INSTALL_DIR := $(INSTALL_DIR)/vtx
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean

all: $(objects)
	@test -d $(SCRIPTS_INSTALL_DIR) || mkdir -p $(SCRIPTS_INSTALL_DIR)
	@cp -rv $(MODULE_PATH) $(SCRIPTS_INSTALL_DIR)/

clean: $(objects_clean)
	@rm -rf $(OUT_DIR)

install: all

uninstall: $(objects_uninstall)
	@rm -rf $(INSTALL_DIR)

#==================================================================
#                          modules
#==================================================================
.PHONY: $(objects) $(objects_clean)

$(objects):
	make -C $@ all

$(objects_clean):
	make -C $(patsubst %_clean,%,$@) clean

$(objects_uninstall):
	make -C $(patsubst %_uninstall,%,$@) uninstall
