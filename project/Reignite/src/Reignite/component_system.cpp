#include "component_system.h"

#include <string>
#include <vector>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "state.h"

#include "Components/transform_component.h"
#include "Components/render_component.h"
#include "Components/light_component.h"
#include "Components/camera_component.h"

namespace Reignite {

  struct ComponentSystem::Data {

    std::shared_ptr<State> state;

    ComponentSystemParams params;

    struct Entity {
      u32 size = 0;
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

  void Reignite::ComponentSystem::addEntity() {

    data->entities.entity.push_back(data->entities.size++); // add entity and increment count
    data->entities.parent.push_back(-1); 

    data->transformComponents.add(vec3f(0.0f));
    data->lightComponents.add(vec3f(0.0f, 0.0f, 1.0f));
    data->lightComponents.used[data->lightComponents.size - 1] = false;

    data->renderComponents.add();
  }

  void Reignite::ComponentSystem::addEntity(s32 parentId) {

    data->entities.entity.push_back(data->entities.size++); // add entity and increment count
    data->entities.parent.push_back(parentId);

    data->transformComponents.add(vec3f(0.0f));
    data->lightComponents.add(vec3f(0.0f, 0.0f, 1.0f));

    data->renderComponents.add();
  }

  Camera* Reignite::ComponentSystem::camera() {

    return &data->camera;
  }

  TransformComponents* Reignite::ComponentSystem::transform() {

    return &data->transformComponents;
  }

  RenderComponents* Reignite::ComponentSystem::render() {

    return &data->renderComponents;
  }

  LightComponents* Reignite::ComponentSystem::light() {

    return &data->lightComponents;
  }

  void Reignite::ComponentSystem::update() {

    data->camera.update(data->state->frameTimer);

    data->transformComponents.update();
    data->renderComponents.update();
    data->lightComponents.update();
  }

  void Reignite::ComponentSystem::initialize(const std::shared_ptr<State> state) {
    
    data->state = state;

    data->entities.entity.reserve(data->params.max_entities);
    data->entities.parent.reserve(data->params.max_entities);

    data->transformComponents.init(data->params.max_entities);
    data->renderComponents.init(data->params.max_entities);
    data->lightComponents.init(data->params.max_entities);

    // initialize camera
    data->camera.position = { 2.15f, 0.3f, -8.75f };
    data->camera.rotation = glm::vec3(-0.75f, 12.5f, 0.0f);
    data->camera.setInputAccess(data->state);
    data->camera.setPerspective(60.0f, (float)state->window->width() / state->window->height(), 0.1f, 256.0f);
    data->camera.updateViewMatrix();

    addEntity();
    addEntity();


    update();
  }

  void Reignite::ComponentSystem::shutdown() {
    
    data->entities.entity.clear();
    data->entities.parent.clear();
    data->transformComponents.clear();
    data->renderComponents.clear();
    data->lightComponents.clear();
  }

}
