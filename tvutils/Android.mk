# Copyright (c) 2014 Amlogic, Inc. All rights reserved.
#
# This source code is subject to the terms and conditions defined in the
# file 'LICENSE' which is part of this source code package.
#
# Description: makefile

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIB_VENDOR := $(wildcard vendor/amlogic)
LIB_SQLITE_PATH := $(wildcard external/sqlite)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
  CFile.cpp \
  CTvLog.cpp \
  CMsgQueue.cpp \
  CSqlite.cpp \
  CThread.cpp \
  serial_base.cpp \
  serial_operate.cpp \
  tvutils.cpp \
  zepoll.cpp \
  tvconfig/CIniFile.cpp \
  tvconfig/tvconfig.cpp

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH)/include \
  $(LIB_SQLITE_PATH)/dist \
  $(LIB_VENDOR)/frameworks/services \
  external/jsoncpp/include

LOCAL_SHARED_LIBRARIES += \
  libsystemcontrolservice

LOCAL_STATIC_LIBRARIES += \
  libjsoncpp

LOCAL_MODULE:= libtv_utils

include $(BUILD_STATIC_LIBRARY)
