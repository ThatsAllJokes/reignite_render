#include "window.h"

#include <string>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "log.h"
#include "state.h"
#include "consts.h"


namespace Reignite {

  static bool s_glfw_initialized = false;
  std::vector<Input*> Reignite::Input::s_instances;


  // WINDOW /////////////////////////////////////////////////////////////////////////////

  struct Window::Data {

    u32 width;
    u32 height;
    std::string title;

    GLFWwindow* window;
    vec2f mousePos;
    Input* input;

    double mX; // avoid continously creating memory
    double mY;
  };

  Window* Window::Create(const std::shared_ptr<State> s) {
    
    return new Window(s);
  }

  Window::Window(const std::shared_ptr<State> s) {

    data = new Data();
    data->width = 1600;
    data->height = 900;
    data->title = "Reignite Render";
    data->window = nullptr;
    data->mousePos = vec2f();

    data->mX = data->mY = 0.0;

    std::vector<s32> temp_keys = { 
      RI_KEY_Q, RI_KEY_W, RI_KEY_E, 
      RI_KEY_A, RI_KEY_S, RI_KEY_D,
      RI_KEY_ESCAPE
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

    RI_INFO("Window created successfuly!");
  }

  void Window::shutdown() {

    glfwDestroyWindow(data->window);
  }

  void Window::update() {

    glfwPollEvents();

    glfwGetCursorPos(data->window, &data->mX, &data->mY);
    data->mousePos.x = (float)data->mX;
    data->mousePos.y = (float)data->mY;
  }

  bool Window::closeWindow() {

    return glfwWindowShouldClose(data->window);
  }

  inline const u16 Window::width() { return data->width; }

  inline const u16 Window::height() { return data->height; }

  inline const std::string Window::title() { return data->title; }

  inline const vec2f Window::mousePosition() { return data->mousePos; }

  inline void* const Window::currentWindow() { return data->window; }


  // INPUT ////////////////////////////////////////////////////////////////////////////////

  struct Input::Data {

    std::map<s32, bool> keys;
    std::map<s32, bool> buttons;
    bool isEnabled;
  };

  Input::Input(std::vector<s32> keysToMonitor) {

    data = new Data();
    data->isEnabled = true;

    for (s32 key : keysToMonitor)
      data->keys[key] = false;

    std::vector<s32> buttons = { 0, 1, 2 };
    for (s32 button : buttons)
      data->buttons[button] = false;

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

  bool Input::isMouseButtonDown(s32 button) {

    bool result = false;
    if (data->isEnabled) {

      std::map<s32, bool>::iterator it = data->buttons.find(button);
      if (it != data->buttons.end()) {
        result = data->buttons[button];
      }
    }

    return result;
  }

  inline bool Input::isEnabled() { return data->isEnabled; }

  inline void Input::setIsEnabled(bool value) { data->isEnabled = value; }

  void Input::setupKeyInputs(Window& window) {

    glfwSetKeyCallback((GLFWwindow*)window.currentWindow(), Input::callback);
    glfwSetMouseButtonCallback((GLFWwindow*)window.currentWindow(), Input::mouseCB);
  }

  void Input::setIsKeyDown(s32 key, bool isDown) {

    std::map<s32, bool>::iterator it = data->keys.find(key);
    if (it != data->keys.end()) {
      data->keys[key] = isDown;
    }
  }

  void Input::setIsMouseButtonDown(s32 button, bool isDown) {

    std::map<s32, bool>::iterator it = data->buttons.find(button);
    if (it != data->buttons.end()) {
      data->buttons[button] = isDown;
    }
  }

  void Input::callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods) {

    for (Input* input : s_instances) {
      input->setIsKeyDown(key, action != GLFW_RELEASE);
    }
  }

  void Input::mouseCB(GLFWwindow* window, s32 button, s32 action, s32 mods) {
    
    for (Input* input : s_instances) {
      input->setIsMouseButtonDown(button, action != GLFW_RELEASE);
    }
  }

} // end of Reignite namespace