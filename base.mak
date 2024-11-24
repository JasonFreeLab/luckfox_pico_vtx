LUCKFOX_SDK_PATH ?= $(HOME)/luckfox-pico

ARCH := arm

TOOLCHAINS_NAME := arm-rockchip830-linux-uclibcgnueabihf
HOST := $(TOOLCHAINS_NAME)
CROSS_COMPILE := $(HOST)-
CMAKE_C_COMPILER := $(CROSS_COMPILE)gcc
CMAKE_CXX_COMPILER := $(CROSS_COMPILE)g++

# KDIR := $(LUCKFOX_SDK_PATH)/sysdrv/source/objs_kernel
KDIR := $(LUCKFOX_SDK_PATH)/sysdrv/source/kernel

BUILDROOT_SYSROOT := $(LUCKFOX_SDK_PATH)/sysdrv/source/buildroot/buildroot-2023.02.6/output/host/arm-buildroot-linux-uclibcgnueabihf/sysroot
LUCKFOX_MEDIA_PATH := $(LUCKFOX_SDK_PATH)/media

OUT_DIR := $(PROJECT_DIR)/build
INSTALL_DIR := $(PROJECT_DIR)/install

# CFG_WIFI_SUPPORT_RTL8812AU = y
CFG_WIFI_SUPPORT_RTL8812CU = y
# CFG_WIFI_SUPPORT_RTL8812EU = y
