#ifndef _TRANSFORM_COMPONENT_
#define _TRANSFORM_COMPONENT_ 1

#include "../basic_types.h"

#include "base_component.h"


namespace Reignite {

  struct TransformComponents : public BaseComponents {

    void UpdateTransformComponents();
    
    std::vector<vec3f> positions = {};
    std::vector<vec3f> rotations = {};
    std::vector<vec3f> scales = {};
    std::vector<mat4f> locals = {};
    std::vector<mat4f> globals = {};
  };

}

#endif // _TRANSFORM_COMPONENT_