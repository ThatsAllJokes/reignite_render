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

    Type type;
    vec3f forward;
    float fov;
    float aspect;
    float near_z;
    float far_z;
    float left;
    float right;
    float bottom;
    float top;
    mat4f view_mat;
    mat4f projection_mat;
  };
}

typedef Reignite::LightComponent LightComp;

bool InitLightComponent(LightComp& tf);

void UpdateLightComponent(LightComp& tf);

#endif // !_RI_LIGHT_COMPONENT_

