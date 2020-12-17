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

    data->camera.update(data->state->deltaTime);

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
    data->camera.setInputAccess(data->state);
    data->camera.setPerspective(60.0f, (float)state->window->width() / state->window->height(), 0.1f, 256.0f);
    data->camera.updateViewMatrix();

    addEntityRender(); // wall 1
    addEntityRender(); // wall 2
    addEntityRender(); // floor
    addEntityRender(); // sphere


    data->transformComponents.position[0] = vec3f(-1.4f, -2.0f, 1.4f);
    data->transformComponents.position[1] = vec3f(2.7f, -1.0f, 1.2f);
    data->transformComponents.position[2] = vec3f(0.0f, 1.0f, 0.0f);
    data->transformComponents.position[3] = vec3f(0.5f, 0.0f, 0.5f);

    data->transformComponents.rotation[0] = vec3f(0.0f, 45.0f, 90.0f);
    data->transformComponents.rotation[1] = vec3f(0.0f, 315.0f, 270.0f);
    data->transformComponents.rotation[2] = vec3f(0.0f, 45.0f, 0.0f);
    data->transformComponents.rotation[3] = vec3f(0.0f, 0.0f, 0.0f);

    data->renderComponents.geometry[0] = 2;
    data->renderComponents.geometry[1] = 2;
    data->renderComponents.geometry[2] = 2;
    data->renderComponents.geometry[3] = 0;

    data->renderComponents.material[0] = 4;
    data->renderComponents.material[1] = 4;
    data->renderComponents.material[2] = 4;
    data->renderComponents.material[3] = 3;

    addEntityLight();
    addEntityLight();
    addEntityLight();

    data->transformComponents.position[4] = vec4f(0.5f, -2.0f, 1.5f, 1.0f);
    data->renderComponents.geometry[4] = 3;
    data->renderComponents.material[4] = 3;
    data->lightComponents.target[4] = vec4f(0.0f, 0.0f, 0.0f, 0.0f);
    data->lightComponents.color[4] = vec4f(1.0f, 1.0f, 1.0f, 1.0f);

    data->transformComponents.position[5] = vec4f(-1.0f, -2.0f, 0.0f, 1.0f);
    data->renderComponents.geometry[5] = 3;
    data->renderComponents.material[5] = 3;
    data->lightComponents.target[5] = vec4f(0.0f, 0.0f, 0.0f, 0.0f);
    data->lightComponents.color[5] = vec4f(1.0f, 0.0f, 0.0f, 1.0f);

    data->transformComponents.position[6] = vec4f(2.5f, -2.0f, 0.0f, 1.0f);
    data->renderComponents.geometry[6] = 3;
    data->renderComponents.material[6] = 3;
    data->lightComponents.target[6] = vec4f(0.0f, 0.0f, 0.0f, 0.0f);
    data->lightComponents.color[6] = vec4f(0.0f, 0.0f, 1.0f, 1.0f);

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
