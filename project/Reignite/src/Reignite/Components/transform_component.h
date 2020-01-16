#ifndef _TRANSFORM_COMPONENT_
#define _TRANSFORM_COMPONENT_ 1

#include <vector>

#include "../basic_types.h"

namespace Reignite {

  struct TransformComponent {

    vec3f position;
    vec3f rotation;
    vec3f scale;
    mat4f model;
    mat4f world;
    mat4f* parent_world;
  };
}

typedef Reignite::TransformComponent TransformComp;

bool InitTransformComponent(TransformComp& tf);

void UpdateTransformComponent(TransformComp& tf); //TODO: Implement transfomr update

#endif // _TRANSFORM_COMPONENT_