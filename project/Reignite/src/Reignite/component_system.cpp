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

  void Reignite::ComponentSystem::addEntityEmpty() {

    addEntityEmpty(-1);
  }

  void Reignite::ComponentSystem::addEntityEmpty(s32 parentId) {

    data->entities.entity.push_back(data->entities.size++); // add entity and increment count
    data->entities.parent.push_back(parentId);

    data->renderComponents.add();
    data->renderComponents.used[data->renderComponents.size - 1] = false;

    data->transformComponents.add(vec3f(0.0f));
    data->lightComponents.add(vec3f(0.0f, 0.0f, 1.0f));
    data->lightComponents.used[data->lightComponents.size - 1] = false;
  }

  void Reignite::ComponentSystem::addEntityRender() {

    addEntityRender(-1);
  }

  void Reignite::ComponentSystem::addEntityRender(s32 parentId) {

    data->entities.entity.push_back(data->entities.size++); // add entity and increment count
    data->entities.parent.push_back(parentId);

    data->renderComponents.add();

    data->transformComponents.add(vec3f(0.0f));
    data->lightComponents.add(vec3f(0.0f, 1.0f, 0.0f));
    data->lightComponents.used[data->lightComponents.size - 1] = false;
  }

  void Reignite::ComponentSystem::addEntityLight() {

    addEntityLight(-1);
  }

  void Reignite::ComponentSystem::addEntityLight(s32 parentId) {

    data->entities.entity.push_back(data->entities.size++); // add entity and increment count
    data->entities.parent.push_back(parentId);

    data->renderComponents.add();
    //data->renderComponents.used[data->renderComponents.size - 1] = false;

    data->transformComponents.add(vec3f(0.0f));
    data->lightComponents.add(vec3f(0.0f, 0.0f, 1.0f));
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
    data->camera.position = { 2.15f, 0.0f, -8.75f };
    data->camera.rotation = glm::vec3(-0.75f, 12.5f, 0.0f);
    data->camera.setInputAccess(data->state);
    data->camera.setPerspective(60.0f, (float)state->window->width() / state->window->height(), 0.1f, 256.0f);
    data->camera.updateViewMatrix();

    addEntityRender();
    addEntityRender();

    data->transformComponents.position[1] = vec3f(0.0f, 1.0f, 0.0f);

    data->renderComponents.geometry[0] = 0;
    data->renderComponents.material[0] = 3;
    //data->renderComponents.texture[0] = { 0, 1, 2, 3 };

    data->renderComponents.geometry[1] = 2;
    data->renderComponents.material[1] = 4;
    //data->renderComponents.texture[0] = { 4, 5, 6, 7 };

    addEntityLight();
    addEntityLight();
    addEntityLight();

    data->transformComponents.position[2] = vec4f(0.0f, -3.0f, 0.0f, 1.0f);
    data->renderComponents.geometry[2] = 0;
    data->renderComponents.material[2] = 4;
    data->lightComponents.target[2] = vec4f(0.0f, 0.0f, 1.0f, 0.0f);
    data->lightComponents.color[2] = vec4f(1.0f, 1.0f, 1.0f, 1.0f);

    data->transformComponents.position[3] = vec4f(-4.0f, -3.0f, -4.0f, 1.0f);
    data->renderComponents.geometry[3] = 0;
    data->renderComponents.material[3] = 4;
    data->lightComponents.target[3] = vec4f(0.0f, 0.0f, 1.0f, 0.0f);
    data->lightComponents.color[3] = vec4f(1.0f, 0.0f, 0.0f, 1.0f);

    data->transformComponents.position[4] = vec4f(4.0f, -3.0f, 4.0f, 1.0f);
    data->renderComponents.geometry[4] = 0;
    data->renderComponents.material[4] = 4;
    data->lightComponents.target[4] = vec4f(0.0f, 0.0f, 1.0f, 0.0f);
    data->lightComponents.color[4] = vec4f(0.0f, 1.0f, 0.0f, 1.0f);

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
