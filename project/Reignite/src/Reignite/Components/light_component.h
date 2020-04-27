#ifndef _RI_LIGHT_COMPONENT_
#define _RI_LIGHT_COMPONENT_ 1

#include "../basic_types.h"

#include "base_component.h"


namespace Reignite {

  struct LightComponents : public BaseComponents {

    void UpdateLightComponents();
    
    enum Type {
      RI_LIGHT_TYPE_DIRECTIONAL,
      RI_LIGHT_TYPE_POINTLIGHT,
      RI_LIGHT_TYPE_SPOTLIGHT
    };

    Type type = RI_LIGHT_TYPE_SPOTLIGHT;
    vec3f position = vec3f();
    vec3f direction = vec3f();
    vec3f color = vec3f();
    mat4f view = mat4f(1.0f);
    mat4f projection = mat4f(1.0f);

    float zFar = 120.0f;
    float zNear = 0.1f;
    float fov = 45.0f;
    float aspect = 4.0f / 5.0f;
  };

}

#endif // _RI_LIGHT_COMPONENT_
