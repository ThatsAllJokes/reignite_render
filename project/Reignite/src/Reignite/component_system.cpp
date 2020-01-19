#include "component_system.h"

#include <string>
#include <vector>
#include <algorithm>

#include "Components/transform_component.h"
#include "Components/camera_component.h"

namespace Reignite {

  struct State {

    std::string title;
    u16 width;
    u16 height;

    std::vector<u32> indices;

    CameraComponent camera;
    std::vector<TransformComponent> transforms;
    //std::vector<GeometryComponent> geometries;
    //std::vector<MaterialComponent> materials;
    //std::vector<RenderComponent> renders;

    //std::vector<Geometry> db_geometries;
    //std::vector<Material> db_materials;
    //std::vector<Texture> db_textures;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };
}

Reignite::ComponentSystem::ComponentSystem(const State* state) {

  initialize(state);
}

Reignite::ComponentSystem::~ComponentSystem() {}

void Reignite::ComponentSystem::update() {

  //std::for_each(state->transforms.begin(), state->transforms.end(), UpdateTransformComponent);
}

bool Reignite::ComponentSystem::initialize(const State* state) { return false; }

void Reignite::ComponentSystem::shutdown() {}

