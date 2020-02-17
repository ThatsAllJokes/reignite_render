#ifndef _RI_LIGHT_COMPONENT_
#define _RI_LIGHT_COMPONENT_ 1

#include "../basic_types.h"


namespace Reignite {

  struct LightComponent {

    enum Type {
      RI_LIGHT_TYPE_DIRECTIONAL,
      RI_LIGHT_TYPE_POINTLIGHT,
      RI_LIGHT_TYPE_SPOTLIGHT
    };

    LightComponent();

    Type type;
    vec3f color;
    vec3f position;
    vec3f direction;
    mat4f view_mat;
    mat4f projection_mat;

    float far_z;
    float near_z;
    float fov;
    float aspect;
  };

  void UpdateLightComponent(LightComponent& lc);
}

#endif // _RI_LIGHT_COMPONENT_

