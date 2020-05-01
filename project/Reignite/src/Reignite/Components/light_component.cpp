#include "light_component.h"

#define GLM_FORCE_RADIANS
#include <gtc/matrix_transform.hpp>


void Reignite::LightComponents::init(u32 maxSize) {

  used.reserve(maxSize);
  active.reserve(maxSize);

  type.reserve(maxSize);
  target.reserve(maxSize);
  color.reserve(maxSize);
  view.reserve(maxSize);
  projection.reserve(maxSize);
}

void Reignite::LightComponents::clear() {

  used.clear();
  active.clear();

  type.clear();
  target.clear();
  color.clear();
  view.clear();
  projection.clear();
}

void Reignite::LightComponents::add(vec3f direction) {

  used.push_back(true);
  active.push_back(true);

  type.push_back(RI_LIGHT_TYPE_POINTLIGHT);
  target.push_back(direction);
  color.push_back(vec3f(1.0f));
  view.push_back(mat4f(1.0f));
  projection.push_back(mat4f(1.0f));

  ++size;
}

void Reignite::LightComponents::update() {

}


