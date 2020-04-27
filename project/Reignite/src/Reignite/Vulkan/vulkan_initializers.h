#ifndef _RI_VULKAN_INITIALIZERS_
#define _RI_VULKAN_INITIALIZERS_ 1

#include <vector>

#include "../basic_types.h"

#include "vulkan/vulkan.h"


namespace vk {
namespace initializers {

  inline VkMemoryAllocateInfo MemoryAllocateInfo() {

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    return allocateInfo;
  }

  inline VkMappedMemoryRange MappedMemoryRange() {

    VkMappedMemoryRange memoryRange = {};
    memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    return memoryRange;
  }

  inline VkCommandBufferAllocateInfo CommandBufferAllocateInfo(
    VkCommandPool commandPool, VkCommandBufferLevel level, u32 bufferCount) {

    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = level;
    allocateInfo.commandBufferCount = bufferCount;
    return allocateInfo;
  }

  inline VkCommandPoolCreateInfo CommandPoolCreateInfo() {

    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    return createInfo;
  }

  inline VkCommandBufferBeginInfo CommandBufferBeginInfo() {

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    return beginInfo;
  }

  inline VkRenderPassCreateInfo RenderPassCreateInfo() {

    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    return createInfo;
  }

  inline VkRenderPassBeginInfo RenderPassBeginInfo() {

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    return beginInfo;
  }

  inline VkImageMemoryBarrier ImageMemoryBarrier() {

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    return imageMemoryBarrier;
  }

  inline VkBufferMemoryBarrier BufferMemoryBarrier() {

    VkBufferMemoryBarrier bufferMemoryBarrier = {};
    bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    return bufferMemoryBarrier;
  }

  inline VkMemoryBarrier MemoryBarrier() {

    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    return memoryBarrier;
  }

  inline VkSamplerCreateInfo SamplerCreateInfo() {

    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.maxAnisotropy = 1;
    return createInfo;
  }

  inline VkImageCreateInfo ImageCreateInfo() {

    VkImageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    return createInfo;
  };

  inline VkImageViewCreateInfo ImageViewCreateInfo() {

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    return createInfo;
  }

  inline VkSemaphoreCreateInfo SemaphoreCreateInfo() {

    VkSemaphoreCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    return createInfo;
  }

  inline VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0) {

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = flags;
    return fenceCreateInfo;
  }

  inline VkSubmitInfo SubmitInfo() {

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    return submitInfo;
  }

  inline VkViewport Viewport(float width, float height, float minDepth, float maxDepth) {

    VkViewport viewport = {};
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    return viewport;
  }

  inline VkRect2D Rect2D(s32 width, s32 height, s32 offsetX, s32 offsetY) {

    VkRect2D rect = {};
    rect.extent.width = width;
    rect.extent.height = height;
    rect.offset.x = offsetX;
    rect.offset.y = offsetY;
    return rect;
  }

  inline VkBufferCreateInfo BufferCreateInfo() {

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    return createInfo;
  }

  inline VkBufferCreateInfo BufferCreateInfo(
    VkBufferUsageFlags usage, VkDeviceSize size) {

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.usage = usage;
    createInfo.size = size;
    return createInfo;
  }

  inline VkDescriptorPoolSize DescriptorPoolSize(
    VkDescriptorType type, u32 descriptorCount) {

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = descriptorCount;
    return poolSize;
  }

  inline VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(
    u32 poolSizeCount, VkDescriptorPoolSize* pPoolSizes, u32 maxSets) {

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = poolSizeCount;
    createInfo.pPoolSizes = pPoolSizes;
    createInfo.maxSets = maxSets;
    return createInfo;
  }

  inline VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(
    const std::vector<VkDescriptorPoolSize>& poolSizes, u32 maxSets) {

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.maxSets = maxSets;
    return createInfo;
  }

  inline VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type,
    VkShaderStageFlags stageFlags, u32 binding, u32 descriptorCount = 1) {

    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.descriptorType = type;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.binding = binding;
    layoutBinding.descriptorCount = descriptorCount;
    return layoutBinding;
  }

  inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(
    const VkDescriptorSetLayoutBinding* pBindings, u32 bindingCount) {

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pBindings = pBindings;
    createInfo.bindingCount = bindingCount;
    return createInfo;
  }

  inline VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding>& bindings) {

    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pBindings = bindings.data();
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    return createInfo;
  }

  inline VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(u32 setLayoutCount = 1) {

    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = setLayoutCount;
    return createInfo;
  }

  inline VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(
    const VkDescriptorSetLayout* pSetLayouts, u32 setLayoutCount = 1) {

    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = setLayoutCount;
    createInfo.pSetLayouts = pSetLayouts;
    return createInfo;
  }

  inline VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool,
    const VkDescriptorSetLayout* pSetLayouts, u32 descriptorSetCount) {

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool  = descriptorPool;
    allocateInfo.pSetLayouts = pSetLayouts;
    allocateInfo.descriptorSetCount = descriptorSetCount;
    return allocateInfo;
  }

  inline VkDescriptorImageInfo DescriptorImageInfo(
    VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout) {

    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.sampler = sampler;
    descriptorImageInfo.imageView = imageView;
    descriptorImageInfo.imageLayout = imageLayout;
    return descriptorImageInfo;
  }

  inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, 
    u32 binding, VkDescriptorBufferInfo* pBufferInfo, u32 descriptorCount = 1) {
  
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = dstSet;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.pBufferInfo = pBufferInfo;
    writeDescriptorSet.descriptorCount = descriptorCount;
    return writeDescriptorSet;
  }

  inline VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type,
    u32 binding, VkDescriptorImageInfo* pImageInfo, u32 descriptorCount = 1) {

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = dstSet;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.pImageInfo = pImageInfo;
    writeDescriptorSet.descriptorCount = descriptorCount;
    return writeDescriptorSet;
  }

  inline VkVertexInputBindingDescription VertexInputBindingDescription(
    u32 binding, u32 stride, VkVertexInputRate inputRate) {

    VkVertexInputBindingDescription vertexInputBindDescription = {};
    vertexInputBindDescription.binding = binding;
    vertexInputBindDescription.stride = stride;
    vertexInputBindDescription.inputRate = inputRate;
    return vertexInputBindDescription;
  }

  inline VkVertexInputAttributeDescription VertexInputAttributeDescription(
    u32 binding, u32 location, VkFormat format, u32 offset) {

    VkVertexInputAttributeDescription vertexInputAttribDescription = {};
    vertexInputAttribDescription.location = location;
    vertexInputAttribDescription.binding = binding;
    vertexInputAttribDescription.format = format;
    vertexInputAttribDescription.offset = offset;
    return vertexInputAttribDescription;
  }

  inline VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreateInfo() {

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    return pipelineVertexInputStateCreateInfo;
  }

  inline VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags,
    VkBool32 primitiveRestartEnable) {

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
    pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssemblyStateCreateInfo.topology = topology;
    pipelineInputAssemblyStateCreateInfo.flags = flags;
    pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
    return pipelineInputAssemblyStateCreateInfo;
  }

  inline VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo(
    VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace,
    VkPipelineRasterizationStateCreateFlags flags = 0) {

    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
    pipelineRasterizationStateCreateInfo.cullMode = cullMode;
    pipelineRasterizationStateCreateInfo.frontFace = frontFace;
    pipelineRasterizationStateCreateInfo.flags = flags;
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    return pipelineRasterizationStateCreateInfo;
  }

  inline VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(
    VkColorComponentFlags colorWriteMask, VkBool32 blendEnable) {

    VkPipelineColorBlendAttachmentState attachmentState = {};
    attachmentState.colorWriteMask = colorWriteMask;
    attachmentState.blendEnable = blendEnable;
    return attachmentState;
  }

  inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(
    u32 attachmentCount, const VkPipelineColorBlendAttachmentState* pAttachments) {

    VkPipelineColorBlendStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments = pAttachments;
    return createInfo;
  }

  inline VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(
    const std::vector<VkPipelineColorBlendAttachmentState>& pAttachments) {

    VkPipelineColorBlendStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.attachmentCount = (u32)pAttachments.size();
    createInfo.pAttachments = pAttachments.data();
    return createInfo;
  }

  inline VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(
    VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp) {

    VkPipelineDepthStencilStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    createInfo.depthTestEnable = depthTestEnable;
    createInfo.depthWriteEnable = depthWriteEnable;
    createInfo.depthCompareOp = depthCompareOp;
    createInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    return createInfo;
  }

  inline VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(
    u32 viewportCount, u32 scissorCount, VkPipelineViewportStateCreateFlags flags = 0) {

    VkPipelineViewportStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.viewportCount = viewportCount;
    createInfo.scissorCount = scissorCount;
    createInfo.flags = flags;
    return createInfo;
  }

  inline VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(
    VkSampleCountFlagBits rasterizationSamples, 
    VkPipelineMultisampleStateCreateFlags flags = 0) {

    VkPipelineMultisampleStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.rasterizationSamples = rasterizationSamples;
    createInfo.flags = flags;
    return createInfo;
  }

  inline VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(
    const VkDynamicState* pDynamicStates, u32 dynamicStateCount,
    VkPipelineDynamicStateCreateFlags flags = 0) {

    VkPipelineDynamicStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    createInfo.pDynamicStates = pDynamicStates;
    createInfo.dynamicStateCount = dynamicStateCount;
    createInfo.flags = flags;
    return createInfo;
  }

  inline VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(
    const std::vector<VkDynamicState>& dynamicStates,
    VkPipelineDynamicStateCreateFlags flags = 0) {

    VkPipelineDynamicStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    createInfo.pDynamicStates = dynamicStates.data();
    createInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    createInfo.flags = flags;
    return createInfo;
  }

  inline VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo() {

    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType =  VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.basePipelineIndex = -1;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    return createInfo;
  }

  inline VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo(
    VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCreateFlags flags = 0) {

    VkGraphicsPipelineCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.layout = layout;
    createInfo.renderPass = renderPass;
    createInfo.flags = flags;
    createInfo.basePipelineIndex = -1;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    return createInfo;
  }

  inline VkPushConstantRange PushConstantRange(
    VkShaderStageFlags stageFlags, u32 size, u32 offset) {

    VkPushConstantRange constRange = {};
    constRange.stageFlags = stageFlags;
    constRange.offset = offset;
    constRange.size = size;
    return constRange;
  }

}} // end of vk::initializers namespace

#endif // _RI_VULKAN_INITIALIZERS_