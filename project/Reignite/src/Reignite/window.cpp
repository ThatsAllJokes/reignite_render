#include "window.h"

#include <string>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "log.h"

#include "Components/transform_component.h"
#include "Components/camera_component.h"

namespace Reignite {

  static bool s_glfw_initialized = false;

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

  struct Window::Data {

    GLFWwindow* window;
  };

  Window* Window::Create(const std::shared_ptr<State> s) {
    
    return new Window(s);
  }

  Window::Window(const std::shared_ptr<State> s) {

    data = new Data();

    initialize(s);
  }

  Window::~Window() {
  
    shutdown();
    
    delete data;
  }

  void Window::initialize(const std::shared_ptr<State> state) {

    this->state = state; // TODO: Resolve this on the future (I do not like this casts)

    RI_INFO("Creating window . . . {0}: ({1}, {2})", state->title, state->width, state->height);

    if(!s_glfw_initialized) {
    
      s16 success = glfwInit();
      assert(success);

      s_glfw_initialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    data->window = glfwCreateWindow((s16)state->width, (s16)state->height, state->title.c_str(), nullptr, nullptr);
    assert(data->window);

    state->window = data->window;

    glfwMakeContextCurrent(data->window);
    glfwSetWindowUserPointer(data->window, nullptr);
  }

  void Window::shutdown() {

    glfwDestroyWindow(data->window);
  }

  void Window::update() {

    glfwPollEvents();
    //glfwSwapBuffers(window);
  }

  bool Window::closeWindow() {

    return glfwWindowShouldClose(data->window);
  }

  inline const u16 Window::getWidth() { return state->width; }

  inline const u16 Window::getHeight() { return state->height; }

  //inline GLFWwindow* const Window::GetCurrentWindow() { return data->window; }
}