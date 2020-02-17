#include "camera_component.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/transform.hpp>

Reignite::CameraComponent::CameraComponent() {

  type = RI_CAMERA_TYPE_PERSPECTIVE;
  forward = glm::vec3(0.0f, 0.0f, 1.0f);
  position = vec3f(0.0f, 1.0f, -6.0f);
  view_mat = glm::mat4(1.0f);
  projection_mat = glm::mat4(1.0f);

  fov = 70.0f;
  aspect = 16.0f / 9.0f;
  near_z = 0.1f;
  far_z = 100.0f;
}

void Reignite::UpdateCameraComponent(CameraComponent& cc) {

  cc.view_mat = glm::lookAt(cc.position, cc.position + cc.forward, vec3f(0.0f, 1.0f, 0.0f));
  cc.projection_mat = glm::perspective(glm::radians(cc.fov), cc.aspect, cc.near_z, cc.far_z);
}