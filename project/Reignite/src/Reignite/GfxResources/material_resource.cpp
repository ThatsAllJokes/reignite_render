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
  colorMap = {};
  normalMap = {};
  roughness = {};
  metallic = {};
  descriptorSet = 0;
}

void Reignite::MaterialResource::destroy() {

  vulkanState = nullptr;
  colorMap.destroy();
  normalMap.destroy();
  roughness.destroy();
  metallic.destroy();
  //descriptorSet
}

void Reignite::MaterialResource::update(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout) {

  VkDescriptorSetAllocateInfo allocInfo = vk::initializers::DescriptorSetAllocateInfo(
    descriptorPool, &descriptorSetLayout, 1);

  VK_CHECK(vkAllocateDescriptorSets(vulkanState->device, &allocInfo, &descriptorSet));
  std::vector<VkWriteDescriptorSet> writeDescriptorSets = {

    // Binding 1: Color map
    vk::initializers::WriteDescriptorSet(descriptorSet,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &colorMap.descriptor),
    // Binding 2: Normal map
    vk::initializers::WriteDescriptorSet(descriptorSet,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalMap.descriptor),
    // Binding 3: Roughness map
    vk::initializers::WriteDescriptorSet(descriptorSet,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &roughness.descriptor),
    // Binding 4: Metallic map
    vk::initializers::WriteDescriptorSet(descriptorSet,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &metallic.descriptor)
  };

  vkUpdateDescriptorSets(vulkanState->device, static_cast<u32>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

}