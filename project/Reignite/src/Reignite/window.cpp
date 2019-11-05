#include "window.h"

#include "log.h"

namespace Reignite {

  static bool s_glfw_initialized = false;

  Window* Window::Create(const WindowParams& params) {
    
    return new Window(params);
  }

  Window::Window(const WindowParams& params) {

    init(params);
  }

  Window::~Window() {
  
    shutdown();
  }

  void Window::init(const WindowParams& p) {

    params.title = p.title;
    params.width = p.width;
    params.height = p.height;

    RI_INFO("Creating window {0} ({1}, {2})", p.title, p.width, p.height);

    if(!s_glfw_initialized) {
    
      s16 success = glfwInit();
      assert(success);

      s_glfw_initialized = true;
    }

    window = glfwCreateWindow((s16)p.width, (s16)p.height, p.title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &params);
  }

  void Window::shutdown() {

    glfwDestroyWindow(window);
  }

  void Window::update() {

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

}