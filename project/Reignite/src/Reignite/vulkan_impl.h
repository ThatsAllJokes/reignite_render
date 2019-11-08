#ifndef _VULKAN_IMPL_
#define _VULKAN_IMPL_ 1

#include <assert.h>
#include <stdio.h>

#include <array>
#include <vector>
#include <algorithm>

#include <volk.h>

#define VK_CHECK(call) \
  do { \
    VkResult result_ = call; \
    assert(result_ == VK_SUCCESS); \
  }while(0)

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif // ARRAYSIZE

VkInstance createInstance() {

  // SHORTCUT: In real Vulkan applications you should probably check if 1.1 is available via vkEnumerateInstanceVersion
  VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
  appInfo.apiVersion = VK_API_VERSION_1_1;

  VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
  createInfo.pApplicationInfo = &appInfo;

#ifdef RI_DEBUG
  const char* debugLayers[] = {

    "VK_LAYER_LUNARG_standard_validation"
  };

  createInfo.ppEnabledLayerNames = debugLayers;
  createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
#endif // RI_DEBUG

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

  printf("%s", message);

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

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, uint32_t physicalDeviceCount) {

  VkPhysicalDevice discrete = 0;
  VkPhysicalDevice fallback = 0;

  for (uint32_t i = 0; i < physicalDeviceCount; ++i) {

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

    printf("GPU%d: %s\n", i, props.deviceName);

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
    printf("Selected GPU: %s\n", props.deviceName);
  }
  else {

    printf("ERROR: No GPUs found\n");
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
  };

  VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
  createInfo.queueCreateInfoCount = 1;
  createInfo.pQueueCreateInfos = &queueInfo;

  createInfo.ppEnabledExtensionNames = extensions;
  createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

  VkDevice device = 0;
  VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, 0, &device));

  return device;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
  createInfo.hinstance = GetModuleHandle(0);
  createInfo.hwnd = glfwGetWin32Window(window);

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

  VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
  createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  createInfo.queueFamilyIndex = familyIndex;

  VkCommandPool commandPool = 0;
  VK_CHECK(vkCreateCommandPool(device, &createInfo, 0, &commandPool));

  return commandPool;
}

VkRenderPass createRenderPass(VkDevice device, VkFormat format) {

  VkAttachmentDescription attachments[1] = {};
  attachments[0].format = format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachments = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachments;

  VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
  createInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
  createInfo.pAttachments = attachments;
  createInfo.subpassCount = 1;
  createInfo.pSubpasses = &subpass;

  VkRenderPass renderPass = 0;
  VK_CHECK(vkCreateRenderPass(device, &createInfo, 0, &renderPass));

  return renderPass;
}

VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass renderPass, VkImageView imageView, uint32_t width, uint32_t height) {

  VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };

  createInfo.renderPass = renderPass;
  createInfo.attachmentCount = 1;
  createInfo.pAttachments = &imageView;
  createInfo.width = width;
  createInfo.height = height;
  createInfo.layers = 1;

  VkFramebuffer framebuffer = 0;
  VK_CHECK(vkCreateFramebuffer(device, &createInfo, 0, &framebuffer));

  return framebuffer;
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format) {

  VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  createInfo.image = image;
  createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  createInfo.format = format;
  createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  createInfo.subresourceRange.levelCount = 1;
  createInfo.subresourceRange.layerCount = 1;

  VkImageView view = 0;
  VK_CHECK(vkCreateImageView(device, &createInfo, 0, &view));

  return view;
}

VkShaderModule loadShader(VkDevice device, const char* path) {

  FILE* file = fopen(path, "rb");
  assert(file);

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  assert(length >= 0);
  fseek(file, 0, SEEK_SET);

  char* buffer = new char[length];
  assert(buffer);

  size_t rc = fread(buffer, 1, length, file);
  assert(rc == size_t(length));
  fclose(file);

  VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  createInfo.codeSize = length;
  createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer);

  VkShaderModule shaderModule = 0;
  VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));

  return shaderModule;
}

VkPipelineLayout createPipelineLayout(VkDevice device) {

  VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

  VkPipelineLayout layout = 0;
  VK_CHECK(vkCreatePipelineLayout(device, &createInfo, 0, &layout));

  return layout;
}

VkPipeline createGraphicsPipeline(VkDevice device, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkShaderModule vs, VkShaderModule fs, VkPipelineLayout layout) {

  VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

  VkPipelineShaderStageCreateInfo stages[2] = {};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vs;
  stages[0].pName = "main";
  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fs;
  stages[1].pName = "main";

  createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
  createInfo.pStages = stages;

  VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
  createInfo.pVertexInputState = &vertexInput;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  createInfo.pInputAssemblyState = &inputAssembly;

  VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  createInfo.pViewportState = &viewportState;

  VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
  rasterizationState.lineWidth = 1;
  createInfo.pRasterizationState = &rasterizationState;

  VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  createInfo.pMultisampleState = &multisampleState;

  VkPipelineDepthStencilStateCreateInfo depthstencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
  createInfo.pDepthStencilState = &depthstencilState;

  VkPipelineColorBlendAttachmentState colorAttachmentState = { };
  colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorblendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
  colorblendState.attachmentCount = 1;
  colorblendState.pAttachments = &colorAttachmentState;
  createInfo.pColorBlendState = &colorblendState;

  VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

  VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
  dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
  dynamicState.pDynamicStates = dynamicStates;
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

struct Swapchain {

  VkSwapchainKHR swapchain;

  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;
  std::vector<VkFramebuffer> framebuffers;

  uint32_t width, height;
  uint32_t imageCount;
};

void createSwapchain(Swapchain& result, VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface,
  uint32_t familyIndex, VkFormat format, VkRenderPass renderPass, VkSwapchainKHR oldSwapchain = 0) {

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
    imageViews[i] = createImageView(device, images[i], format);
    assert(imageViews[i]);
  }

  std::vector<VkFramebuffer> framebuffers(imageCount);
  for (uint32_t i = 0; i < imageCount; ++i) {
    framebuffers[i] = createFramebuffer(device, renderPass, imageViews[i], width, height);
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

void resizeSwapchainIfNecessary(Swapchain& result, VkPhysicalDevice physicalDevice,
  VkDevice device, VkSurfaceKHR surface, uint32_t familyIndex, VkFormat format, VkRenderPass renderPass) {

  VkSurfaceCapabilitiesKHR surfaceCaps;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

  uint32_t newWidth = surfaceCaps.currentExtent.width;
  uint32_t newHeight = surfaceCaps.currentExtent.height;

  if (result.width == newWidth && result.height == newHeight)
    return;

  Swapchain old = result;
  createSwapchain(result, physicalDevice, device, surface, familyIndex, format, renderPass, old.swapchain);
  VK_CHECK(vkDeviceWaitIdle(device));
  destroySwapchain(device, old);
}


#endif // _VULKAN_IMPL_

