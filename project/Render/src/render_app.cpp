#include <reignite.h>

class Render : public Reignite::Application {
 public:

  Render() {}
  ~Render() {}
};

Reignite::Application* Reignite::CreateApplication() {

  return new Render();
}
