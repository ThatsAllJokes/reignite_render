#include "transform_component.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/transform.hpp>


void Reignite::TransformComponents::init(u32 maxSize) {

  used.reserve(maxSize);
  active.reserve(maxSize);

  position.reserve(maxSize);
  rotation.reserve(maxSize);
  scale.reserve(maxSize);
  local.reserve(maxSize);
  global.reserve(maxSize);
}

void Reignite::TransformComponents::clear() {

  used.clear();
  active.clear();

  position.clear();
  rotation.clear();
  scale.clear();
  local.clear();
  global.clear();
}

void Reignite::TransformComponents::add(vec3f pos) {

  used.push_back(true);
  active.push_back(true);

  position.push_back(pos);
  rotation.push_back(vec3f(0.0f));
  scale.push_back(vec3f(1.0f));
  local.push_back(mat4f(1.0f));
  global.push_back(mat4f(1.0f));
  
  ++size;
}

void Reignite::TransformComponents::update() {

  for (u32 i = 0; i < size; ++i) {

    glm::mat4 mat_transform = glm::translate(glm::mat4(1.0f), position[i]);
    glm::mat4 mat_scale = glm::scale(scale[i]);

    glm::mat4 mat_rotation_x = glm::rotate(glm::radians(rotation[i].x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 mat_rotation_y = glm::rotate(glm::radians(rotation[i].y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 mat_rotation_z = glm::rotate(glm::radians(rotation[i].z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 mat_rotation = mat_rotation_x * mat_rotation_y * mat_rotation_z;
  
    local[i] = mat_transform * mat_rotation * mat_scale;
    global[i] = local[i];
  }

  /*if (nullptr != tf.parent_world) {
    tf.global = *(tf.parent_world) * tf.local;
  }
  else { }*/
}