#include "component_system.h"

#include <string>
#include <vector>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Components/transform_component.h"
#include "Components/camera_component.h"

namespace Reignite {

  struct State {

    std::string title;
    u16 width;
    u16 height;

    GLFWwindow* window;

    std::vector<u32> indices;

    CameraComponent camera;
    std::vector<TransformComponent> transforms;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  struct ComponentSystem::Data {

    u16 sample;
  };

  Reignite::ComponentSystem::ComponentSystem(const std::shared_ptr<State> state) {

    data = new Data();

    initialize(state);
  }

  Reignite::ComponentSystem::~ComponentSystem() {
  
    shutdown();

    delete data;
  }

  void Reignite::ComponentSystem::update() {

    //std::for_each(state->transforms.begin(), state->transforms.end(), UpdateTransformComponent);
  }

  void Reignite::ComponentSystem::initialize(const std::shared_ptr<State> state) {
    
    this->state = state;
  }

  void Reignite::ComponentSystem::shutdown() {}

}
