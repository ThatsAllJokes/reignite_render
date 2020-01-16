#include "window.h"

#include "log.h"

#include "Components/transform_component.h"

namespace Reignite {

  struct State {

    std::string title;
    u16 width;
    u16 height;

    std::vector<u32> indices;

    std::vector<TransformComponent> transforms;
    //std::vector<GeometryComponent> geometries;
    //std::vector<MaterialComponent> materials;
    //std::vector<RenderComponent> renders;
    //CameraComponent camera;

    //std::vector<Geometry> db_geometries;
    //std::vector<Material> db_materials;
    //std::vector<Texture> db_textures;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  static bool s_glfw_initialized = false;

  Window* Window::Create(const State* state) {
    
    return new Window(state);
  }

  Window::Window(const State* state) {

    init(state);
  }

  Window::~Window() {
  
    shutdown();
  }

  void Window::init(const State* p) {

    state = (State*)p; // TODO: Resolve this on the future (I do not like this casts)

    RI_INFO("Creating window . . . {0}: ({1}, {2})", p->title, p->width, p->height);

    if(!s_glfw_initialized) {
    
      s16 success = glfwInit();
      assert(success);

      s_glfw_initialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow((s16)p->width, (s16)p->height, p->title.c_str(), nullptr, nullptr);
    assert(window);

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &state);
  }

  void Window::shutdown() {

    glfwDestroyWindow(window);
  }

  void Window::update() {

    glfwPollEvents();
    //glfwSwapBuffers(window);
  }

  bool Window::closeWindow() {

    return glfwWindowShouldClose(window);
  }

  inline const u16 Window::getWidth() { return state->width; }

  inline const u16 Window::getHeight() { return state->height; }
}