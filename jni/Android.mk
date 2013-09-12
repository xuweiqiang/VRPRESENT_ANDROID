# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANliY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH:= $(call my-dir)

LOCAL_ROOTPATH:= $(call my-dir)/../../../../..

NDK:= /Users/xuweiqiang25/Workspace/android-ndk-r8b


include $(CLEAR_VARS)

LOCAL_MODULE    := libvrpresent
LOCAL_CFLAGS    := -Werror -DGFX_ANDROID_OPENGL_ES_SHADER -DANDROID_NDK  -Wno-psabi
LOCAL_SRC_FILES += GfxPresentMain.cpp GfxPresentPrepare.cpp GfxVideoWindow.cpp GfxProcAndroidOpenGLES.cpp
LOCAL_SRC_FILES += GfxStreamAndroidOpenGLES.cpp ESRender.cpp
LOCAL_LDLIBS    := -llog -lGLESv2 
LOCAL_C_INCLUDES :=  $(LOCAL_ROOTPATH)/inc $(LOCAL_PATH)/inc/libwin32/include/ $(LOCAL_PATH)/inc/imglib/
LOCAL_LDFLAGS += $(LOCAL_PATH)/inc/libwin32/libwin32_arm.a

LOCAL_C_INCLUDES += $(LOCAL_ROOTPATH)/3rdparty/imglib/
#LOCAL_CFLAGS    += -DSTATIC_IMGPROC_LIB
#LOCAL_LDFLAGS += $(LOCAL_ROOTPATH)/3rdparty/imglib/lnx/armv6/ci_imgproc_armv6_lnx.a
 
#LOCAL_ARM_MODE := arm		

LOCAL_C_INCLUDES += $(NDK)/sources/android/cpufeatures
LOCAL_STATIC_LIBRARIES += cpufeatures


include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := libgl2jni
LOCAL_CFLAGS    := -Werror -DGFX_ANDROID_OPENGL_ES_SHADER  -Wno-psabi
LOCAL_SRC_FILES := gl_code.cpp
LOCAL_LDLIBS    := -llog -lGLESv2 
LOCAL_C_INCLUDES := $(LOCAL_ROOTPATH)/inc/ $(LOCAL_PATH)/inc/libwin32/include/
LOCAL_LDFLAGS += $(LOCAL_PATH)/inc/libwin32/libwin32_arm.a

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libciviproc_arm
LOCAL_SRC_FILES := libciviproc_arm.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libciviproc_neon
LOCAL_SRC_FILES := libciviproc_neon.so
include $(PREBUILT_SHARED_LIBRARY)


$(call import-module,android/cpufeatures)





