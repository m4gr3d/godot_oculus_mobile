//
// Created by Dishaan Ahuja.
//

#include "common.h"
#include "ovr_layer.h"
#include "ovr_mobile_session.h"
#include "VrApi.h"
#include "VrApi_Helpers.h"

namespace ovrmobile {

OvrLayer::OvrLayer() {
  swap_chain = vrapi_CreateAndroidSurfaceSwapChain2(1, 1, false);
  secure_swap_chain = vrapi_CreateAndroidSurfaceSwapChain2(1, 1, true);

  layer = vrapi_DefaultLayerEquirect2();
  layer.Header.Flags = VRAPI_FRAME_LAYER_FLAG_CLIP_TO_TEXTURE_RECT;

  layer.HeadPose.Pose.Position.x = 0.0f;
  layer.HeadPose.Pose.Position.y = 0.0f;
  layer.HeadPose.Pose.Position.z = 0.0f;
  layer.HeadPose.Pose.Orientation.x = 0.0f;
  layer.HeadPose.Pose.Orientation.y = 0.0f;
  layer.HeadPose.Pose.Orientation.z = 0.0f;
  layer.HeadPose.Pose.Orientation.w = 1.0f;

  layer.TexCoordsFromTanAngles = ovrMatrix4f_CreateIdentity();
  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
    layer.Textures[eye].ColorSwapChain = swap_chain;
    layer.Textures[eye].SwapChainIndex = 0;
  }

  set_format(VideoScreenFormat::EQR_MONO_360);

  // Add the generated layer to the session's layer set.
  auto* session = OvrMobileSession::get_singleton_instance();
  session->add_ovr_layer(this);
}

OvrLayer::~OvrLayer() {
  // Remove the generated layer from the session's layer set.
  auto* session = OvrMobileSession::get_singleton_instance();
  session->remove_ovr_layer(this);

  vrapi_DestroyTextureSwapChain(swap_chain);
  vrapi_DestroyTextureSwapChain(secure_swap_chain);
}

void OvrLayer::set_format(VideoScreenFormat format) {
  switch (format) {
    case VideoScreenFormat::EQR_MONO_180:
      layer.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Textures[0].TextureMatrix = ovrMatrix4f_CreateScale(2, 1, 1);
      layer.Textures[0].TextureMatrix.M[0][2] = -.5f; // .5f = 90 degrees due to 2x scale.

      layer.Textures[1].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Textures[1].TextureMatrix = ovrMatrix4f_CreateScale(2, 1, 1);
      layer.Textures[1].TextureMatrix.M[0][2] = -.5f;
      break;
    case VideoScreenFormat::EQR_MONO_360:
      layer.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Textures[0].TextureMatrix = ovrMatrix4f_CreateIdentity();

      layer.Textures[1].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Textures[1].TextureMatrix = ovrMatrix4f_CreateIdentity();
      break;
    case VideoScreenFormat::EQR_LEFT_RIGHT_180:
      layer.Textures[0].TextureRect = {0.0f, 0.0f, 0.5f, 1.0f};
      layer.Textures[0].TextureMatrix = ovrMatrix4f_CreateIdentity();
      layer.Textures[0].TextureMatrix.M[0][2] = -.25f; // .25f = 90 degrees.

      layer.Textures[1].TextureRect = {0.5f, 0.0f, 0.5f, 1.0f};
      layer.Textures[1].TextureMatrix = ovrMatrix4f_CreateIdentity();
      layer.Textures[1].TextureMatrix.M[0][2] = .25f;
      break;
    case VideoScreenFormat::EQR_TOP_BOTTOM_360:
      layer.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 0.5f};
      layer.Textures[0].TextureMatrix = ovrMatrix4f_CreateScale(1, .5f, 1);

      layer.Textures[1].TextureRect = {0.0f, 0.5f, 1.0f, 0.5f};
      layer.Textures[1].TextureMatrix = ovrMatrix4f_CreateScale(1, .5f, 1);
      layer.Textures[1].TextureMatrix.M[1][2] = .5f;
      break;
    case VideoScreenFormat::RECTILINEAR_20X9_MONO: {
      // TODO(dishaan): Use a cylindrical layer and adjust its aspect ratio based on the incoming
      // video (currently hardcoded for 20:9 aspect ratio).
      const float widthDegrees = 70.0f;
      const float heightDegrees = 31.5f;
      const float widthScale = 360.0f / widthDegrees;
      const float heightScale = 180.0f / heightDegrees;
      const float widthTranslation = widthScale * ((180.0f - widthDegrees / 2) / 360.0f);
      const float heightTranslation = heightScale * ((90.0f - heightDegrees / 2) / 180.0f);

      layer.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Textures[0].TextureMatrix = ovrMatrix4f_CreateScale(widthScale, heightScale, 1);
      layer.Textures[0].TextureMatrix.M[0][2] = -widthTranslation;
      layer.Textures[0].TextureMatrix.M[1][2] = -heightTranslation;

      layer.Textures[1].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Textures[1].TextureMatrix = ovrMatrix4f_CreateScale(widthScale, heightScale, 1);
      layer.Textures[1].TextureMatrix.M[0][2] = -widthTranslation;
      layer.Textures[1].TextureMatrix.M[1][2] = -heightTranslation;

      break;
    }
    default:
      ALOGE("VideoScreen::SetFormat(%d) unsupported", format);
  }
}

void OvrLayer::set_surface_protected(bool is_protected) {
  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
    layer.Textures[eye].ColorSwapChain = is_protected ? secure_swap_chain : swap_chain;
  }
}

jobject OvrLayer::get_layer_surface() {
  return vrapi_GetTextureSwapChainAndroidSurface(layer.Textures[0].ColorSwapChain);
}

ovrLayerHeader2 *OvrLayer::get_layer_header() {
  return &layer.Header;
}

} // namespace ovrmobile


