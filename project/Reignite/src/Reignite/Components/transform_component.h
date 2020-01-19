#ifndef _TRANSFORM_COMPONENT_
#define _TRANSFORM_COMPONENT_ 1

#include "../basic_types.h"

namespace Reignite {

  struct TransformComponent {

    vec3f position;
    vec3f rotation;
    vec3f scale;
    mat4f local;
    mat4f global;
    mat4f* parent_world;
  };
}

typedef Reignite::TransformComponent TransformComp;

bool InitTransformComponent(TransformComp& tf);

void UpdateTransformComponent(TransformComp& tf);

#endif // _TRANSFORM_COMPONENT_