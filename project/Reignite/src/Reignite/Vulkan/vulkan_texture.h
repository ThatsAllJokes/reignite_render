#ifndef _RI_VULKAN_TEXTURE_
#define _RI_VULKAN_TEXTURE_ 1

#include <string>

#include "../basic_types.h"

#include <volk.h>


namespace vk {

  class Texture {
   public:

    void updateDescriptor();
    void destroy();

    VkDevice device; // temporal
    VkPhysicalDevice physicalDevice; // temporal

    VkImage image;
    VkImageView view;
    VkImageLayout imageLayout;
    VkDeviceMemory deviceMemory;
    u32 width;
    u32 height;
    u32 mipLevels;
    u32 layerCount;
    VkDescriptorImageInfo descriptor;
    VkSampler sampler;
  };


  class Texture2D : public Texture {
   public:

    void loadFromFile(std::string file, VkFormat format,
      VkDevice vkDevice, VkPhysicalDevice vkPhysDevice, VkCommandPool vkCmdPool, VkQueue copyQueue, 
      VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, 
      VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    void loadFromFileSTB(std::string file, VkFormat format,
      VkDevice vkDevice, VkPhysicalDevice vkPhysDevice, VkCommandPool vkCmdPool, VkQueue copyQueue,
      VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
      VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  };

  class TextureCubeMap : public Texture {
   public:

    void loadFromFile(std::string file);
  };

} // end of vk namespace


#endif // _RI_VULKAN_TEXTURE_
