#include "render_context.h"

Reignite::RenderContext::RenderContext(const State* state) {

  this->state = (State*)state;
}

Reignite::RenderContext::~RenderContext() {}

void Reignite::RenderContext::init(const RenderContextParams& params) {

  this->params = params;
}

void Reignite::RenderContext::finish() {

  render_should_close = true;
}