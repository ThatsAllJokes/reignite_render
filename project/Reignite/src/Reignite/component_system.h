#ifndef _RI_COMPONENT_SYSTEM_H_
#define _RI_COMPONENT_SYSTEM_H_ 1

#include <string>

#include "core.h"

namespace Reignite {

  struct State;

  /*struct Geometry {
    std::vector<Vertex> vertices;
    std::vector<u16> indices;
  };

  struct Material {
    std::string path;
  };

  struct Texture {
    std::string path;
  };*/

  /*class GeometryComponent {
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
  };*/

  class REIGNITE_API ComponentSystem {
   public:

    ComponentSystem(const State* state);
    ~ComponentSystem();

    void update();

   private:

    bool initialize(const State* state);
    void shutdown();

    State* state;
  };

} // end of Reignite namespace

#endif // _RI_COMPONENT_SYSTEM_H_



