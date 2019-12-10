#ifndef _RI_APPLICATION_H_
#define _RI_APPLICATION_H_ 1

#include <memory>
#include <vector>

#include "core.h"
#include "window.h"

namespace Reignite {

  struct State {
    std::string title;
    u16 width;
    u16 height;

    std::vector<u32> indices;

    std::vector<TransformComponent> transforms;
    std::vector<GeometryComponent> geometries;
    std::vector<MaterialComponent> materials;
    std::vector<RenderComponent> renders;
    CameraComponent camera;

    std::vector<Geometry> db_geometries;
    std::vector<Material> db_materials;
    std::vector<Texture> db_textures;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  class REIGNITE_API Application {
   public:

    Application();
    virtual ~Application();

    void Run();

   protected:

    State state;

    bool is_running;
    std::unique_ptr<Window> window;

    std::unique_ptr<ComponentSystem> component_system;
  };

  // To be defined in client
  Application* CreateApplication();
}

#endif // _RI_APPLICATION_H_

