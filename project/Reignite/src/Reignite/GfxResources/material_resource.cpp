#include "material_resource.h"


// Metallic material sample data
//glm::vec3(1.0f, 0.765557f, 0.336057f), 0.1f, 1.0f)

void Reignite::MaterialResource::init() {

  name = "Default";
  params.r = 1.0f;
  params.g = 1.0f;
  params.b = 1.0f;
  params.roughness = 1.0f;
  params.metallic = 1.0f;

  vulkanState = nullptr;
  uboBasics = {};
  uboLights = {};
  uboShadows = {};
  descriptorSet = 0;
}

void Reignite::MaterialResource::destroy() {

  vulkanState = nullptr;
  uboBasics.destroy();
  uboLights.destroy();
  uboShadows.destroy();
  //descriptorSet
}
