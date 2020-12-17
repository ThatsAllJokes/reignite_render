#ifndef _VULKAN_IMPL_
#define _VULKAN_IMPL_ 1

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <array>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <gtc/matrix_transform.hpp>

#include <volk.h>

#include "vulkan_initializers.h"
#include "vulkan_swapchain.h"
#include "vulkan_texture.h"
#include "vulkan_buffer.h"

#include "../basic_types.h"
#include "../log.h"


// New implementation start --->

VkResult CreateRenderPass(VkDevice device, VkRenderPass& renderPass,
  std::vector<VkAttachmentReference>& colorReference,
  std::vector<VkAttachmentReference>& depthReference,
  std::vector<VkAttachmentDescription>& attachmentDescriptions);

VkResult CreateFramebuffer(VkDevice device, VkFramebuffer& framebuffer, 
  VkRenderPass renderPass, u32 width, u32 height, 
  std::vector<VkImageView>& attachments);

VkResult CreateSampler(VkDevice device, VkSampler& sampler);

struct PipelineCreateInfo {

  VkFrontFace frontFace;
  std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
  std::array<std::string, 2> filenames;
  std::array<VkShaderStageFlagBits, 2> stages;
  VkPipelineLayout pipelineLayout;
  VkRenderPass renderPass;
  VkPipelineDepthStencilStateCreateInfo depthStencilState;
  VkPipelineVertexInputStateCreateInfo vertexInputState;
  VkPipelineCache pipelineCache;
};

VkResult CreateGraphicsPipeline(VkDevice device, VkPipeline& pipeline, PipelineCreateInfo& pipelineInfo);

VkResult CreateDescriptorPool(VkDevice device, VkDescriptorPool& descriptorPool,
  std::vector<VkDescriptorPoolSize>& poolSizes);



void GenerateDeferredDebugQuads(vk::VulkanState* vulkanState,
  vk::Buffer* vertices, vk::Buffer* indices, u32& indexCount);

void GenerateShadowDebugQuads(vk::VulkanState* vulkanState,
  vk::Buffer* vertices, vk::Buffer* indices, u32& indexCount);



void DefineTexturedPBRPipeline();

// New implementation end <---

// VERTEX DEFINITION //////////////////////////////////////////////////////////////////////

struct Vertex {
  float position[3];
  float normal[3];
  float texcoord[2];
  float color[3];
  float tangent[3];
  //float bitangent[3];

  static std::vector<VkVertexInputBindingDescription> getBindingDescription() {

    std::vector<VkVertexInputBindingDescription> bindingDescription = {
    
      vk::initializers::VertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
    };

    return bindingDescription;
  }

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescription() {

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
    
      vk::initializers::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),
      vk::initializers::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),
      vk::initializers::VertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6),
      vk::initializers::VertexInputAttributeDescription(0, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),
      vk::initializers::VertexInputAttributeDescription(0, 4, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 11)
    };

    return attributeDescriptions;
  }
};

// DEVICE ////////////////////////////////////////////////////////////////////////////////

VkInstance createInstance();

VkBool32 debugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
  size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

VkDebugReportCallbackEXT registerDebugCallback(VkInstance instance);

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, std::string* physicalDeviceNames,
  uint32_t physicalDeviceCount, VkSampleCountFlagBits& msaaSamples);

VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool,
  VkQueue graphicsQueue);

// Deferred Tool functions ///////////////////////////////////////////////////////////////////////////////

inline VkPipelineShaderStageCreateInfo loadShader(VkDevice device,
  std::string filename, VkShaderStageFlagBits stage) {

  VkPipelineShaderStageCreateInfo shaderStage = {};
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = stage;
  shaderStage.module = vk::tools::loadShader(device, filename.c_str());
  shaderStage.pName = "main";

  assert(shaderStage.module != VK_NULL_HANDLE);

  return shaderStage;
}

#endif // _VULKAN_IMPL_

