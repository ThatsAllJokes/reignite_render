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

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
  
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

  VkPhysicalDeviceFeatures physicalDeviceFeatures;
  vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

  VkSampleCountFlags counts =
    physicalDeviceProperties.limits.framebufferColorSampleCounts &
    physicalDeviceProperties.limits.framebufferDepthSampleCounts;

  return VK_SAMPLE_COUNT_1_BIT;

  if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
  if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
  if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
  if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
  if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
  if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

  return VK_SAMPLE_COUNT_1_BIT;
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
    msaaSamples = getMaxUsableSampleCount(result);
    RI_INFO("Selected GPU: {0}\n", props.deviceName);
  }
  else {

    RI_ERROR("ERROR: No GPUs found\n");
  }
  return result;
}

VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t familyIndex) {

  float queuePriorities[] = { 1.0f };

  VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
  queueInfo.queueFamilyIndex = familyIndex;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = queuePriorities;

  const char* extensions[] = {

    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, // Allows to send data/objects to the shader
  };

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE; // TODO: Check feature availability (unlikely not supporting)

  VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  createInfo.queueCreateInfoCount = 1;
  createInfo.pQueueCreateInfos = &queueInfo;
  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

  VkDevice device = 0;
  VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, 0, &device));

  return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
  createInfo.hwnd = glfwGetWin32Window(window);
  createInfo.hinstance = GetModuleHandle(0);

  VkSurfaceKHR surface = 0;
  VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, 0, &surface));
  return surface;
#else
#error Unsupported platform
#endif // VK_USE_PLATFORM_WIN32_KHR
}

VkFormat getSwapchainFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

  uint32_t formatCount = 0;
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, 0));

  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()));

  if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
    return VK_FORMAT_R8G8B8A8_UNORM;

  for (uint32_t i = 0; i < formatCount; ++i)
    if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM || formats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
      return formats[i].format;

  return formats[0].format;
}

VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR surfaceCaps, uint32_t familyIndex, VkFormat format, uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain) {

  VkCompositeAlphaFlagBitsKHR surfaceComposite =
    (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
    ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
    : (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
    ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
    : (surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
    ? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
    : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

  VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  createInfo.surface = surface;
  createInfo.minImageCount = std::max(2u, surfaceCaps.minImageCount);
  createInfo.imageFormat = format;
  createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  createInfo.imageExtent.width = width;
  createInfo.imageExtent.height = height;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.queueFamilyIndexCount = 1;
  createInfo.pQueueFamilyIndices = &familyIndex;
  createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  createInfo.compositeAlpha = surfaceComposite;
  createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // TODO: Research presentation mode
  createInfo.oldSwapchain = oldSwapchain;

  VkSwapchainKHR swapchain = 0;
  VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain));

  return swapchain;
}

VkSemaphore createSemaphore(VkDevice device) {

  VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

  VkSemaphore semaphore = 0;
  VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &semaphore));

  return semaphore;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t familyIndex) {

  VkCommandPoolCreateInfo createInfo = vk::initializers::CommandPoolCreateInfo();
  createInfo.queueFamilyIndex = familyIndex;
  createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkCommandPool commandPool = 0;
  VK_CHECK(vkCreateCommandPool(device, &createInfo, 0, &commandPool));

  return commandPool;
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

VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
  VkImageTiling tiling, VkFormatFeatureFlags features) {

  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    }
    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  assert(!"Failed to find supported format");
}

VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice) {

  return FindSupportedFormat(physicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkRenderPass createRenderPass(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat format, VkSampleCountFlagBits msaaSamples) {

  std::array<VkAttachmentDescription, 2> attachments = {};
  attachments[0].format = format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  attachments[1].format = FindDepthFormat(physicalDevice);
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorReference = {};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthReference = {};
  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;
  subpassDescription.pDepthStencilAttachment = &depthReference;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;
  subpassDescription.pResolveAttachments = nullptr;

  std::array<VkSubpassDependency, 2> dependencies;
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpassDescription;
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  renderPassInfo.pDependencies = dependencies.data();

  VkRenderPass renderPass = 0;
  VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, 0, &renderPass));

  return renderPass;
}

VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass renderPass,
  VkImageView imageView, VkImageView depthImageView, VkImageView colorImageView,
  uint32_t width, uint32_t height) {

  std::array<VkImageView, 2> attachments = { imageView, depthImageView };

  VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
  createInfo.renderPass = renderPass;
  createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  createInfo.pAttachments = attachments.data();
  createInfo.width = width;
  createInfo.height = height;
  createInfo.layers = 1;

  VkFramebuffer framebuffer = 0;
  VK_CHECK(vkCreateFramebuffer(device, &createInfo, 0, &framebuffer));

  return framebuffer;
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
  VkImageAspectFlags aspectFlags, uint32_t mipLevels) {

  VkImageViewCreateInfo createInfo = vk::initializers::ImageViewCreateInfo();
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format = format;
  createInfo.subresourceRange = {};
  createInfo.subresourceRange.aspectMask = aspectFlags;
  createInfo.subresourceRange.baseMipLevel = 0;
  createInfo.subresourceRange.levelCount = mipLevels;
  createInfo.subresourceRange.baseArrayLayer = 0;
  createInfo.subresourceRange.layerCount = 1;
  createInfo.image = image;

  VkImageView imageView;
  VK_CHECK(vkCreateImageView(device, &createInfo, 0, &imageView));

  return imageView;
}


VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout) {

  VkPushConstantRange pushConstantRange = 
    vk::initializers::PushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Reignite::MaterialResource::PushBlock), 0);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
    vk::initializers::PipelineLayoutCreateInfo(&descriptorSetLayout);
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  VkPipelineLayout layout = 0;
  VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, 0, &layout));

  return layout;
}

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device) {

  std::vector<VkDescriptorSetLayoutBinding> bindings = { 
    
    vk::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),

    vk::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 1),

    vk::initializers::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_SHADER_STAGE_FRAGMENT_BIT, 2),
  };

  VkDescriptorSetLayoutCreateInfo layoutInfo = vk::initializers::DescriptorSetLayoutCreateInfo(bindings);

  VkDescriptorSetLayout descriptorSetLayout;
  VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));

  return descriptorSetLayout;
}

VkPipeline createGraphicsPipeline(VkDevice device, VkPipelineCache pipelineCache, VkRenderPass renderPass,
  VkShaderModule vs, VkShaderModule fs, VkPipelineLayout layout, VkSampleCountFlagBits msaaSamples) {

  VkPipelineShaderStageCreateInfo stages[2] = {};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vs;
  stages[0].pName = "main";
  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fs;
  stages[1].pName = "main";

  VkGraphicsPipelineCreateInfo createInfo = vk::initializers::GraphicsPipelineCreateInfo();
  createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
  createInfo.pStages = stages;

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescription = Vertex::getAttributeDescription();

  VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
  vertexInput.vertexBindingDescriptionCount = 1;
  vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
  vertexInput.pVertexBindingDescriptions = bindingDescription.data();
  vertexInput.pVertexAttributeDescriptions = attributeDescription.data();
  createInfo.pVertexInputState = &vertexInput;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  createInfo.pInputAssemblyState = &inputAssembly;

  VkPipelineViewportStateCreateInfo viewportState =
    vk::initializers::PipelineViewportStateCreateInfo(1, 1);

  createInfo.pViewportState = &viewportState;

  VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
  rasterizationState.lineWidth = 1;
  rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  createInfo.pRasterizationState = &rasterizationState;

  VkPipelineMultisampleStateCreateInfo multisampleState = 
    vk::initializers::PipelineMultisampleStateCreateInfo(msaaSamples);
  createInfo.pMultisampleState = &multisampleState;

  VkPipelineDepthStencilStateCreateInfo depthstencilState =
    vk::initializers::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
  depthstencilState.depthBoundsTestEnable = VK_FALSE;
  depthstencilState.minDepthBounds = 0.0f;
  depthstencilState.maxDepthBounds = 1.0f;
  depthstencilState.stencilTestEnable = VK_FALSE;
  depthstencilState.front = {};
  createInfo.pDepthStencilState = &depthstencilState;

  VkPipelineColorBlendAttachmentState colorAttachmentState = 
    vk::initializers::PipelineColorBlendAttachmentState(
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);

  VkPipelineColorBlendStateCreateInfo colorblendState =
    vk::initializers::PipelineColorBlendStateCreateInfo(1 , &colorAttachmentState);
  createInfo.pColorBlendState = &colorblendState;

  VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState = 
    vk::initializers::PipelineDynamicStateCreateInfo(dynamicStates, 2);
  createInfo.pDynamicState = &dynamicState;

  createInfo.layout = layout;
  createInfo.renderPass = renderPass;

  VkPipeline pipeline = 0;
  VK_CHECK(vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, 0, &pipeline));

  return pipeline;
}

VkImageMemoryBarrier imageBarrier(VkImage image,
  VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
  VkImageLayout oldLayout, VkImageLayout newLayout) {

  VkImageMemoryBarrier result = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
  result.srcAccessMask = srcAccessMask;
  result.dstAccessMask = dstAccessMask;
  result.oldLayout = oldLayout;
  result.newLayout = newLayout;
  result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  result.image = image;
  result.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  result.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
  result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

  return result;
}

void createSwapchain(Swapchain& result, VkPhysicalDevice physicalDevice, VkDevice device,
  VkSurfaceKHR surface, uint32_t familyIndex, VkFormat format, VkRenderPass renderPass,
  VkImageView depthImageView, VkImageView colorImageView, VkSwapchainKHR oldSwapchain) {

  VkSurfaceCapabilitiesKHR surfaceCaps;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

  uint32_t width = surfaceCaps.currentExtent.width;
  uint32_t height = surfaceCaps.currentExtent.height;

  VkSwapchainKHR swapchain = createSwapchain(device, surface, surfaceCaps, familyIndex, format, width, height, oldSwapchain);
  assert(swapchain);

  uint32_t imageCount = 0;
  VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, 0));

  std::vector<VkImage> images(imageCount);
  VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data()));

  std::vector<VkImageView> imageViews(imageCount);
  for (uint32_t i = 0; i < imageCount; ++i) {
    imageViews[i] = createImageView(device, images[i], format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    assert(imageViews[i]);
  }

  std::vector<VkFramebuffer> framebuffers(imageCount);
  for (uint32_t i = 0; i < imageCount; ++i) {
    framebuffers[i] = createFramebuffer(device, renderPass, imageViews[i], depthImageView, colorImageView, width, height);
    assert(framebuffers[i]);
  }

  result.swapchain = swapchain;
  result.images = images;
  result.imageViews = imageViews;
  result.framebuffers = framebuffers;

  result.width = width;
  result.height = height;
  result.imageCount = imageCount;
}

void destroySwapchain(VkDevice device, const Swapchain& swapchain) {

  for (uint32_t i = 0; i < swapchain.imageCount; ++i)
    vkDestroyFramebuffer(device, swapchain.framebuffers[i], 0);

  for (uint32_t i = 0; i < swapchain.imageCount; ++i)
    vkDestroyImageView(device, swapchain.imageViews[i], 0);

  vkDestroySwapchainKHR(device, swapchain.swapchain, 0);
}

// TODO: Check uniform buffers recreation when modifying swapchain
// TODO: Check description pool recreation when modifying swapchain
void resizeSwapchainIfNecessary(Swapchain& result, VkPhysicalDevice physicalDevice,
  VkDevice device, VkSurfaceKHR surface, uint32_t familyIndex, VkFormat format,
  VkRenderPass renderPass, VkImageView depthImageView) {

  VkSurfaceCapabilitiesKHR surfaceCaps;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

  uint32_t newWidth = surfaceCaps.currentExtent.width;
  uint32_t newHeight = surfaceCaps.currentExtent.height;

  if (result.width == newWidth && result.height == newHeight)
    return;

  // TODO: Check depth buffer resizing

  Swapchain old = result;
  //createSwapchain(result, physicalDevice, device, surface, familyIndex, format, renderPass, depthImageView, old.swapchain);
  VK_CHECK(vkDeviceWaitIdle(device));
  destroySwapchain(device, old);
}

void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, Buffer* buffer,
  VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags properties) {

  buffer->device = device;

  VkBufferCreateInfo bufferInfo = vk::initializers::BufferCreateInfo(usageFlags, size);
  VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer->buffer));

  VkMemoryRequirements memRequirements;
  VkMemoryAllocateInfo allocInfo = vk::initializers::MemoryAllocateInfo();
  vkGetBufferMemoryRequirements(device, buffer->buffer, &memRequirements);
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
  VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &buffer->bufferMemory));

  buffer->alignment = memRequirements.alignment;
  buffer->size = size;
  buffer->usageFlags = usageFlags;
  buffer->memoryPropertyFlags = properties;

  buffer->descriptor.offset = 0;
  buffer->descriptor.buffer = buffer->buffer;
  buffer->descriptor.range = VK_WHOLE_SIZE;

  VK_CHECK(vkBindBufferMemory(device, buffer->buffer, buffer->bufferMemory, 0));
}

void destroyBuffer(VkDevice device, const Buffer& buffer) {

  vkFreeMemory(device, buffer.bufferMemory, 0);
  vkDestroyBuffer(device, buffer.buffer, 0);
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

void copyBuffer(VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
  VkCommandPool commandPool, VkQueue graphicsQueue) {

  VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}

Buffer createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
  const std::vector<Vertex>& vertices, VkCommandPool commandPool, VkQueue graphicsQueue) {

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  Buffer stagingBuffer;
  createBuffer(device, physicalDevice, &stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  Buffer buffer;
  createBuffer(device, physicalDevice, &buffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  void* data;
  VK_CHECK(vkMapMemory(device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data));
  memcpy(data, vertices.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBuffer.bufferMemory);

  copyBuffer(device, stagingBuffer.buffer, buffer.buffer,
    bufferSize, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
  vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

  return buffer;
}

Buffer createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
  const std::vector<u32>& indices, VkCommandPool commandPool, VkQueue graphicsQueue) {

  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  Buffer stagingBuffer;
  createBuffer(device, physicalDevice, &stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  void* data;
  VK_CHECK(vkMapMemory(device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data));
  memcpy(data, indices.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBuffer.bufferMemory);

  Buffer buffer;
  createBuffer(device, physicalDevice, &buffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  copyBuffer(device, stagingBuffer.buffer, buffer.buffer,
    bufferSize, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
  vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

  return buffer;
}

Buffer createUniformBuffer(VkDevice device, VkPhysicalDevice physicalDevice) {

  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  Buffer uniformBufferObject;
  createBuffer(device, physicalDevice, &uniformBufferObject, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  return uniformBufferObject;
}

void MapUniformBuffer(VkDevice device, Buffer& buffer, void* data, u32 uboSize) {

  void* mapped;
  vkMapMemory(device, buffer.bufferMemory, 0, uboSize, 0, &mapped);
  memcpy(mapped, data, uboSize);
  vkUnmapMemory(device, buffer.bufferMemory);
}

Buffer createUniformBufferParams(VkDevice device, VkPhysicalDevice physicalDevice) {

  VkDeviceSize bufferSize = sizeof(UBOParams);

  Buffer uniformBufferObject;
  createBuffer(device, physicalDevice, &uniformBufferObject, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  return uniformBufferObject;
}

VkDescriptorPool createDescriptorPool(VkDevice device, uint32_t maxDescriptors, uint32_t maxSamplers) {

  std::vector<VkDescriptorPoolSize> poolSizes = {
    vk::initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxDescriptors),
    vk::initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSamplers)
  };

  VkDescriptorPoolCreateInfo poolInfo = 
    vk::initializers::DescriptorPoolCreateInfo(static_cast<u32>(poolSizes.size()), 
      poolSizes.data(), 3);

  VkDescriptorPool descriptorPool;
  VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));

  return descriptorPool;
}

VkDescriptorSet createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
  VkDescriptorSetLayout descriptorSetLayout, Buffer& uniformBuffer, Buffer& params,
  VkImageView textureImageView, VkSampler textureSampler) {

  VkDescriptorSetAllocateInfo allocInfo = 
    vk::initializers::DescriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

  VkDescriptorSet descriptorSet;
  VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer = uniformBuffer.buffer;
  bufferInfo.offset = 0;
  bufferInfo.range = sizeof(UniformBufferObject);

  VkDescriptorBufferInfo paramsInfo = {};
  paramsInfo.buffer = params.buffer;
  paramsInfo.offset = 0;
  paramsInfo.range = sizeof(UBOParams);

  VkDescriptorImageInfo imageInfo = vk::initializers::DescriptorImageInfo(
    textureSampler, textureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  std::array<VkWriteDescriptorSet, 3> descriptorWrites = {
  
    vk::initializers::WriteDescriptorSet(descriptorSet, 
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo),

    vk::initializers::WriteDescriptorSet(descriptorSet,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &paramsInfo),

    vk::initializers::WriteDescriptorSet(descriptorSet,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &imageInfo)
  };

  vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
    descriptorWrites.data(), 0, nullptr);

  return descriptorSet;
}

void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
  uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
  VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {

  VkImageCreateInfo imageInfo = vk::initializers::ImageCreateInfo();
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage; // VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
  imageInfo.samples = numSamples;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VK_CHECK(vkCreateImage(device, &imageInfo, nullptr, &image)); // TODO: Check for different supported formats

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = vk::initializers::MemoryAllocateInfo();
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory));

  vkBindImageMemory(device, image, imageMemory, 0);
}

void generateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
  VkQueue graphicsQueue, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight,
  uint32_t mipLevels) {

  // Check if image format supports linear blitting
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

  if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    assert(!"Texture image format does not support linear blitting");
  }

  VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = image;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int32_t mipWidth = texWidth;
  int32_t mipHeight = texHeight;

  for (uint32_t i = 1; i < mipLevels; i++) {

    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkImageBlit blit = {};
    blit.srcOffsets[0] = { 0, 0, 0 };
    blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = { 0, 0, 0 };
    blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    if (mipWidth > 1) mipWidth /= 2;
    if (mipHeight > 1) mipHeight /= 2;
  }

  barrier.subresourceRange.baseMipLevel = mipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    0, 0, nullptr, 0, nullptr, 1, &barrier);

  EndSingleTimeCommands(device, commandBuffer, commandPool, graphicsQueue);
}

// Deferred Tool functions ///////////////////////////////////////////////////////////////////////////////

/*void CreateFramebufferAttachment(VkDevice device, VkPhysicalDevice physicalDevice, 
  VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment* attachment,
  s32 offScreenWidth, s32 offScreenHeight) {

  VkImageAspectFlags aspectMask = 0;
  VkImageLayout imageLayout;

  attachment->format = format;

  if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
    aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  }

  if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }

  assert(aspectMask > 0);

  VkImageCreateInfo image = vk::initializers::ImageCreateInfo();
  image.imageType = VK_IMAGE_TYPE_2D;
  image.format = format;
  image.extent.width = offScreenWidth;
  image.extent.height = offScreenHeight;
  image.extent.depth = 1;
  image.mipLevels = 1;
  image.arrayLayers = 1;
  image.samples = VK_SAMPLE_COUNT_1_BIT;
  image.tiling = VK_IMAGE_TILING_OPTIMAL;
  image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

  VkMemoryAllocateInfo memAlloc = vk::initializers::MemoryAllocateInfo();
  VkMemoryRequirements memReqs;

  VK_CHECK(vkCreateImage(device, &image, nullptr, &attachment->image));
  vkGetImageMemoryRequirements(device, attachment->image, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex = findMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK(vkAllocateMemory(device, &memAlloc, nullptr, &attachment->mem));
  VK_CHECK(vkBindImageMemory(device, attachment->image, attachment->mem, 0));

  VkImageViewCreateInfo imageView = vk::initializers::ImageViewCreateInfo();
  imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageView.format = format;
  imageView.subresourceRange = {};
  imageView.subresourceRange.aspectMask = aspectMask;
  imageView.subresourceRange.baseMipLevel = 0;
  imageView.subresourceRange.levelCount = 1;
  imageView.subresourceRange.baseArrayLayer = 0;
  imageView.subresourceRange.layerCount = 1;
  imageView.image = attachment->image;
  VK_CHECK(vkCreateImageView(device, &imageView, nullptr, &attachment->view));
}*/


