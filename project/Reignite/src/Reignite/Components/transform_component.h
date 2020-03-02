#ifndef _TRANSFORM_COMPONENT_
#define _TRANSFORM_COMPONENT_ 1

#include "base_component.h"
#include "../basic_types.h"


namespace Reignite {

  struct TransformComponent : public BaseComponent {

    TransformComponent();
    TransformComponent(const TransformComponent& tc);

    vec3f position;
    vec3f rotation;
    vec3f scale;
    mat4f local;
    mat4f global;
    mat4f* parent_world;
  };

  void UpdateTransformComponent(TransformComponent& tc);
}

#endif // _TRANSFORM_COMPONENT_