#include "vulkan_impl.h"

#include <stb_image.h>

#include "../tools.h"

#include "../render_context.h"
#include "../GfxResources/material_resource.h"


// new implementation start --->

VkResult CreateRenderPass(VkDevice device, VkRenderPass& renderPass,
  std::vector<VkAttachmentReference>& colorReference, 
  std::vector<VkAttachmentReference>& depthReference,
  std::vector<VkAttachmentDescription>& attachmentDescriptions) {

  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = static_cast<u32>(colorReference.size());
  subpassDescription.pColorAttachments = colorReference.data();
  subpassDescription.pDepthStencilAttachment = depthReference.data();

  std::array<VkSubpassDependency, 2> subpassDependencies;
  subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependencies[0].dstSubpass = 0;
  subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  subpassDependencies[1].srcSubpass = 0;
  subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassCreateInfo = {};
  renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
  renderPassCreateInfo.attachmentCount = static_cast<u32>(attachmentDescriptions.size());
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpassDescription;
  renderPassCreateInfo.dependencyCount = 2;
  renderPassCreateInfo.pDependencies = subpassDependencies.data();

  return vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
}

VkResult CreateFramebuffer(VkDevice device, VkFramebuffer& framebuffer, VkRenderPass renderPass,
  u32 width, u32 height, std::vector<VkImageView>& attachments) {

  VkFramebufferCreateInfo fbufferCreateInfo = {};
  fbufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fbufferCreateInfo.pNext = NULL;
  fbufferCreateInfo.renderPass = renderPass;
  fbufferCreateInfo.pAttachments = attachments.data();
  fbufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  fbufferCreateInfo.width = width;
  fbufferCreateInfo.height = height;
  fbufferCreateInfo.layers = 1;

  return vkCreateFramebuffer(device, &fbufferCreateInfo, nullptr, &framebuffer);
}

VkResult CreateSampler(VkDevice device, VkSampler& sampler) {

  VkSamplerCreateInfo samplerCreateInfo = vk::initializers::SamplerCreateInfo();
  samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
  samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
  samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
  samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
  samplerCreateInfo.mipLodBias = 0.0f;
  samplerCreateInfo.maxAnisotropy = 1.0f;
  samplerCreateInfo.minLod = 0.0f;
  samplerCreateInfo.maxLod = 1.0f;
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

  return vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);
}

VkResult CreateGraphicsPipeline(VkDevice device, VkPipeline& pipeline, PipelineCreateInfo& pipelineInfo) {

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
    vk::initializers::PipelineInputAssemblyStateCreateInfo(
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

  VkPipelineRasterizationStateCreateInfo rasterizationState =
    vk::initializers::PipelineRasterizationStateCreateInfo(
      VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, pipelineInfo.frontFace, 0);

  VkPipelineColorBlendStateCreateInfo colorBlendState =
    vk::initializers::PipelineColorBlendStateCreateInfo(pipelineInfo.blendAttachmentStates);

  VkPipelineViewportStateCreateInfo viewportState =
    vk::initializers::PipelineViewportStateCreateInfo(1, 1, 0);

  VkPipelineMultisampleStateCreateInfo multisampleState =
    vk::initializers::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

  std::vector<VkDynamicState> dynamicStateEnables = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

  VkPipelineDynamicStateCreateInfo dynamicState =
    vk::initializers::PipelineDynamicStateCreateInfo(
      dynamicStateEnables.data(), static_cast<u32>(dynamicStateEnables.size()), 0);

  std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};
  shaderStages[0] = loadShader(device,
    Reignite::Tools::GetAssetPath() + "shaders/" + pipelineInfo.filenames[0] + ".spv",
    pipelineInfo.stages[0]);

  shaderStages[1] = loadShader(device,
    Reignite::Tools::GetAssetPath() + "shaders/" + pipelineInfo.filenames[1] + ".spv",
    pipelineInfo.stages[1]);

  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo =
    vk::initializers::GraphicsPipelineCreateInfo(pipelineInfo.pipelineLayout, pipelineInfo.renderPass, 0);

  graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
  graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
  graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
  graphicsPipelineCreateInfo.pViewportState = &viewportState;
  graphicsPipelineCreateInfo.pDepthStencilState = &pipelineInfo.depthStencilState;
  graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
  graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
  graphicsPipelineCreateInfo.pStages = shaderStages.data();
  graphicsPipelineCreateInfo.pVertexInputState = &pipelineInfo.vertexInputState;

  return vkCreateGraphicsPipelines(device, pipelineInfo.pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline);
}

VkResult CreateDescriptorPool(VkDevice device, VkDescriptorPool& descriptorPool,
  std::vector<VkDescriptorPoolSize>& poolSizes) {

  VkDescriptorPoolCreateInfo poolInfo = 
    vk::initializers::DescriptorPoolCreateInfo(
      static_cast<u32>(poolSizes.size()), 
      poolSizes.data(), 
      20);

  return vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
}


void GenerateDeferredDebugQuads(vk::VulkanState* vulkanState,
  vk::Buffer* vertices, vk::Buffer* indices, u32& indexCount) {

  std::vector<Vertex> vertexBuffer;

  float x = 0.0f;
  float y = 0.0f;
  for (u32 i = 0; i < 3; ++i) {

    vertexBuffer.push_back({ { x + 1.0f, y + 1.0f, 0.0f }, { 0.0f, 0.0f, (float)i }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } });
    vertexBuffer.push_back({ { x,        y + 1.0f, 0.0f }, { 0.0f, 0.0f, (float)i }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } });
    vertexBuffer.push_back({ { x,        y,        0.0f }, { 0.0f, 0.0f, (float)i }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } });
    vertexBuffer.push_back({ { x + 1.0f, y,        0.0f }, { 0.0f, 0.0f, (float)i }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } });

    x += 1.0f;
    if (x > 1.0f) {
      x = 0.0f;
      y += 1.0f;
    }
  }

  VK_CHECK(vulkanState->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    vertexBuffer.size() * sizeof(Vertex), &vertices->buffer, &vertices->memory,
    vertexBuffer.data()));

  std::vector<u32> indexBuffer = { 0, 1, 2, 2, 3, 0 };
  for (u32 i = 0; i < 3; ++i) {

    u32 indices[6] = { 0, 1, 2, 2, 3, 0 };
    for (auto index : indices) {

      indexBuffer.push_back(i * 4 + index);
    }
  }

  indexCount = static_cast<uint32_t>(indexBuffer.size());

  VK_CHECK(vulkanState->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    indexBuffer.size() * sizeof(u32), &indices->buffer, &indices->memory,
    indexBuffer.data()));
}

void GenerateShadowDebugQuads(vk::VulkanState* vulkanState,
  vk::Buffer* vertices, vk::Buffer* indices, u32& indexCount) {

  std::vector<Vertex> vertexBuffer;

  vertexBuffer.push_back({ { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } });
  vertexBuffer.push_back({ { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } });
  vertexBuffer.push_back({ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } });
  vertexBuffer.push_back({ { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } });

  VK_CHECK(vulkanState->createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    vertexBuffer.size() * sizeof(Vertex), &vertices->buffer, &vertices->memory,
    vertexBuffer.data()));

  std::vector<u32> indexBuffer = { 0, 1, 2, 2, 3, 0 };
  for (u32 i = 0; i < 3; ++i) {

    u32 indices[6] = { 0, 1, 2, 2, 3, 0 };
    for (auto index : indices) {

      indexBuffer.push_back(i * 4 + index);
    }
  }

  indexCount = static_cast<uint32_t>(indexBuffer.size());

  VK_CHECK(vulkanState->createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    indexBuffer.size() * sizeof(u32), &indices->buffer, &indices->memory,
    indexBuffer.data()));
}



void DefineTexturedPBRPipeline() {

  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {

    // Binding 1 : Position texture target / Scene colormap
    vk::initializers::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 0),
    // Binding 2 : Normals texture target
    vk::initializers::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 1),
    // Binding 3 : Albedo texture target
    vk::initializers::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 2),
    // Binding 4 : Roughness texture
    vk::initializers::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 3),
    // Binding 5 : Metalllic texture
    vk::initializers::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 4),
    // Binding 6 : Fragment shader uniform buffer
    vk::initializers::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 5),
    // Binding 7 : Shadow map
    vk::initializers::DescriptorSetLayoutBinding(
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 6),
  };

  VkDescriptorSetLayoutCreateInfo descriptorLayout = vk::initializers::DescriptorSetLayoutCreateInfo(
    setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));

  //VK_CHECK(vkCreateDescriptorSetLayout(data->device, &descriptorLayout, nullptr, &data->materials[4].descriptorSetLayout));

  //pPipelineLayoutCreateInfo = vk::initializers::PipelineLayoutCreateInfo(descSetLayouts2.data(), 3);

  //VK_CHECK(vkCreatePipelineLayout(data->device, &pPipelineLayoutCreateInfo, nullptr, &data->materials[data->matDefDebug].pipelineLayout));

}

// new implementation end <----


VkInstance createInstance() {

  // SHORTCUT: In real Vulkan applications you should probably check if 1.1 is available via vkEnumerateInstanceVersion
  VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
  appInfo.apiVersion = VK_API_VERSION_1_1;

  VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  createInfo.pApplicationInfo = &appInfo;

#ifdef _DEBUG
  const char* debugLayers[] = {

    "VK_LAYER_LUNARG_standard_validation"
  };

  createInfo.ppEnabledLayerNames = debugLayers;
  createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
#endif // _DEBUG

  const char* extensions[] = {

    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
  };

  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

  VkInstance instance = 0;
  VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));

  return instance;
}

VkBool32 debugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
  size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void*pUserData) {

  const char* type =
    (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    ? "ERROR"
    : (flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
    ? "WARNING"
    : "INFO";

  char message[4096];
  snprintf(message, ARRAYSIZE(message), "%s: %s\n", type, pMessage);
  RI_ERROR("{0}\n", message);

#ifdef _WIN32
  OutputDebugString(message);
#endif // _WIN32


  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    assert(!"Validation error encountered!");

  return VK_FALSE;
}

VkDebugReportCallbackEXT registerDebugCallback(VkInstance instance) {

  VkDebugReportCallbackCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
  createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
  createInfo.pfnCallback = debugReportCallback;

  VkDebugReportCallbackEXT callback = 0;
  VK_CHECK(vkCreateDebugReportCallbackEXT(instance, &createInfo, 0, &callback));

  return callback;
}

uint32_t getGraphicsFamilyIndex(VkPhysicalDevice physicalDevice) {

  uint32_t queueCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, 0);

  std::vector<VkQueueFamilyProperties> queues(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queues.data());

  for (uint32_t i = 0; i < queueCount; ++i)
    if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      return i;

  return VK_QUEUE_FAMILY_IGNORED;
}

bool supportsPresentation(VkPhysicalDevice physicalDevice, uint32_t familyIndex) {

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, familyIndex);
#else
  return true;
#endif
}

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, std::string* physicalDeviceNames, 
  uint32_t physicalDeviceCount, VkSampleCountFlagBits& msaaSamples) {

  VkPhysicalDevice discrete = 0;
  VkPhysicalDevice fallback = 0;

  for (uint32_t i = 0; i < physicalDeviceCount; ++i) {

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
    physicalDeviceNames[i] = props.deviceName;

    RI_INFO("GPU{0}: {1}", i, props.deviceName);

    uint32_t familyIndex = getGraphicsFamilyIndex(physicalDevices[i]);
    if (familyIndex == VK_QUEUE_FAMILY_IGNORED)
      continue;

    if (!supportsPresentation(physicalDevices[i], familyIndex))
      continue;

    if (!discrete && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      discrete = physicalDevices[i];
    }

    if (!fallback) {
      fallback = physicalDevices[i];
    }
  }

  VkPhysicalDevice result = discrete ? discrete : fallback;

  if (result) {

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(result, &props);
    //msaaSamples = getMaxUsableSampleCount(result);
    RI_INFO("Selected GPU: {0}\n", props.deviceName);
  }
  else {

    RI_ERROR("ERROR: No GPUs found\n");
  }
  return result;
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {

  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type!");
}

VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool) {

  VkCommandBufferAllocateInfo allocInfo = vk::initializers::CommandBufferAllocateInfo(
    commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

  VkCommandBuffer commandBuffer;
  VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

  VkCommandBufferBeginInfo beginInfo = vk::initializers::CommandBufferBeginInfo();
  VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

  return commandBuffer;
}

void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue graphicsQueue) {

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = vk::initializers::SubmitInfo();
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VkFenceCreateInfo fenceInfo = vk::initializers::FenceCreateInfo(0);
  VkFence fence;
  VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &fence));

  VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence));
  vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000); // Fence timeout
  vkDestroyFence(device, fence, nullptr);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

