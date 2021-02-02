//
// Created by Dishaan Ahuja.
//

#include "common.h"
#include "ovr_layer.h"
#include "ovr_mobile_session.h"
#include "VrApi.h"
#include "VrApi_Helpers.h"

namespace ovrmobile {

void OvrLayer::setup_equirect_layer() {
  swap_chain = vrapi_CreateAndroidSurfaceSwapChain2(1, 1, false);
  secure_swap_chain = vrapi_CreateAndroidSurfaceSwapChain2(1, 1, true);

  layer.Equirect = vrapi_DefaultLayerEquirect2();
  layer.Equirect.Header.Flags = VRAPI_FRAME_LAYER_FLAG_CLIP_TO_TEXTURE_RECT;

  layer.Equirect.HeadPose.Pose.Position.x = 0.0f;
  layer.Equirect.HeadPose.Pose.Position.y = 0.0f;
  layer.Equirect.HeadPose.Pose.Position.z = 0.0f;
  layer.Equirect.HeadPose.Pose.Orientation.x = 0.0f;
  layer.Equirect.HeadPose.Pose.Orientation.y = 0.0f;
  layer.Equirect.HeadPose.Pose.Orientation.z = 0.0f;
  layer.Equirect.HeadPose.Pose.Orientation.w = 1.0f;

  layer.Equirect.TexCoordsFromTanAngles = ovrMatrix4f_CreateIdentity();
  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
    layer.Equirect.Textures[eye].ColorSwapChain = swap_chain;
    layer.Equirect.Textures[eye].SwapChainIndex = 0;
  }

  active_layer_type = VRAPI_LAYER_TYPE_EQUIRECT2;
  set_format(VideoScreenFormat::EQR_MONO_360);
}

void OvrLayer::setup_cylinder_layer(int width, int height) {
  swap_chain = vrapi_CreateAndroidSurfaceSwapChain2(1, 1, false);
  secure_swap_chain = vrapi_CreateAndroidSurfaceSwapChain2(1, 1, true);
  layer.Cylinder = vrapi_DefaultLayerCylinder2();
  const float fadeLevel = 1.0f;
  layer.Header.ColorScale.x = layer.Header.ColorScale.y = layer.Header.ColorScale.z =
  layer.Header.ColorScale.w = fadeLevel;
  layer.Header.SrcBlend = VRAPI_FRAME_LAYER_BLEND_SRC_ALPHA;
  layer.Header.DstBlend = VRAPI_FRAME_LAYER_BLEND_ONE_MINUS_SRC_ALPHA;
  layer.Cylinder.Header.Flags = VRAPI_FRAME_LAYER_FLAG_CLIP_TO_TEXTURE_RECT;
  auto* session = OvrMobileSession::get_singleton_instance();
  auto tracking = session->get_head_tracker();
  layer.Cylinder.HeadPose = tracking.HeadPose;

  ovrVector3f translation = {0.0f, 0.0f, -2.0f};
  float rotateYawDegrees = 0.0f;
  float rotatePitchDegrees = 0.0f;
  float radius = 3.0f;
  const ovrMatrix4f cylinderTransform = CylinderModelMatrix(translation, rotateYawDegrees,
      rotatePitchDegrees, radius);

  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
    ovrMatrix4f modelViewMatrix =
        ovrMatrix4f_Multiply(&tracking.Eye[eye].ViewMatrix, &cylinderTransform);
    layer.Cylinder.Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_Inverse(&modelViewMatrix);
    layer.Cylinder.Textures[eye].ColorSwapChain = swap_chain;
    layer.Cylinder.Textures[eye].SwapChainIndex = 0;

    const float heightDegrees = 30.0f;
    const float widthDegrees = heightDegrees * (16.0f/9.0f);
    const float widthScale = 180.0f / widthDegrees;
    const float heightScale = 60.0f / heightDegrees;
    const float widthTranslation = widthScale * (((180.0f - widthDegrees) / 2) / 180.0f);
    const float heightTranslation = heightScale * (((60.0f - heightDegrees) / 2) / 60.0f);
    layer.Cylinder.Textures[eye].TextureMatrix = ovrMatrix4f_CreateScale(widthScale,
        heightScale, 1);
    layer.Cylinder.Textures[eye].TextureMatrix.M[0][2] = -widthTranslation;
    layer.Cylinder.Textures[eye].TextureMatrix.M[1][2] = heightTranslation;
    layer.Cylinder.Textures[eye].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
  }
  active_layer_type = VRAPI_LAYER_TYPE_CYLINDER2;
}

void OvrLayer::update_cylinder_layer(ovrTracking2 tracking) {
  layer.Cylinder.HeadPose = tracking.HeadPose;

  ovrVector3f translation = {0.0f, 0.0f, -2.0f};
  float rotateYawDegrees = 0.0f;
  float rotatePitchDegrees = 0.0f;
  float radius = 3.0f;
  const ovrMatrix4f cylinderTransform = CylinderModelMatrix(translation, rotateYawDegrees,
                                                            rotatePitchDegrees, radius);
  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
    ovrMatrix4f modelViewMatrix =
        ovrMatrix4f_Multiply(&tracking.Eye[eye].ViewMatrix, &cylinderTransform);
    layer.Cylinder.Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_Inverse(&modelViewMatrix);
  }
}

OvrLayer::OvrLayer(int width, int height) {

  // TODO: Determine type of layer based on format, not whether width/height is supplied.
  if (width == 0 && height == 0) {
    setup_equirect_layer();
  } else {
    setup_cylinder_layer(width, height);
  }

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
      setup_equirect_layer();
      layer.Equirect.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Equirect.Textures[0].TextureMatrix = ovrMatrix4f_CreateScale(2, 1, 1);
      layer.Equirect.Textures[0].TextureMatrix.M[0][2] = -.5f; // .5f = 90 degrees due to 2x scale.

      layer.Equirect.Textures[1].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Equirect.Textures[1].TextureMatrix = ovrMatrix4f_CreateScale(2, 1, 1);
      layer.Equirect.Textures[1].TextureMatrix.M[0][2] = -.5f;
      break;
    case VideoScreenFormat::EQR_MONO_360:
      setup_equirect_layer();
      layer.Equirect.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Equirect.Textures[0].TextureMatrix = ovrMatrix4f_CreateIdentity();

      layer.Equirect.Textures[1].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Equirect.Textures[1].TextureMatrix = ovrMatrix4f_CreateIdentity();
      break;
    case VideoScreenFormat::EQR_LEFT_RIGHT_180:
      setup_equirect_layer();
      layer.Equirect.Textures[0].TextureRect = {0.0f, 0.0f, 0.5f, 1.0f};
      layer.Equirect.Textures[0].TextureMatrix = ovrMatrix4f_CreateIdentity();
      layer.Equirect.Textures[0].TextureMatrix.M[0][2] = -.25f; // .25f = 90 degrees.

      layer.Equirect.Textures[1].TextureRect = {0.5f, 0.0f, 0.5f, 1.0f};
      layer.Equirect.Textures[1].TextureMatrix = ovrMatrix4f_CreateIdentity();
      layer.Equirect.Textures[1].TextureMatrix.M[0][2] = .25f;
      break;
    case VideoScreenFormat::EQR_TOP_BOTTOM_360:
      setup_equirect_layer();
      layer.Equirect.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 0.5f};
      layer.Equirect.Textures[0].TextureMatrix = ovrMatrix4f_CreateScale(1, .5f, 1);

      layer.Equirect.Textures[1].TextureRect = {0.0f, 0.5f, 1.0f, 0.5f};
      layer.Equirect.Textures[1].TextureMatrix = ovrMatrix4f_CreateScale(1, .5f, 1);
      layer.Equirect.Textures[1].TextureMatrix.M[1][2] = .5f;
      break;
    case VideoScreenFormat::RECTILINEAR_20X9_MONO: {
      setup_equirect_layer();
      // TODO(dishaan): Use a cylindrical layer and adjust its aspect ratio based on the incoming
      // video (currently hardcoded for 20:9 aspect ratio).
      const float widthDegrees = 70.0f;
      const float heightDegrees = 31.5f;
      const float widthScale = 360.0f / widthDegrees;
      const float heightScale = 180.0f / heightDegrees;
      const float widthTranslation = widthScale * ((180.0f - widthDegrees / 2) / 360.0f);
      const float heightTranslation = heightScale * ((90.0f - heightDegrees / 2) / 180.0f);

      layer.Equirect.Textures[0].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Equirect.Textures[0].TextureMatrix = ovrMatrix4f_CreateScale(widthScale, heightScale, 1);
      layer.Equirect.Textures[0].TextureMatrix.M[0][2] = -widthTranslation;
      layer.Equirect.Textures[0].TextureMatrix.M[1][2] = -heightTranslation;

      layer.Equirect.Textures[1].TextureRect = {0.0f, 0.0f, 1.0f, 1.0f};
      layer.Equirect.Textures[1].TextureMatrix = ovrMatrix4f_CreateScale(widthScale, heightScale, 1);
      layer.Equirect.Textures[1].TextureMatrix.M[0][2] = -widthTranslation;
      layer.Equirect.Textures[1].TextureMatrix.M[1][2] = -heightTranslation;
      break;
    }
    default:
      ALOGE("VideoScreen::SetFormat(%d) unsupported", format);
  }
}

void OvrLayer::set_surface_protected(bool is_protected) {
  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
    layer.Equirect.Textures[eye].ColorSwapChain = is_protected ? secure_swap_chain :
                                                  swap_chain;
    layer.Cylinder.Textures[eye].ColorSwapChain = is_protected ? secure_swap_chain :
                                                  swap_chain;
  }
}

jobject OvrLayer::get_layer_surface() {
  return vrapi_GetTextureSwapChainAndroidSurface(
      active_layer_type == VRAPI_LAYER_TYPE_EQUIRECT2 ?
      layer.Equirect.Textures[0].ColorSwapChain : layer.Cylinder.Textures[0].ColorSwapChain);
}

ovrLayerHeader2 *OvrLayer::get_layer_header() {
  return active_layer_type == VRAPI_LAYER_TYPE_EQUIRECT2 ? &layer.Equirect.Header : &layer.Cylinder.Header;
}

} // namespace ovrmobile


