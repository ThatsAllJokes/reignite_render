#ifndef _RI_VULKAN_STATE_
#define _RI_VULKAN_STATE_ 1

#include <vector>

#include <volk.h>

#include "../basic_types.h"


namespace vk {

  class Buffer;

  class VulkanState {
   public:

    VulkanState(VkPhysicalDevice physicalDevice);
    ~VulkanState();

    u32 getMemoryType(u32 typeBits, VkMemoryPropertyFlags properties);

    u32 getQueueFamilyIndex(VkQueueFlagBits queueFlags);

    VkResult createDevice(
      VkPhysicalDeviceFeatures enabledFeatures, 
      std::vector<const char*> enabledExtensions, 
      void* pNextChain, bool useSwapChain = true, 
      VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

    VkResult createBuffer(
      VkBufferUsageFlags usageFlags,
      VkMemoryPropertyFlags memoryPropertyFlags,
      VkDeviceSize size,
      VkBuffer* buffer,
      VkDeviceMemory* memory,
      void* data = nullptr);

    VkResult createBuffer(
      VkBufferUsageFlags usageFlags,
      VkMemoryPropertyFlags memoryPropertyFlags,
      VkDeviceSize size,
      vk::Buffer* buffer,
      void* data = nullptr);

    void copyBuffer(
      vk::Buffer* src,
      vk::Buffer* dst, 
      VkQueue queue, 
      VkBufferCopy* copyRegion = nullptr);

    VkCommandPool createCommandPool(u32 queueFamilyIndex, 
      VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);

    void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

    bool extensionSupported(std::string extension);

    VkFormat getSupportedDepthFormat(bool checkSamplingSupport);


    VkDevice device;
    VkPhysicalDevice physicalDevice;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceFeatures enabledFeatures;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<std::string> supportedExtensions;

    VkCommandPool commandPool = VK_NULL_HANDLE;

    struct {
      u32 graphics;
      u32 compute;
      u32 transfer;
    } queueFamilyIndices;
  };
}


#endif // _RI_VULKAN_STATE_
