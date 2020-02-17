#include "light_component.h"

#define GLM_FORCE_RADIANS
#include <gtc/matrix_transform.hpp>


Reignite::LightComponent::LightComponent() {

  type = RI_LIGHT_TYPE_SPOTLIGHT;
  direction = vec3f(0.0f, 0.0f, 1.0f);
  position = vec3f(0.0f, 2.0f, -8.0f);
  view_mat = mat4f(1.0f);
  projection_mat = mat4f(1.0f);

  far_z = 100.0f;
  near_z = 0.1f;
  fov = 45.0f;
  aspect = 4.0f / 5.0f;
}

void Reignite::UpdateLightComponent(LightComponent& lc) {

  lc.view_mat = glm::lookAt(lc.position, vec3f(0.0f), vec3f(0.0f, 1.0f, 0.0f));
  lc.projection_mat = glm::perspective(lc.fov, lc.aspect, lc.near_z, lc.far_z);
}


