#ifndef _RI_VULKAN_OVERLAY_
#define _RI_VULKAN_OVERLAY_ 1

#include <imgui.h>
#include <volk.h>

#include  "../basic_types.h"

#include "../tools.h"

#include "vulkan_impl.h"
#include "vulkan_state.h"
#include "vulkan_buffer.h"


namespace vk {

  class Overlay {
   public:

    Overlay();
    ~Overlay() {}

    void prepareResources();
    void preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass);

    bool update();
    void draw(const VkCommandBuffer commandBuffer);
    void resize(u32 width, u32 height);

    void freeResources();

    bool checkBox(const char* label, bool* value);
    bool sliderFloat(const char* label, float* value, float min, float max);

    struct PushConstBlock {
      vec2f scale;
      vec2f translate;
    } pushConstBlock;

    vk::Buffer vertexBuffer;
    vk::Buffer indexBuffer;
    s32 vertexCount = 0;
    s32 indexCount = 0;

    std::vector<VkPipelineShaderStageCreateInfo> shaders;

    VkSampler sampler;
    VkImage fontImage = VK_NULL_HANDLE;
    VkImageView fontView = VK_NULL_HANDLE;
    VkDeviceMemory fontMemory = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;

    VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    uint32_t subpass = 0;

    vk::VulkanState* state;
    VkQueue queue;

    bool visible = true;
    bool updated = false;
    float scale = 1.0f;
  };

} // end of vk namespace

#endif // _RI_VULKAN_OVERLAY_

