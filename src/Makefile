#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
objects := wifi libnl libpcap libsodium wfb-ng luckfox_pico_rtsp_opencv
objects_clean := $(addsuffix _clean,$(objects))
objects_uninstall := $(addsuffix _uninstall,$(objects))
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean

all: $(objects)
libpcap : libnl
wfb-ng : libnl libpcap libsodium

clean: $(objects_clean)

install: all

uninstall: $(objects_uninstall)

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
