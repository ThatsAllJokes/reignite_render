#include "light_component.h"

#define GLM_FORCE_RADIANS
#include <gtc/matrix_transform.hpp>

bool InitLightComponent(LightComp& lc) {

  lc.type = Reignite::LightComponent::Type::RI_LIGHT_TYPE_SPOTLIGHT;
  lc.direction = vec3f(0.0f, 0.0f, 1.0f);
  lc.position = vec3f(0.0f, 2.0f, -8.0f);
  lc.view_mat = mat4f(1.0f);
  lc.projection_mat = mat4f(1.0f);

  float far_z = 100.0f;
  float near_z = 0.1f;
  float fov = 45.0f;
  float aspect = 4.0f / 5.0f;

  return false;
}

void UpdateLightComponent(LightComp& lc) {

  lc.view_mat = glm::lookAt(lc.position, vec3f(0.0f), vec3f(0.0f, 1.0f, 0.0f));
  lc.projection_mat = glm::perspective(lc.fov, lc.aspect, lc.near_z, lc.far_z);
}

void DestroyLightComponent(LightComp& lc) {

}

