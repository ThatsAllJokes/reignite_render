#ifndef _RI_LIGHT_COMPONENT_
#define _RI_LIGHT_COMPONENT_ 1

#include "../basic_types.h"

#include "base_component.h"


namespace Reignite {

  struct LightComponents : public BaseComponents {

    void init(u32 maxSize);
    void clear();

    void add(vec3f direction);

    void update();
    
    enum Type {
      RI_LIGHT_TYPE_DIRECTIONAL,
      RI_LIGHT_TYPE_POINTLIGHT,
      RI_LIGHT_TYPE_SPOTLIGHT
    };

    std::vector<Type> type;
    std::vector<vec3f> target;
    std::vector<vec3f> color;
    std::vector<mat4f> view;
    std::vector<mat4f> projection;

    float zFar = 120.0f;
    float zNear = 0.1f;
    float fov = 45.0f;
    float aspect = 4.0f / 5.0f;
  };

}

#endif // _RI_LIGHT_COMPONENT_
