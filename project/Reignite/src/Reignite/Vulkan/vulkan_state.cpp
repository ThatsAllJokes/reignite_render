#include "vulkan_state.h"

#include <algorithm>

#include "vulkan_tools.h"
#include "vulkan_initializers.h"
#include "vulkan_buffer.h"


vk::VulkanState::VulkanState(VkPhysicalDevice physicalDevice) {

  assert(physicalDevice);
  this->physicalDevice = physicalDevice;

  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  vkGetPhysicalDeviceFeatures(physicalDevice, &features);
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

  u32 queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
  assert(queueFamilyCount > 0);
  queueFamilyProperties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

  u32 extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
  if (extensionCount > 0) {

    std::vector<VkExtensionProperties> extensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
      &extensionCount, &extensions.front()) == VK_SUCCESS) {

      for (auto ext : extensions) {

        supportedExtensions.push_back(ext.extensionName);
      }
    }
  }
}

vk::VulkanState::~VulkanState() {

  if (commandPool)
    vkDestroyCommandPool(device, commandPool, nullptr);

  if (device)
    vkDestroyDevice(device, nullptr);

}

u32 vk::VulkanState::getMemoryType(u32 typeBits, VkMemoryPropertyFlags properties) {

  for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
    if ((typeBits & (1 << i)) &&
      (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {

      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type!");
}

u32 vk::VulkanState::getQueueFamilyIndex(VkQueueFlagBits queueFlags) {

  if (queueFlags & VK_QUEUE_COMPUTE_BIT) {

    for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.size()); ++i) {

      if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
        (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {

        return i;
        break;
      }
    }
  }

  if (queueFlags & VK_QUEUE_TRANSFER_BIT) {

    for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.size()); ++i) {

      if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
        ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
        ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {

        return i;
        break;
      }
    }
  }

  for (u32 i = 0; i < static_cast<u32>(queueFamilyProperties.size()); ++i) {

    if (queueFamilyProperties[i].queueFlags & queueFlags) {

      return i;
      break;
    }
  }

  throw std::runtime_error("Could not find a matching queue family index");
}

VkResult vk::VulkanState::createDevice(VkPhysicalDeviceFeatures enabledFeatures,
  std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapchain,
  VkQueueFlags requestedQueueTypes) {

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = {};
  const float defaultQueuePriority(0.0f);

  if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {

    queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &defaultQueuePriority;
    queueCreateInfos.push_back(queueInfo);
  }
  else {
    queueFamilyIndices.graphics = VK_NULL_HANDLE;
  }

  if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {

    queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
    if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {

      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }
  }
  else {
    queueFamilyIndices.compute = queueFamilyIndices.graphics;
  }

  if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {

    queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
    if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute)) {

      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }
  }
  else {
    queueFamilyIndices.transfer = queueFamilyIndices.graphics;
  }

  std::vector<const char*> deviceExtensions(enabledExtensions);
  if (useSwapchain)
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  
  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

  if (pNextChain) {
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.features = enabledFeatures;
    physicalDeviceFeatures2.pNext = pNextChain;
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.pNext = &physicalDeviceFeatures2;
  }

  if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
    deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    //enableDebugMarkers = true;
  }

  if (deviceExtensions.size() > 0) {
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  }

  VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

  if (result == VK_SUCCESS)
    commandPool = createCommandPool(queueFamilyIndices.graphics);

  this->enabledFeatures = enabledFeatures;

  return result;
}

VkResult vk::VulkanState::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
  VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data) {

  VkBufferCreateInfo bufferCreateInfo = vk::initializers::BufferCreateInfo(usageFlags, size);
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VK_CHECK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer));

  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAlloc = vk::initializers::MemoryAllocateInfo();
  vkGetBufferMemoryRequirements(device, *buffer, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
  VK_CHECK(vkAllocateMemory(device, &memAlloc, nullptr, memory));

  if (data != nullptr) {

    void* mapped;
    VK_CHECK(vkMapMemory(device, *memory, 0, size, 0, &mapped));
    memcpy(mapped, data, size);

    if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {

      VkMappedMemoryRange mappedRange = vk::initializers::MappedMemoryRange();
      mappedRange.memory = *memory;
      mappedRange.offset = 0;
      mappedRange.size = size;
      vkFlushMappedMemoryRanges(device, 1, &mappedRange);
    }

    vkUnmapMemory(device, *memory);
  }

  VK_CHECK(vkBindBufferMemory(device, *buffer, *memory, 0));
  return VK_SUCCESS;
}

VkResult vk::VulkanState::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
  VkDeviceSize size, vk::Buffer* buffer, void* data) {

  buffer->device = device;

  VkBufferCreateInfo bufferCreateInfo = vk::initializers::BufferCreateInfo(usageFlags, size);
  VK_CHECK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer->buffer));

  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAlloc = vk::initializers::MemoryAllocateInfo();
  vkGetBufferMemoryRequirements(device, buffer->buffer, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
  VK_CHECK(vkAllocateMemory(device, &memAlloc, nullptr, &buffer->memory));

  buffer->alignment = memReqs.alignment;
  buffer->size = size;
  buffer->usageFlags = usageFlags;
  buffer->memoryPropertyFlags = memoryPropertyFlags;

  if (data != nullptr)
  {
    VK_CHECK(buffer->map());
    memcpy(buffer->mapped, data, size);
    if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
      buffer->flush();

    buffer->unmap();
  }

  buffer->setupDescriptor();
  return buffer->bind();
}

void vk::VulkanState::copyBuffer(vk::Buffer* src, vk::Buffer* dst,
  VkQueue queue, VkBufferCopy* copyRegion) {

  assert(dst->size <= src->size);
  assert(src->buffer);

  VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
  VkBufferCopy bufferCopy{};
  if (copyRegion == nullptr) {
    bufferCopy.size = src->size;
  }
  else {
    bufferCopy = *copyRegion;
  }

  vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);
  flushCommandBuffer(copyCmd, queue);
}

VkCommandPool vk::VulkanState::createCommandPool(
  u32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags) {

  VkCommandPoolCreateInfo commandPoolCrateInfo = {};
  commandPoolCrateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolCrateInfo.queueFamilyIndex = queueFamilyIndex;
  commandPoolCrateInfo.flags = createFlags;
  VkCommandPool cmdPool;
  VK_CHECK(vkCreateCommandPool(device, &commandPoolCrateInfo, nullptr, &cmdPool));
  return cmdPool;
}

VkCommandBuffer vk::VulkanState::createCommandBuffer(
  VkCommandBufferLevel level, bool begin) {

  VkCommandBufferAllocateInfo allocateInfo = 
    vk::initializers::CommandBufferAllocateInfo(commandPool, level, 1);

  VkCommandBuffer commandBuffer;
  VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

  if (begin) {
    VkCommandBufferBeginInfo createInfo = vk::initializers::CommandBufferBeginInfo();
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &createInfo));
  }

  return commandBuffer;
}

void vk::VulkanState::flushCommandBuffer(
  VkCommandBuffer commandBuffer, VkQueue queue, bool free) {

  if (commandBuffer == VK_NULL_HANDLE)
    return;

  VK_CHECK(vkEndCommandBuffer(commandBuffer));

  VkSubmitInfo submitInfo = vk::initializers::SubmitInfo();
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VkFenceCreateInfo fenceCreateInfo = vk::initializers::FenceCreateInfo(0);
  VkFence fence;
  VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

  VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));
  VK_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));

  vkDestroyFence(device, fence, nullptr);

  if (free) {
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }
}

bool vk::VulkanState::extensionSupported(std::string extension) {

  return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

VkFormat vk::VulkanState::getSupportedDepthFormat(bool checkSamplingSupport) {

  std::vector<VkFormat> depthFormats = { 
    VK_FORMAT_D32_SFLOAT_S8_UINT, 
    VK_FORMAT_D32_SFLOAT, 
    VK_FORMAT_D24_UNORM_S8_UINT, 
    VK_FORMAT_D16_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM 
  };
  
  for (auto& format : depthFormats) {

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
    if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      
      if (checkSamplingSupport) {
        
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
          continue;
        }
      }

      return format;
    }
  }

  throw std::runtime_error("Could not find a matching depth format");
}