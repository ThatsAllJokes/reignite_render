#include "vulkan_tools.h"

#include "vulkan_initializers.h"


VkBool32 vk::tools::GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat) {

  std::vector<VkFormat> depthFormats = {
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM
  };

  for (auto& format : depthFormats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      *depthFormat = format;
      return true;
    }
  }

  return false;
}

void vk::tools::SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, 
  VkImageLayout oldImageLayout, VkImageLayout newImageLayout, 
  VkImageSubresourceRange subresourceRange, 
  VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {

  VkImageMemoryBarrier imageMemoryBarrier = vk::initializers::ImageMemoryBarrier();
  imageMemoryBarrier.oldLayout = oldImageLayout;
  imageMemoryBarrier.newLayout = newImageLayout;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange = subresourceRange;

  switch (oldImageLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED: 
      imageMemoryBarrier.srcAccessMask = 0; 
      break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED: 
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT; 
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: 
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: 
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; 
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: 
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: 
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; 
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: 
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; 
      break;
    default: /* Other source layouts aren't handled (yet) */ 
      break;
  }

  switch (newImageLayout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: 
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; 
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: 
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; 
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: 
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: 
      imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; 
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      if (imageMemoryBarrier.srcAccessMask == 0) {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
      }
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    default: /* Other source layouts aren't handled (yet) */ 
      break;
  }

  vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 
    0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void vk::tools::SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, 
  VkImageAspectFlags aspectMask,
  VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
  VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {

  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = aspectMask;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = 1;
  subresourceRange.layerCount = 1;
  SetImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

VkShaderModule vk::tools::loadShader(VkDevice device, const char* filename) {

  FILE* file = fopen(filename, "rb");
  assert(file);

  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
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