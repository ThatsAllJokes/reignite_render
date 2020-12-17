#ifndef _RI_CAMERA_COMPONENT_
#define _RI_CAMERA_COMPONENT_ 1

#include <memory>

#include "../basic_types.h"


namespace Reignite {

  struct State;

  struct Camera {

    void setInputAccess(const std::shared_ptr<State> state);

    void updateViewMatrix();

    void setPerspective(float fov, float aspect, float znear, float zfar);

    void update(float deltaTime);

    enum Type {
      RI_CAMERA_TYPE_PERSPECTIVE,
      RI_CAMERA_TYPE_ORTHOGONAL
    };

    Type type = Type::RI_CAMERA_TYPE_PERSPECTIVE;
    vec3f position = vec3f();
    vec3f direction = vec3f(0.0f, 0.0f, 1.0f);

    mat4f view = mat4f(1.0f);
    mat4f projection = mat4f(1.0f);

    float fov;
    float Znear;
    float Zfar;

    float movementSpeed = 1.0f;
    float rotationSpeed = 1.0f;

    bool flipY = false;

    std::shared_ptr<State> state;
  };

}



#endif // _CAMERA_COMPONENT_