#include "vulkan_overlay.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../tools.h"


vk::Overlay::Overlay() {

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.FontGlobalScale = scale;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
}

void vk::Overlay::prepareResources() {

  ImGuiIO& io = ImGui::GetIO();

  unsigned char* fontData;
  int texWidth, texHeight;
  std::string fontPath = (Reignite::Tools::GetAssetPath() + "fonts/Roboto-Medium.ttf");
  io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
  io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
  VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

  VkImageCreateInfo imageInfo = vk::initializers::ImageCreateInfo();
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo.extent.width = texWidth;
  imageInfo.extent.height = texHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VK_CHECK(vkCreateImage(state->device, &imageInfo, nullptr, &fontImage));
  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(state->device, fontImage, &memReqs);
  VkMemoryAllocateInfo memAllocInfo = vk::initializers::MemoryAllocateInfo();
  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = state->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK(vkAllocateMemory(state->device, &memAllocInfo, nullptr, &fontMemory));
  VK_CHECK(vkBindImageMemory(state->device, fontImage, fontMemory, 0));

  VkImageViewCreateInfo viewInfo = vk::initializers::ImageViewCreateInfo();
  viewInfo.image = fontImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.layerCount = 1;
  VK_CHECK(vkCreateImageView(state->device, &viewInfo, nullptr, &fontView));

  vk::Buffer stagingBuffer;

  VK_CHECK(state->createBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    uploadSize,
    &stagingBuffer));

  stagingBuffer.map();
  memcpy(stagingBuffer.mapped, fontData, uploadSize);
  stagingBuffer.unmap();

  VkCommandBuffer copyCmd = state->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

  vk::tools::SetImageLayout(
    copyCmd,
    fontImage,
    VK_IMAGE_ASPECT_COLOR_BIT,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_PIPELINE_STAGE_HOST_BIT,
    VK_PIPELINE_STAGE_TRANSFER_BIT);

  VkBufferImageCopy bufferCopyRegion = {};
  bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  bufferCopyRegion.imageSubresource.layerCount = 1;
  bufferCopyRegion.imageExtent.width = texWidth;
  bufferCopyRegion.imageExtent.height = texHeight;
  bufferCopyRegion.imageExtent.depth = 1;

  vkCmdCopyBufferToImage(
    copyCmd,
    stagingBuffer.buffer,
    fontImage,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &bufferCopyRegion
  );

  vk::tools::SetImageLayout(
    copyCmd,
    fontImage,
    VK_IMAGE_ASPECT_COLOR_BIT,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  state->flushCommandBuffer(copyCmd, queue, true);

  stagingBuffer.destroy();

  VkSamplerCreateInfo samplerInfo = vk::initializers::SamplerCreateInfo();
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK(vkCreateSampler(state->device, &samplerInfo, nullptr, &sampler));

  std::vector<VkDescriptorPoolSize> poolSizes = {
    vk::initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
  };
  VkDescriptorPoolCreateInfo descriptorPoolInfo = vk::initializers::DescriptorPoolCreateInfo(poolSizes, 2);
  VK_CHECK(vkCreateDescriptorPool(state->device, &descriptorPoolInfo, nullptr, &descriptorPool));

  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
    vk::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
  };
  VkDescriptorSetLayoutCreateInfo descriptorLayout = vk::initializers::DescriptorSetLayoutCreateInfo(setLayoutBindings);
  VK_CHECK(vkCreateDescriptorSetLayout(state->device, &descriptorLayout, nullptr, &descriptorSetLayout));

  VkDescriptorSetAllocateInfo allocInfo = vk::initializers::DescriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
  VK_CHECK(vkAllocateDescriptorSets(state->device, &allocInfo, &descriptorSet));
  VkDescriptorImageInfo fontDescriptor = vk::initializers::DescriptorImageInfo(
    sampler,
    fontView,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  );
  std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
    vk::initializers::WriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
  };
  vkUpdateDescriptorSets(state->device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void vk::Overlay::preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass) {

  VkPushConstantRange pushConstantRange = vk::initializers::PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::initializers::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
  VK_CHECK(vkCreatePipelineLayout(state->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
    vk::initializers::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

  VkPipelineRasterizationStateCreateInfo rasterizationState =
    vk::initializers::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

  VkPipelineColorBlendAttachmentState blendAttachmentState{};
  blendAttachmentState.blendEnable = VK_TRUE;
  blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlendState =
    vk::initializers::PipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

  VkPipelineDepthStencilStateCreateInfo depthStencilState =
    vk::initializers::PipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);

  VkPipelineViewportStateCreateInfo viewportState =
    vk::initializers::PipelineViewportStateCreateInfo(1, 1, 0);

  VkPipelineMultisampleStateCreateInfo multisampleState =
    vk::initializers::PipelineMultisampleStateCreateInfo(rasterizationSamples);

  std::vector<VkDynamicState> dynamicStateEnables = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };
  VkPipelineDynamicStateCreateInfo dynamicState =
    vk::initializers::PipelineDynamicStateCreateInfo(dynamicStateEnables);

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = 
    vk::initializers::GraphicsPipelineCreateInfo(pipelineLayout, renderPass);

  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pColorBlendState = &colorBlendState;
  pipelineCreateInfo.pMultisampleState = &multisampleState;
  pipelineCreateInfo.pViewportState = &viewportState;
  pipelineCreateInfo.pDepthStencilState = &depthStencilState;
  pipelineCreateInfo.pDynamicState = &dynamicState;
  pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaders.size());
  pipelineCreateInfo.pStages = shaders.data();
  pipelineCreateInfo.subpass = subpass;

  std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
    vk::initializers::VertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
  };
  std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
    vk::initializers::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),	// Location 0: Position
    vk::initializers::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),	  // Location 1: UV
    vk::initializers::VertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),	// Location 0: Color
  };
  VkPipelineVertexInputStateCreateInfo vertexInputState = 
    vk::initializers::PipelineVertexInputStateCreateInfo();
  vertexInputState.vertexBindingDescriptionCount = 
    static_cast<uint32_t>(vertexInputBindings.size());
  vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
  vertexInputState.vertexAttributeDescriptionCount = 
    static_cast<uint32_t>(vertexInputAttributes.size());
  vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

  pipelineCreateInfo.pVertexInputState = &vertexInputState;

  VK_CHECK(vkCreateGraphicsPipelines(state->device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

bool vk::Overlay::update() {

  ImDrawData* imDrawData = ImGui::GetDrawData();
  bool updateCmdBuffers = false;
  if (!imDrawData) { return false; };

  VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
  VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

  if ((vertexBufferSize == 0) || (indexBufferSize == 0))
    return false;

  if ((vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
    vertexBuffer.unmap();
    vertexBuffer.destroy();
    VK_CHECK(state->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize, &vertexBuffer));
    vertexCount = imDrawData->TotalVtxCount;
    vertexBuffer.unmap();
    vertexBuffer.map();
    updateCmdBuffers = true;
  }

  VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
  if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
    indexBuffer.unmap();
    indexBuffer.destroy();
    VK_CHECK(state->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferSize, &indexBuffer));
    indexCount = imDrawData->TotalIdxCount;
    indexBuffer.map();
    updateCmdBuffers = true;
  }

  ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
  ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

  for (int n = 0; n < imDrawData->CmdListsCount; n++) {
    const ImDrawList* cmd_list = imDrawData->CmdLists[n];
    memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtxDst += cmd_list->VtxBuffer.Size;
    idxDst += cmd_list->IdxBuffer.Size;
  }

  vertexBuffer.flush();
  indexBuffer.flush();

  return updateCmdBuffers;
}

void vk::Overlay::draw(const VkCommandBuffer commandBuffer) {

  ImDrawData* imDrawData = ImGui::GetDrawData();
  int32_t vertexOffset = 0;
  int32_t indexOffset = 0;

  if ((!imDrawData) || (imDrawData->CmdListsCount == 0))
    return;

  ImGuiIO& io = ImGui::GetIO();

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

  pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
  pushConstBlock.translate = glm::vec2(-1.0f);
  vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

  VkDeviceSize offsets[1] = { 0 };
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

  for (int32_t i = 0; i < imDrawData->CmdListsCount; i++) {

    const ImDrawList* cmd_list = imDrawData->CmdLists[i];
    for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {

      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
      VkRect2D scissorRect;
      scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
      scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
      scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
      scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
      vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
      vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
      indexOffset += pcmd->ElemCount;
    }

    vertexOffset += cmd_list->VtxBuffer.Size;
  }
}

void vk::Overlay::resize(u32 width, u32 height) {

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2((float)(width), (float)(height));
}

void vk::Overlay::freeResources() {

  ImGui::DestroyContext();
  vertexBuffer.destroy();
  indexBuffer.destroy();
  vkDestroyImageView(state->device, fontView, nullptr);
  vkDestroyImage(state->device, fontImage, nullptr);
  vkFreeMemory(state->device, fontMemory, nullptr);
  vkDestroySampler(state->device, sampler, nullptr);
  vkDestroyDescriptorSetLayout(state->device, descriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(state->device, descriptorPool, nullptr);
  vkDestroyPipelineLayout(state->device, pipelineLayout, nullptr);
  vkDestroyPipeline(state->device, pipeline, nullptr);
}

bool vk::Overlay::checkBox(const char* label, bool* value) {

  bool res = ImGui::Checkbox(label, value);
  if (res) { updated = true; }
  return res;
}

bool vk::Overlay::sliderFloat(const char* label, float* value, float min, float max) {

  bool res = ImGui::SliderFloat(label, value, min, max);
  if (res) { updated = true; }
  return res;
}
