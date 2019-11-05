#include "application.h"


Reignite::Application::Application() {
  
  window = std::unique_ptr<Window>(Window::Create());
}

Reignite::Application::~Application() { }

void Reignite::Application::Run() {

  while (is_running) {

    window->update();
  }
}




