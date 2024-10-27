LUCKFOX_SDK_PATH ?= /home/jc/luckfox-pico

ARCH := arm

TOOLCHAINS_NAME := arm-rockchip830-linux-uclibcgnueabihf
HOST := $(TOOLCHAINS_NAME)
CROSS_COMPILE := $(HOST)-
CMAKE_C_COMPILER := $(CROSS_COMPILE)gcc
CMAKE_CXX_COMPILER := $(CROSS_COMPILE)g++

KDIR := $(LUCKFOX_SDK_PATH)/sysdrv/source/objs_kernel
# KDIR := $(LUCKFOX_SDK_PATH)/sysdrv/source/kernel/


OUT_DIR := $(PROJECT_DIR)/build
INSTALL_DIR := $(PROJECT_DIR)/install/
