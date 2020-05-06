#include "render_component.h"


void Reignite::RenderComponents::init(u32 maxSize) {

  used.reserve(maxSize);
  active.reserve(maxSize);

  geometry.reserve(maxSize);
  material.reserve(maxSize);
}

void Reignite::RenderComponents::clear() {

  used.clear();
  active.clear();

  geometry.clear();
  material.clear();
}

void Reignite::RenderComponents::add() {

  used.push_back(true);
  active.push_back(true);

  geometry.push_back(0);
  material.push_back(0);

  ++size;
}

void Reignite::RenderComponents::update() {

}