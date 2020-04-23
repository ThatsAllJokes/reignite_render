#include "vulkan_texture.h"

#include "ktx.h"
#include "ktxvulkan.h"

#include "../tools.h"


void vk::Texture::updateDescriptor() {

  descriptor.sampler = sampler;
  descriptor.imageView = view;
  descriptor.imageLayout = imageLayout;
}

void vk::Texture::destroy() {

  vkDestroyImageView(device, view, nullptr);
  vkDestroyImage(device, image, nullptr);

  if (sampler)
    vkDestroySampler(device, sampler, nullptr);

  vkFreeMemory(device, deviceMemory, nullptr);
}

ktxResult loadKTXFile(std::string filename, ktxTexture** target) {

  ktxResult result = KTX_SUCCESS;
  result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);

  return result;
}

void vk::Texture2D::loadFromFile(std::string filename, VkFormat format,
  VkDevice vkDevice, VkPhysicalDevice vkPhysDevice, VkCommandPool vkCmdPool,
  VkQueue copyQueue, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout) {

  ktxTexture* ktxTexture = nullptr;
  ktxResult result = loadKTXFile(filename, &ktxTexture);
  assert(result == KTX_SUCCESS);

  device = vkDevice;
  width = ktxTexture->baseWidth;
  height = ktxTexture->baseHeight;
  mipLevels = ktxTexture->numLevels;

  ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
  ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

  VkFormatProperties formatProperties = {};
  vkGetPhysicalDeviceFormatProperties(vkPhysDevice, format, &formatProperties);

  VkMemoryAllocateInfo memAllocInfo = vk::initializers::MemoryAllocateInfo();
  VkMemoryRequirements memReqs = {};

  VkCommandBuffer copyCmd = BeginSingleTimeCommands(vkDevice, vkCmdPool);

  VkBuffer stagingBuffer = 0;
  VkDeviceMemory stagingMemory = 0;

  VkBufferCreateInfo bufferCreateInfo = vk::initializers::BufferCreateInfo();
  bufferCreateInfo.size = ktxTextureSize;
  bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VK_CHECK(vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

  vkGetBufferMemoryRequirements(vkDevice, stagingBuffer, &memReqs);
  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = findMemoryType(vkPhysDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  VK_CHECK(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &stagingMemory));
  VK_CHECK(vkBindBufferMemory(vkDevice, stagingBuffer, stagingMemory, 0));

  u8* data = nullptr;
  VK_CHECK(vkMapMemory(vkDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
  memcpy(data, ktxTextureData, ktxTextureSize);
  vkUnmapMemory(vkDevice, stagingMemory);

  std::vector<VkBufferImageCopy> bufferCopyRegions = {}; // copy regions for each mip level

  for (u32 i = 0; i < mipLevels; ++i) {

    ktx_size_t offset;
    KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i , 0, 0, &offset);
    assert(result == KTX_SUCCESS);

    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = i;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> i;
    bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> i;
    bufferCopyRegion.imageExtent.depth = 1;
    bufferCopyRegion.bufferOffset = offset;

    bufferCopyRegions.push_back(bufferCopyRegion);
  }

  VkImageCreateInfo imageCreateInfo = vk::initializers::ImageCreateInfo();
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = format;
  imageCreateInfo.mipLevels = mipLevels;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.extent = { width, height, 1 };
  imageCreateInfo.usage = imageUsageFlags;

  if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {

    imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  VK_CHECK(vkCreateImage(vkDevice, &imageCreateInfo, nullptr, &image));

  vkGetImageMemoryRequirements(vkDevice, image, &memReqs);
  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = findMemoryType(vkPhysDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &deviceMemory));
  VK_CHECK(vkBindImageMemory(vkDevice, image, deviceMemory, 0));

  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = mipLevels;
  subresourceRange.layerCount = 1;

  vk::tools::SetImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT); // TODO: Check exception on SetImageLayout

  vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

  this->imageLayout = imageLayout;

  vk::tools::SetImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    imageLayout, subresourceRange,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

  EndSingleTimeCommands(vkDevice, copyCmd, vkCmdPool, copyQueue);

  vkFreeMemory(vkDevice, stagingMemory, nullptr);
  vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);

  ktxTexture_Destroy(ktxTexture);

  VkSamplerCreateInfo samplerCreateInfo = {};
  samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.mipLodBias = 0.0f;
  samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerCreateInfo.minLod = 0.0f;
  // Max level-of-detail should match mip level count
  samplerCreateInfo.maxLod = mipLevels; // TODO: (useStaging) ? (float)mipLevels : 0.0f;
  // Only enable anisotropic filtering if enabled on the devicec
  samplerCreateInfo.maxAnisotropy = 16; // TODO: device->enabledFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
  samplerCreateInfo.anisotropyEnable = 1; // TODO: device->enabledFeatures.samplerAnisotropy;
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK(vkCreateSampler(vkDevice, &samplerCreateInfo, nullptr, &sampler));

  VkImageViewCreateInfo viewCreateInfo = {};
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format = format;
  viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
  viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
  // Linear tiling usually won't support mip maps
  // Only set mip map count if optimal tiling is used
  viewCreateInfo.subresourceRange.levelCount = mipLevels; // TODO: (useStaging) ? mipLevels : 1;
  viewCreateInfo.image = image;
  VK_CHECK(vkCreateImageView(vkDevice, &viewCreateInfo, nullptr, &view));

  updateDescriptor();
}

void vk::Texture2D::loadFromFileSTB(std::string filename, VkFormat format,
  VkDevice vkDevice, VkPhysicalDevice vkPhysDevice, VkCommandPool vkCmdPool,
  VkQueue copyQueue, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout) {

  void* texData;
  s32 texWidth, texHeight;
  bool result = Reignite::Tools::LoadTextureFile(filename, texWidth, texHeight, &texData);
  assert(result);

  device = vkDevice;
  width = (u32)texWidth;
  height = (u32)texHeight;
  mipLevels = 1;

  u32 texSize = width * height * 4;

  VkFormatProperties formatProperties = {};
  vkGetPhysicalDeviceFormatProperties(vkPhysDevice, format, &formatProperties);

  VkMemoryAllocateInfo memAllocInfo = vk::initializers::MemoryAllocateInfo();
  VkMemoryRequirements memReqs = {};

  VkCommandBuffer copyCmd = BeginSingleTimeCommands(vkDevice, vkCmdPool);

  VkBuffer stagingBuffer = 0;
  VkDeviceMemory stagingMemory = 0;

  VkBufferCreateInfo bufferCreateInfo = vk::initializers::BufferCreateInfo();
  bufferCreateInfo.size = texSize;
  bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VK_CHECK(vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

  vkGetBufferMemoryRequirements(vkDevice, stagingBuffer, &memReqs);
  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = findMemoryType(vkPhysDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  VK_CHECK(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &stagingMemory));
  VK_CHECK(vkBindBufferMemory(vkDevice, stagingBuffer, stagingMemory, 0));

  u8* data = nullptr;
  VK_CHECK(vkMapMemory(vkDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
  memcpy(data, texData, texSize);
  vkUnmapMemory(vkDevice, stagingMemory);

  std::vector<VkBufferImageCopy> bufferCopyRegions = {}; // copy regions for each mip level

  for (u32 i = 0; i < mipLevels; ++i) {

    //ktx_size_t offset;
    //KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
    //assert(result == KTX_SUCCESS);

    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = i;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = texWidth >> i;
    bufferCopyRegion.imageExtent.height = texHeight >> i;
    bufferCopyRegion.imageExtent.depth = 1;
    bufferCopyRegion.bufferOffset = 0; //offset;

    bufferCopyRegions.push_back(bufferCopyRegion);
  }

  VkImageCreateInfo imageCreateInfo = vk::initializers::ImageCreateInfo();
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = format;
  imageCreateInfo.mipLevels = mipLevels;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.extent = { width, height, 1 };
  imageCreateInfo.usage = imageUsageFlags;

  if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {

    imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  VK_CHECK(vkCreateImage(vkDevice, &imageCreateInfo, nullptr, &image));

  vkGetImageMemoryRequirements(vkDevice, image, &memReqs);
  memAllocInfo.allocationSize = memReqs.size;
  memAllocInfo.memoryTypeIndex = findMemoryType(vkPhysDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK(vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &deviceMemory));
  VK_CHECK(vkBindImageMemory(vkDevice, image, deviceMemory, 0));

  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = mipLevels;
  subresourceRange.layerCount = 1;

  vk::tools::SetImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT); // TODO: Check exception on SetImageLayout

  vkCmdCopyBufferToImage(copyCmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

  this->imageLayout = imageLayout;

  vk::tools::SetImageLayout(copyCmd, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    imageLayout, subresourceRange,
    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

  EndSingleTimeCommands(vkDevice, copyCmd, vkCmdPool, copyQueue);

  vkFreeMemory(vkDevice, stagingMemory, nullptr);
  vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);

  Reignite::Tools::FreeTextureData(texData);

  VkSamplerCreateInfo samplerCreateInfo = {};
  samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
  samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCreateInfo.mipLodBias = 0.0f;
  samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerCreateInfo.minLod = 0.0f;
  // Max level-of-detail should match mip level count
  samplerCreateInfo.maxLod = mipLevels; // TODO: (useStaging) ? (float)mipLevels : 0.0f;
  // Only enable anisotropic filtering if enabled on the devicec
  samplerCreateInfo.maxAnisotropy = 16; // TODO: device->enabledFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
  samplerCreateInfo.anisotropyEnable = 1; // TODO: device->enabledFeatures.samplerAnisotropy;
  samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK(vkCreateSampler(vkDevice, &samplerCreateInfo, nullptr, &sampler));

  VkImageViewCreateInfo viewCreateInfo = {};
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format = format;
  viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
  viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
  // Linear tiling usually won't support mip maps
  // Only set mip map count if optimal tiling is used
  viewCreateInfo.subresourceRange.levelCount = mipLevels; // TODO: (useStaging) ? mipLevels : 1;
  viewCreateInfo.image = image;
  VK_CHECK(vkCreateImageView(vkDevice, &viewCreateInfo, nullptr, &view));

  updateDescriptor();
}