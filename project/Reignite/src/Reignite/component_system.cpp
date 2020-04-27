#include "component_system.h"

#include <string>
#include <vector>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "window.h"

#include "Components/transform_component.h"
#include "Components/render_component.h"
#include "Components/light_component.h"
#include "Components/camera_component.h"

namespace Reignite {

  struct State {

    float frameTimer;

    std::shared_ptr<Window> window;
    Input* input;

    vec2f mousePos;
    struct {
      bool left = false;
      bool right = false;
      bool middle = false;
    } mouseButtons;

    std::shared_ptr<ComponentSystem> compSystem;
  };

  struct ComponentSystem::Data {

    ComponentSystemParams params;

    struct Entity {
      std::vector<s32> entity;
      std::vector<s32> parent;
    } entities;

    Camera camera;
    TransformComponents transformComponents;
    RenderComponents renderComponents;
    LightComponents lightComponents;
  };

  Reignite::ComponentSystem::ComponentSystem(const std::shared_ptr<State> s) {

    data = new Data();

    initialize(s);
  }

  Reignite::ComponentSystem::~ComponentSystem() {
  
    shutdown();

    delete data;
  }

  vec3f Reignite::ComponentSystem::viewPosition() { return data->camera.position; };

  mat4f Reignite::ComponentSystem::view() { return data->camera.view; };

  mat4f Reignite::ComponentSystem::projection() { return data->camera.projection; };

  void Reignite::ComponentSystem::update() {

    data->camera.update(state->frameTimer);

    data->transformComponents.UpdateTransformComponents();
    data->renderComponents.UpdateRenderComponents();
    data->lightComponents.UpdateLightComponents();
  }

  void Reignite::ComponentSystem::initialize(const std::shared_ptr<State> s) {
    
    this->state = s;

    data->camera.position = { 2.15f, 0.3f, -8.75f };
    data->camera.rotation = glm::vec3(-0.75f, 12.5f, 0.0f);
    data->camera.setInputAccess(this->state);
    data->camera.setPerspective(60.0f, (float)state->window->width() / state->window->height(), 0.1f, 256.0f);
    data->camera.updateViewMatrix();

    //data->entities.entity.push_back((u32)data->entities.entity.size());
    //data->entities.parent.push_back((u32)data->entities.parent.size());

    /*TransformComponent tc;
    tc.position = vec3f(0.0f, 0.0f, 0.0f);
    data->transform_components.push_back(tc);

    RenderComponent rc;
    data->render_components.push_back(rc);

    LightComponent lc;
    lc.is_active = false;
    data->light_components.push_back(lc);*/

    update();
  }

  void Reignite::ComponentSystem::shutdown() {}

}
