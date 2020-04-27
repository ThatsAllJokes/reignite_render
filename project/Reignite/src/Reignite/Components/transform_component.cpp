#include "transform_component.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/transform.hpp>



void Reignite::TransformComponents::UpdateTransformComponents() {

  for (u32 i = 0; i < size; ++i) {

    glm::mat4 mat_transform = glm::translate(glm::mat4(1.0f), positions[i]);
    glm::mat4 mat_scale = glm::scale(scales[i]);

    glm::mat4 mat_rotation_x = glm::rotate(rotations[i].x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 mat_rotation_y = glm::rotate(rotations[i].y, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 mat_rotation_z = glm::rotate(rotations[i].z, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 mat_rotation = mat_rotation_x * mat_rotation_y * mat_rotation_z;
  
    locals[i] = mat_transform * mat_rotation * mat_scale;
    globals[i] = locals[i];
  }

  /*if (nullptr != tf.parent_world) {
    tf.global = *(tf.parent_world) * tf.local;
  }
  else { }*/
}