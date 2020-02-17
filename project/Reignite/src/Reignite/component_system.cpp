#include "component_system.h"

#include <string>
#include <vector>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Components/transform_component.h"
#include "Components/render_component.h"
#include "Components/light_component.h"
#include "Components/camera_component.h"

namespace Reignite {

  struct State {

    std::string title;
    u16 width;
    u16 height;

    GLFWwindow* window;

    std::vector<u32> indices;

    CameraComponent camera;
    std::vector<TransformComponent> transform_components;
    std::vector<RenderComponent> render_components;
    std::vector<LightComponent> light_components;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  struct ComponentSystem::Data {

    ComponentSystemParams params;
  };

  Reignite::ComponentSystem::ComponentSystem(const std::shared_ptr<State> s) {

    data = new Data();

    initialize(s);
  }

  Reignite::ComponentSystem::~ComponentSystem() {
  
    shutdown();

    delete data;
  }

  void Reignite::ComponentSystem::update() {

    UpdateCameraComponent(state->camera);
    std::for_each(state->transform_components.begin(), state->transform_components.end(), UpdateTransformComponent);
    std::for_each(state->render_components.begin(), state->render_components.end(), UpdateRenderComponent);
    std::for_each(state->light_components.begin(), state->light_components.end(), UpdateLightComponent);
  }

  void Reignite::ComponentSystem::initialize(const std::shared_ptr<State> s) {
    
    this->state = s;

    InitCameraComponent(state->camera);

    TransformComponent tc;
    InitTransformComponent(tc);
    state->transform_components.push_back(tc);

    RenderComponent rc;
    InitRenderComponent(rc);
    state->render_components.push_back(rc);

    LightComponent lc;
    InitLightComponent(lc);
    state->light_components.push_back(lc);
  }

  void Reignite::ComponentSystem::shutdown() {}

}
