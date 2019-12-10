#ifndef _RI_COMPONENT_SYSTEM_H_
#define _RI_COMPONENT_SYSTEM_H_ 1

#include <vector>
#include <string>

#include "core.h"
#include "basic_types.h"

namespace Reignite {

  struct State;

  struct Geometry {
    std::vector<vec3f> vertices;
    std::vector<u16> indices;
  };

  struct Material {
    std::string path;
  };

  struct Texture {
    std::string path;
  };

  class TransformComponent {
    vec4f position;
    vec4f rotation;
    vec4f scale;
    mat4f model;
    mat4f world;
    mat4f* parent_world;
  };

  class GeometryComponent {
    Geometry* mesh;
  };

  class MaterialComponent {
    Material* shader;
    Texture* texture;
  };

  class RenderComponent {
    bool visible;
    GeometryComponent* geometry;
    MaterialComponent* material;
  };

  class CameraComponent {
    float fov;
    float aspect;
    float near_z;
    float far_z;
    mat4f proj_matrix;
    mat4f view_matrix;
  };

  class LightComponent;

  class REIGNITE_API ComponentSystem {
   public:

    ComponentSystem(const State* state);
    ~ComponentSystem();

    void update();
    void draw();

   private:

    bool initialize(const State* state);
    void shutdown();

    void updateTransforms();
    void updateGeometries();
    void updateMaterials();
    void updateRenders();
    void updateCameras();
    void updateLights();

    State* state;
  };

} // end of Reignite namespace

#endif // _RI_COMPONENT_SYSTEM_H_



