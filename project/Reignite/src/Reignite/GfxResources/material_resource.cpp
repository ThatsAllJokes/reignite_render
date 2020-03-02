#include "material_resource.h"


//glm::vec3(1.0f, 0.765557f, 0.336057f), 0.1f, 1.0f)
void CreateMaterial(Material& material) {
  material.name = "Albedo";
  material.params = {
    1.0f, 0.765557f, 0.336057f,
    0.1f, 1.0f };

  material.device = 0;
  material.uniformBuffer = { 0, 0 };
  material.lightParams = { 0, 0 };
  material.descriptorSet = 0;
}

void DestroyMaterial(Material& material) {

  destroyBuffer(material.device, material.uniformBuffer);
  destroyBuffer(material.device, material.lightParams);
}