/**
* Created by Fredia Huya-Kouadio.
*/

#define XR_USE_PLATFORM_ANDROID

#include <openxr.h>
#include "ovr_mobile_session.h"
#include "openxr_oculus.h"

namespace ovrmobile {

OvrMobileSession::OvrMobileSession() {
  // TODO: Complete
}

OvrMobileSession::~OvrMobileSession() {
  // TODO: Complete
}

bool OvrMobileSession::initialize() {
  if (initialized) {
    return initialized;
  }

  ALOGV("OvrMobileSession::initialize() called");

  JNIEnv* env = godot::android_api->godot_android_get_env();
  java.ActivityObject = env->NewGlobalRef(godot::android_api->godot_android_get_activity());
  java.Env = env;
  env->GetJavaVM(&java.Vm);

  PFN_xrInitializeLoaderKHR xrInitializeLoadedKHR;
  xrGetInstanceProcAddr(XR_NULL_HANDLE,
                        "xrInitializeLoaderKHR",
                        (PFN_xrVoidFunction*) &xrInitializeLoadedKHR);
  if (xrInitializeLoadedKHR == nullptr) {
    ALOGE("Unable to setup PFN_xrInitializeLoaderKHR");
  }

  XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid;
  memset(&loaderInitializeInfoAndroid, 0, sizeof(loaderInitializeInfoAndroid));
  loaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
  loaderInitializeInfoAndroid.next = nullptr;
  loaderInitializeInfoAndroid.applicationVM = java.Vm;
  loaderInitializeInfoAndroid.applicationContext = &java.ActivityObject;
  xrInitializeLoadedKHR((XrLoaderInitInfoBaseHeaderKHR*) &loaderInitializeInfoAndroid);

  // init our extensions container
  ovrmobile::OpenGLExtensions::initExtensions();
}
}
