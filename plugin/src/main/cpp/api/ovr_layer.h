//
// Created by Dishaan Ahuja.
//

#ifndef GODOT_OCULUS_MOBILE_PLUGIN_OVR_LAYER_H
#define GODOT_OCULUS_MOBILE_PLUGIN_OVR_LAYER_H

#include "VrApi_Types.h"
#include "VrApi_Helpers.h"
#include <jni.h>
#include <core/Godot.hpp>

namespace ovrmobile {
namespace {
using namespace godot;
}

// Keep in sync with OvrLayer.kt
enum class VideoScreenFormat {
    EQR_MONO_180,
    EQR_MONO_360,
    EQR_LEFT_RIGHT_180,
    EQR_TOP_BOTTOM_360,
    RECTILINEAR_20X9_MONO,
};

class OvrLayer {

 public:
    OvrLayer(int width, int height);
    ~OvrLayer();

    void set_format(VideoScreenFormat format);
    void set_surface_protected(bool is_protected);

    jobject get_layer_surface();

    ovrLayerHeader2 *get_layer_header();

 private:
    ovrLayer_Union2 layer;
    ovrLayerType2 active_layer_type;
    ovrTextureSwapChain* swap_chain;
    ovrTextureSwapChain* secure_swap_chain;

    void setup_equirect_layer();
    void setup_cylinder_layer(int width, int height);
};

} // namespace ovrmobile

#endif // GODOT_OCULUS_MOBILE_PLUGIN_OVR_LAYER_H
