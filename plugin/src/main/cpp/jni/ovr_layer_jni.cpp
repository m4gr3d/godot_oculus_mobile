//
// Created by Dishaan Ahuja.
//

#include "api/ovr_layer.h"
#include "jni_common.h"
#include <jni.h>

#undef JNI_PACKAGE_NAME
#define JNI_PACKAGE_NAME org_godotengine_plugin_vr_oculus_mobile_api

#undef JNI_CLASS_NAME
#define JNI_CLASS_NAME OvrLayer

namespace {
ovrmobile::OvrLayer *get_layer(jlong layer_pointer) {
  return reinterpret_cast<ovrmobile::OvrLayer *>(layer_pointer);
}
} // namespace

extern "C" {
JNIEXPORT jlong JNICALL JNI_METHOD(nativeGenerateLayer)(JNIEnv *,
                                                        jobject,
                                                        jint width,
                                                        jint height) {
  auto *layer = new ovrmobile::OvrLayer(width, height);
  return jptr(layer);
}

JNIEXPORT void JNICALL JNI_METHOD(nativeSetFormat)(JNIEnv *,
                                                   jobject,
                                                   jlong layer_pointer,
                                                   jint format) {
  auto *layer = get_layer(layer_pointer);
  layer->set_format(static_cast<ovrmobile::VideoScreenFormat>(format));
}

JNIEXPORT void JNICALL JNI_METHOD(nativeSetSurfaceProtected)(JNIEnv *,
                                                             jobject,
                                                             jlong layer_pointer,
                                                             jboolean is_protected) {
  auto *layer = get_layer(layer_pointer);
  layer->set_surface_protected(is_protected);
}

JNIEXPORT void JNICALL JNI_METHOD(nativeDestroyLayer)(JNIEnv *, jobject, jlong layer_pointer) {
  auto *layer = get_layer(layer_pointer);
  delete layer;
}

JNIEXPORT jobject JNICALL JNI_METHOD(nativeGetLayerSurface)(JNIEnv *,
                                                            jobject,
                                                            jlong layer_pointer) {
  auto *layer = get_layer(layer_pointer);
  return layer->get_layer_surface();
}
};
