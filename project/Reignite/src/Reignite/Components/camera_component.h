#ifndef _RI_CAMERA_COMPONENT_
#define _RI_CAMERA_COMPONENT_ 1

#include "../basic_types.h"


namespace Reignite {

  struct CameraComponent {

    enum Type {
      RI_CAMERA_TYPE_PERSPECTIVE,
      RI_CAMERA_TYPE_ORTHOGONAL
    };

    CameraComponent();

    Type type;
    vec3f forward;
    vec3f position;
    mat4f view_mat;
    mat4f projection_mat;

    float fov;
    float aspect;
    float near_z;
    float far_z;
  };

  void UpdateCameraComponent(CameraComponent& cam_comp);
}



#endif // _CAMERA_COMPONENT_