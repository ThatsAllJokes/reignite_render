#include "transform_component.h"

bool InitTransformComponent(TransformComp& tf) {

  tf.position = vec3f(0.0f, 0.0f, 0.0f);
  tf.rotation = vec3f(0.0f, 0.0f, 0.0f);
  tf.scale = vec3f(0.0f, 0.0f, 0.0f);
  tf.model = mat4f(1.0f);
  tf.world = mat4f(1.0f);
  tf.parent_world = nullptr;

  return true;
}

void UpdateTransformComponent(TransformComp& tf) { }