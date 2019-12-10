#include "application.h"


Reignite::Application::Application() {

  window = std::unique_ptr<Window>(Window::Create(&state));
  is_running = true;

  component_system = std::unique_ptr<ComponentSystem>(new ComponentSystem(&state));
}

Reignite::Application::~Application() { }

void Reignite::Application::Run() {

  while (is_running) {

    window->update();
    component_system->update();
    component_system->draw();
  }
}




