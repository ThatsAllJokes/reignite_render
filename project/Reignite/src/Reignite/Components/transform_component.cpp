#include "transform_component.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/transform.hpp>

bool InitTransformComponent(TransformComp& tf) {

  tf.position = vec3f(0.0f, 0.0f, 0.0f);
  tf.rotation = vec3f(0.0f, 0.0f, 0.0f);
  tf.scale = vec3f(1.0f, 1.0f, 1.0f);
  tf.local = mat4f(1.0f);
  tf.global = mat4f(1.0f);
  tf.parent_world = nullptr;

  return true;
}

void UpdateTransformComponent(TransformComp& tf) {

  glm::mat4 mat_transform = glm::translate(glm::mat4(1.0f), tf.position);
  glm::mat4 mat_rotation_x = glm::rotate(tf.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 mat_rotation_y = glm::rotate(tf.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 mat_rotation_z = glm::rotate(tf.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
  glm::mat4 mat_rotation = mat_rotation_x * mat_rotation_y * mat_rotation_z;
  glm::mat4 mat_scale = glm::scale(tf.scale);
  tf.local = mat_transform * mat_rotation * mat_scale;

  if (nullptr != tf.parent_world) {
    tf.global = *(tf.parent_world) * tf.local;
  }
  else {
    tf.global = tf.local;
  }

}