#include "camera_component.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/transform.hpp>

bool InitCameraComponent(CameraComp& tf) {

  tf.position = vec3f(2.0f, 2.0f, 2.0f);
  tf.type = CameraComp::RI_CAMERA_TYPE_PERSPECTIVE;
  tf.forward = glm::vec3(0.0f, 0.0f, 1.0f);
  tf.fov = 70.0f;
  tf.aspect = 16.0f / 9.0f;
  tf.near_z = 0.1f;
  tf.far_z = 100.0f;
  tf.left = 100.0f;
  tf.right = 100.0f;
  tf.bottom = 60.0f;
  tf.top = 60.0f;
  tf.view_mat = glm::mat4(1.0f);
  tf.projection_mat = glm::mat4(1.0f);

  return true;
}

void UpdateCameraComponent(CameraComp& tf) {

  tf.view_mat = glm::lookAt(tf.position, tf.position + tf.forward, vec3f(0.0f, 1.0f, 0.0f));

  switch (tf.type) {
  case CameraComp::RI_CAMERA_TYPE_PERSPECTIVE:
    tf.projection_mat = glm::perspective(glm::radians(tf.fov), tf.aspect, tf.near_z, tf.far_z);
    break;
  case CameraComp::RI_CAMERA_TYPE_ORTHOGONAL:
    tf.projection_mat = glm::ortho(tf.left, tf.right, tf.bottom, tf.top, tf.near_z, tf.far_z);
    break;
  default:
    assert(!"Unasigned camera type?");
    break;
  }
}