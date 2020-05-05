#include "application.h"

#include <chrono>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "timer.h"
#include "state.h"

#include "Components/transform_component.h"
#include "Components/render_component.h"
#include "Components/light_component.h"
#include "Components/camera_component.h"


namespace Reignite {

  Application::Application() {

    initialize();
  }

  Application::~Application() {

    shutdown();
  }

  void Application::Run() {

    while (!state->input->isKeyDown(256) && !window->closeWindow() && is_running) {

      Reignite::Timer::StartTime();

      window->update();
      component_system->update();

      render_context->drawScene();
      render_context->drawOverlay();

      auto timeDiff = Reignite::Timer::EndTime();
      state->frameTimer = (float)timeDiff / 1000.0f;
    }
  }

  void Application::initialize() {

    state = std::shared_ptr<State>(new State());

    window = std::unique_ptr<Window>(Window::Create(state));
    Input::setupKeyInputs(*window.get());
    state->window = std::make_unique<Window>(*window.get());

    component_system = std::unique_ptr<ComponentSystem>(new ComponentSystem(state));
    state->compSystem = std::make_unique<ComponentSystem>(*component_system.get());

    render_context = std::unique_ptr<RenderContext>(new RenderContext(state));

    is_running = true;
  }

  void Application::shutdown() {

    is_running = false;
  }

}




