#ifndef _RI_MATERIAL_RESOURCE_
#define _RI_MATERIAL_RESOURCE_ 1

#include <string>

#include "../basic_types.h"

#include "../Vulkan/vulkan_impl.h"

namespace Reignite {

  struct MaterialResource {

    MaterialResource() {}
    MaterialResource(std::string n, vec3f c, float r, float m) : name(n) {
      params.r = c.r;
      params.g = c.g;
      params.b = c.b;
      params.roughness = r;
      params.metallic = m;
    }

    std::string name;
    struct PushBlock {
      float r, g, b;
      float roughness;
      float metallic;
    } params;

    VkDescriptorSet descriptorSet;
    Buffer uniformBuffer;
  };

} // end of Reignite namespace

typedef Reignite::MaterialResource Material;

void CreateMaterial(Material& material);

void DestroyMaterial(Material& material);

#endif // _RI_MATERIAL_RESOURCE_

