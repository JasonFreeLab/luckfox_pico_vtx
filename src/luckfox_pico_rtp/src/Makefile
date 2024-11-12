#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TARGET := luckfox_pico_rtp

BUILD_DIR := $(OUT_DIR)/src/$(TARGET)
LOCAL_PREFIX :=  $(BUILD_DIR)/_install

CXX_DEFINES := -DOPENCV_DISABLE_THREAD_SUPPORT=1 -DRV1106_1103 -O1
CXX_INCLUDES := -I$(CURDIR)/../include \
	-I$(LUCKFOX_MEDIA_PATH)/samples/common \
	-I$(LUCKFOX_MEDIA_PATH)/samples/common/isp3.x \
	-I$(LUCKFOX_MEDIA_PATH)/out/include \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rkaiq \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rkaiq/algos \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rkaiq/common \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rkaiq/iq_parser \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rkaiq/iq_parser_v2 \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rkaiq/uAPI2 \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rkaiq/xcore \
	-I$(LUCKFOX_MEDIA_PATH)/out/include/rockchip \
	-I$(BUILDROOT_SYSROOT)/usr/lib/glib-2.0/include \
	-I$(BUILDROOT_SYSROOT)/usr/include/gstreamer-1.0 \
	-I$(BUILDROOT_SYSROOT)/usr/include/glib-2.0

CXX_FLAGS := $(CFLAGS)
_LDFLAGS := $(LDFLAGS) -L$(LUCKFOX_MEDIA_PATH)/out/lib \
	-L$(BUILDROOT_SYSROOT)/usr/lib \
	-L$(BUILDROOT_SYSROOT)/usr/lib/gstreamer-1.0 \
	-L$(BUILDROOT_SYSROOT)/usr/lib/glib-2.0 \
	-lrga -lsample_comm -lrockit -lrkaiq -lrockchip_mpp -lrtsp \
	-lgstreamer-1.0 -lm -ldl -lgstapp-1.0 -lglib-2.0 -pthread -liconv -lintl -lgobject-2.0 -lgmodule-2.0 -lgstbase-1.0 -lpcre -lffi

SRCS := $(wildcard *.cc)
OBJS := $(SRCS:.cc=.o)
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean
all: $(BUILD_DIR) $(LOCAL_PREFIX) $(TARGET)

$(BUILD_DIR):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

$(LOCAL_PREFIX):
	@test -d $(LOCAL_PREFIX) || mkdir -p $(LOCAL_PREFIX)

%.o: %.cc
	$(CXX) $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -c -o $(BUILD_DIR)/$@ $<

$(TARGET): $(OBJS)
	cd $(BUILD_DIR)/ && \
	$(CXX) -o $(LOCAL_PREFIX)/$@ $^ $(_LDFLAGS) && \
	cd -

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(LOCAL_PREFIX)