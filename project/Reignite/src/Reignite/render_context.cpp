#include "render_context.h"

#include "display_list.h"


struct Reignite::RenderContext::Data {

  RenderContextParams params;

  // Render state
  bool render_should_close;
};

Reignite::RenderContext::RenderContext(const State* state) {

  this->state = (State*)state;
  init(state); // TODO: This should be variable in the future
}

Reignite::RenderContext::~RenderContext() {

  shutdown();
}

void Reignite::RenderContext::submitDisplayList(Reignite::DisplayList* displayList) {

}

void Reignite::RenderContext::init(const State* state, const RenderContextParams& params) {

  data->params = params;
}

void Reignite::RenderContext::shutdown() {

  data->render_should_close = true;
}