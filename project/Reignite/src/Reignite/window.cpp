#include "window.h"

#include <string>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "log.h"
#include "component_system.h"

#include "Components/transform_component.h"
#include "Components/render_component.h"
#include "Components/light_component.h"
#include "Components/camera_component.h"

namespace Reignite {

  static bool s_glfw_initialized = false;
  std::vector<Input*> Reignite::Input::s_instances;

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

    std::vector<ComponentSystem> compSystem;
  };

  struct Window::Data {

    u32 width;
    u32 height;
    std::string title;

    GLFWwindow* window;
    Input* input;
  };

  Window* Window::Create(const std::shared_ptr<State> s) {
    
    return new Window(s);
  }

  Window::Window(const std::shared_ptr<State> s) {

    data = new Data();
    data->width = 1280;
    data->height = 720;
    data->title = "Sample text";
    data->window = nullptr;

    std::vector<s32> temp_keys = { 
      GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E, 
      GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
      GLFW_KEY_ESCAPE
    };
    data->input = new Input(temp_keys);

    initialize(s);
  }

  Window::~Window() {
  
    shutdown();
    
    delete data;
  }

  void Window::initialize(const std::shared_ptr<State> state) {

    this->state = state;

    RI_INFO("Creating window . . . {0}: ({1}, {2})", data->title, data->width, data->height);

    if(!s_glfw_initialized) {
    
      s16 success = glfwInit();
      assert(success);

      s_glfw_initialized = true;
    }

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    data->window = glfwCreateWindow((s16)data->width, (s16)data->height, data->title.c_str(), nullptr, nullptr);
    assert(data->window);

    state->input = data->input;

    glfwMakeContextCurrent(data->window);
    glfwSetWindowUserPointer(data->window, nullptr);
  }

  void Window::shutdown() {

    glfwDestroyWindow(data->window);
  }

  void Window::update() {

    glfwPollEvents();
  }

  void Window::updateMouseInput() {

    double mX, mY;
    glfwGetCursorPos(data->window, &mX, &mY);

    state->mousePos.x = (float)mX;
    state->mousePos.y = (float)mY;

    int leftButtonState = glfwGetMouseButton(data->window, GLFW_MOUSE_BUTTON_LEFT);
    int rightButtonState = glfwGetMouseButton(data->window, GLFW_MOUSE_BUTTON_RIGHT);
    int middleButtonState = glfwGetMouseButton(data->window, GLFW_MOUSE_BUTTON_MIDDLE);

    state->mouseButtons.left = (leftButtonState == GLFW_PRESS);
    state->mouseButtons.right = (rightButtonState == GLFW_PRESS);
    state->mouseButtons.middle = (middleButtonState == GLFW_PRESS);
  }

  bool Window::closeWindow() {

    return glfwWindowShouldClose(data->window);
  }

  inline const u16 Window::width() { return data->width; }

  inline const u16 Window::height() { return data->height; }

  inline const std::string Window::title() { return data->title; }

  inline void* const Window::currentWindow() { return data->window; }

  
  Input::Input(std::vector<int> keysToMonitor) {

    data = new Data();
    data->isEnabled = true;

    for (s32 key : keysToMonitor) {
      data->keys[key] = false;
    }

    s_instances.push_back(this);
  }

  Input::~Input() {
  
    s_instances.erase(
      std::remove(s_instances.begin(), s_instances.end(), this),
      s_instances.end());
  }

  bool Input::isKeyDown(s32 key) {

    bool result = false;
    if (data->isEnabled) {
      
      std::map<s32, bool>::iterator it = data->keys.find(key);
      if (it != data->keys.end()) {
        result = data->keys[key];
      }
    }

    return result;
  }

  void Input::setIsKeyDown(s32 key, bool isDown) {

    std::map<s32, bool>::iterator it = data->keys.find(key);
    if (it != data->keys.end()) {
      data->keys[key] = isDown;
    }
  }

  void Input::setupKeyInputs(Window& window) {

    glfwSetKeyCallback((GLFWwindow*)window.currentWindow(), Input::callback);
  }

  void Input::callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods) {

    for (Input* input : s_instances) {
      input->setIsKeyDown(key, action != GLFW_RELEASE);
    }
  }

} // end of Reignite namespace