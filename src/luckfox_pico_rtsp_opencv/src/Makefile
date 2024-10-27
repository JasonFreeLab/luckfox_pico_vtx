#===============================================================================
# export variables
#===============================================================================
PROJECT_DIR := $(shell cd $(CURDIR)/../../.. && /bin/pwd)
include $(PROJECT_DIR)/base.mak

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
objects := luckfox_pico_rtsp_opencv

CXX_DEFINES := -DOPENCV_DISABLE_THREAD_SUPPORT=1 -DRV1106_1103 -O1
CXX_INCLUDES := -I../include \
	-I../include/opencv4 \
	-I../include/rkaiq \
	-I../include/rkaiq/algos \
	-I../include/rkaiq/common \
	-I../include/rkaiq/iq_parser \
	-I../include/rkaiq/iq_parser_v2 \
	-I../include/rkaiq/uAPI2 \
	-I../include/rkaiq/xcore
CXX_FLAGS := $(CFLAGS)
_LDFLAGS := $(LDFLAGS) -L../lib \
	-lsample_comm -lrockit -lrkaiq -lrockchip_mpp -lrtsp \
	-lopencv_core -lopencv_imgproc -lrga

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean
all: $(objects)

%.o: %.cc
	$(CXX) $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -c -o $@ $<

$(objects): main.o luckfox_mpi.o
	$(CXX) -o $@ $^ $(_LDFLAGS)

clean:
	rm *.o $(objects)