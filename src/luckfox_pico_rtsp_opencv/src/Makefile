#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TARGET := luckfox_pico_rtsp_opencv

BUILD_DIR := $(OUT_DIR)/src/$(TARGET)
LOCAL_PREFIX :=  $(BUILD_DIR)/_install

CXX_DEFINES := -DOPENCV_DISABLE_THREAD_SUPPORT=1 -DRV1106_1103 -O1
CXX_INCLUDES := -I$(CURDIR)/../include \
	-I$(CURDIR)/../include/opencv4 \
	-I$(CURDIR)/../include/rkaiq \
	-I$(CURDIR)/../include/rkaiq/algos \
	-I$(CURDIR)/../include/rkaiq/common \
	-I$(CURDIR)/../include/rkaiq/iq_parser \
	-I$(CURDIR)/../include/rkaiq/iq_parser_v2 \
	-I$(CURDIR)/../include/rkaiq/uAPI2 \
	-I$(CURDIR)/../include/rkaiq/xcore
CXX_FLAGS := $(CFLAGS)
_LDFLAGS := $(LDFLAGS) -L$(CURDIR)/../lib \
	-lsample_comm -lrockit -lrkaiq -lrockchip_mpp -lrtsp \
	-lopencv_core -lopencv_imgproc -lrga

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