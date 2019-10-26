#pragma once

#ifdef RI_PLATFORM_WINDOWS

extern Reignite::Application* Reignite::CreateApplication();

int main(int argc, char** argv) {

  auto app = Reignite::CreateApplication();
  app->Run();
  delete app;
}

#endif
