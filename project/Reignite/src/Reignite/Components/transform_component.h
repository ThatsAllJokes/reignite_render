#ifndef _TRANSFORM_COMPONENT_
#define _TRANSFORM_COMPONENT_ 1

#include "../basic_types.h"

#include "base_component.h"


namespace Reignite {

  struct TransformComponents : public BaseComponents {

    void init(u32 maxSize);
    void clear();

    void add(vec3f position);

    void update();
    
    std::vector<vec3f> position;
    std::vector<vec3f> rotation;
    std::vector<vec3f> scale;
    std::vector<mat4f> local;
    std::vector<mat4f> global;
  };

}

#endif // _TRANSFORM_COMPONENT_