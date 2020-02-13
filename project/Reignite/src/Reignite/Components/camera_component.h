#ifndef _RI_CAMERA_COMPONENT_
#define _RI_CAMERA_COMPONENT_ 1

#include "../basic_types.h"


namespace Reignite {

  struct CameraComponent {

    enum Type {
      RI_CAMERA_TYPE_PERSPECTIVE,
      RI_CAMERA_TYPE_ORTHOGONAL
    };

    vec3f position;
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

typedef Reignite::CameraComponent CameraComp;

bool InitCameraComponent(CameraComp& tf);

void UpdateCameraComponent(CameraComp& tf);

#endif // _CAMERA_COMPONENT_