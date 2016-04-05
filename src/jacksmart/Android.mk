#
# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#     Unless required by applicable law or agreed to in writing, software
#     distributed under the License is distributed on an "AS IS" BASIS,
#     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#     See the License for the specific language governing permissions and
#     limitations under the License.
#

LOCAL_PATH := $(call my-dir)

# Brillo service
include $(CLEAR_VARS)
LOCAL_MODULE := jacksmart
LOCAL_C_INCLUDES := external/tinyalsa/include external/gtest/include \
  $(TOP)/frameworks/av/media/libstagefright \
  $(TOP)/frameworks/native/include/media/openmax \
  $(TOP)/device/generic/brillo/pts/audio/common \
LOCAL_CFLAGS := -Wall -Werror -Wno-sign-promo
LOCAL_CPP_EXTENSION := .cc
LOCAL_INIT_RC := jacksmart.rc
LOCAL_REQUIRED_MODULES := jacksmart.json jacksmart.schema.json
LOCAL_SRC_FILES := \
    include/peripherals/gpio/gpio.cc \
    include/peripherals/i2c/facehelper.cc \
    include/peripherals/sound/alsasound.cc \
    include/peripherals/sound/sound.cc \
    jacksmart.cc
LOCAL_SHARED_LIBRARIES := libbrillo libbrillo-dbus libbrillo-stream \
  libchrome libchrome-dbus libdbus libweaved libtinyalsa \
  libstagefright libstagefright_foundation libutils libbase libbinder \
  libbrillo-binder libmedia
#LOCAL_RTTI_FLAG := -frtti
LOCAL_RTTI_FLAG := -fno-rtti
include $(BUILD_EXECUTABLE)


# TEST APP
include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= external/tinyalsa/include \
  $(TOP)/frameworks/av/media/libstagefright \
  $(TOP)/frameworks/native/include/media/openmax \
  $(TOP)/device/generic/brillo/pts/audio/common
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES:= \
    include/peripherals/gpio/gpio.cc \
    include/peripherals/sound/alsasound.cc \
    include/peripherals/sound/sound.cc \
    include/peripherals/spi/spi.cc \
    include/peripherals/spi/apa102.cc \
    testsmart.cc
LOCAL_MODULE := jacksmart-test
LOCAL_SHARED_LIBRARIES := libbrillo libbrillo-dbus libbrillo-stream \
  libchrome libchrome-dbus libdbus libweaved libtinyalsa \
  libstagefright libstagefright_foundation libutils libbase libbinder libmedia
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

# Weave schema files
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := jacksmart.json
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/weaved/traits
LOCAL_SRC_FILES := etc/weaved/traits/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)
