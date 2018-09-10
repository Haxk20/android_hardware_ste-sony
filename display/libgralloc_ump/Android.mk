# 
# Copyright (C) 2010 ARM Limited. All rights reserved.
# 
# Portions of this code have been modified from the original.
# These modifications are:
#    * The build configuration for the Gralloc module
# 
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

# HAL module implemenation, not prelinked and stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libGLESv1_CM

# Include the UMP header files
LOCAL_C_INCLUDES += \
    bionic/libc/include \
    $(LOCAL_PATH)/../include

LOCAL_SRC_FILES := \
	gralloc_module.cpp

LOCAL_MODULE := gralloc.$(TARGET_BOARD_PLATFORM)1
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true

LOCAL_CFLAGS:= -DLOG_TAG=\"gralloc\" -DGRALLOC_32_BITS
#LOCAL_CFLAGS+= -DMALI_VSYNC_EVENT_REPORT_ENABLE

LOCAL_STATIC_LIBRARIES        := libgralloc1-adapter-exynos4
LOCAL_SHARED_LIBRARIES        += libsync

ifeq ($(TARGET_USES_GRALLOC1), true)
LOCAL_CFLAGS += -DADVERTISE_GRALLOC1
endif

include $(BUILD_SHARED_LIBRARY)