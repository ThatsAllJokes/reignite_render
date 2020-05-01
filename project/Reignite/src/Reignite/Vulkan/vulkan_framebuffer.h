#ifndef _RI_VULKAN_FRAMEBUFFER_
#define _RI_VULKAN_FRAMEBUFFER_ 1

#include "../basic_types.h"

#include "vulkan_initializers.h"
#include "vulkan_state.h"


namespace vk {

  struct FramebufferAttachment {

    bool hasDepth() {

      std::vector<VkFormat> formats = {
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_X8_D24_UNORM_PACK32,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
      };

      return std::find(formats.begin(), formats.end(), format) != std::end(formats);
    }

    bool hasStencil() {

      std::vector<VkFormat> formats = {
        VK_FORMAT_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
      };

      return std::find(formats.begin(), formats.end(), format) != std::end(formats);
    }

    bool isDepthStencil() { return (hasDepth() || hasStencil()); }

    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkFormat format;
    VkImageSubresourceRange subresourceRange;
    VkAttachmentDescription description;
  };

  struct AttachmentCreateInfo {
    u32 width;
    u32 height;
    u32 layerCount;
    VkFormat format;
    VkImageUsageFlags usage;
  };

  class Framebuffer {
   public:

    Framebuffer(vk::VulkanState* vulkanState) {
      assert(vulkanState);
      this->vulkanState = vulkanState;
    }

    ~Framebuffer() {

      assert(vulkanState);
      for (auto attachment : attachments) {

        vkDestroyImage(vulkanState->device, attachment.image, nullptr);
        vkDestroyImageView(vulkanState->device, attachment.view, nullptr);
        vkFreeMemory(vulkanState->device, attachment.memory, nullptr);
      }

      vkDestroySampler(vulkanState->device, sampler, nullptr);
      vkDestroyRenderPass(vulkanState->device, renderPass, nullptr);
      vkDestroyFramebuffer(vulkanState->device, framebuffer, nullptr);
    }

    u32 addAttachment(vk::AttachmentCreateInfo createInfo) {

      VkImageAspectFlags aspectMask = 0;
      vk::FramebufferAttachment attachment;

      attachment.format = createInfo.format;

      if (createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

      if (createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      
        if (attachment.hasDepth())
          aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (attachment.hasStencil())
          aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
      
      }

      assert(aspectMask > 0);

      VkImageCreateInfo imageCreateInfo = vk::initializers::ImageCreateInfo();
      imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
      imageCreateInfo.format = createInfo.format;
      imageCreateInfo.extent.width = createInfo.width;
      imageCreateInfo.extent.height = createInfo.height;
      imageCreateInfo.extent.depth = 1;
      imageCreateInfo.mipLevels = 1;
      imageCreateInfo.arrayLayers = createInfo.layerCount;
      imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageCreateInfo.usage = createInfo.usage;

      VkMemoryAllocateInfo memAlloc = vk::initializers::MemoryAllocateInfo();
      VkMemoryRequirements memReqs;

      VK_CHECK(vkCreateImage(vulkanState->device, &imageCreateInfo, nullptr, &attachment.image));
      vkGetImageMemoryRequirements(vulkanState->device, attachment.image, &memReqs);
      memAlloc.allocationSize = memReqs.size;
      memAlloc.memoryTypeIndex = vulkanState->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      VK_CHECK(vkAllocateMemory(vulkanState->device, &memAlloc, nullptr, &attachment.memory));
      VK_CHECK(vkBindImageMemory(vulkanState->device, attachment.image, attachment.memory, 0));
      
      attachment.subresourceRange = {};
      attachment.subresourceRange.aspectMask = aspectMask;
      attachment.subresourceRange.levelCount = 1;
      attachment.subresourceRange.layerCount = createInfo.layerCount;

      VkImageViewCreateInfo imageViewCreateInfo = vk::initializers::ImageViewCreateInfo();
      imageViewCreateInfo.viewType = (createInfo.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
      imageViewCreateInfo.format = createInfo.format;
      imageViewCreateInfo.subresourceRange = attachment.subresourceRange;
      imageViewCreateInfo.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
      imageViewCreateInfo.image = attachment.image;
      VK_CHECK(vkCreateImageView(vulkanState->device, &imageViewCreateInfo, nullptr, &attachment.view));
    
      // This is the part I was doing outside the original createAttachment function
      attachment.description = {};
      attachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
      attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachment.description.storeOp = (createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachment.description.format = createInfo.format;
      attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      if (attachment.hasDepth() || attachment.hasStencil()) {

        attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      }
      else {
        attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      }

      attachments.push_back(attachment);
      return static_cast<u32>(attachments.size() - 1);
    }

    VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode) {

      VkSamplerCreateInfo samplerCreateInfo = vk::initializers::SamplerCreateInfo();
      samplerCreateInfo.magFilter = magFilter;
      samplerCreateInfo.minFilter = minFilter;
      samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      samplerCreateInfo.addressModeU = addressMode;
      samplerCreateInfo.addressModeV = addressMode;
      samplerCreateInfo.addressModeW = addressMode;
      samplerCreateInfo.mipLodBias = 0.0f;
      samplerCreateInfo.maxAnisotropy = 1.0f;
      samplerCreateInfo.minLod = 0.0f;
      samplerCreateInfo.maxLod = 1.0f;
      samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

      return vkCreateSampler(vulkanState->device, &samplerCreateInfo, nullptr, &sampler);
    }

    VkResult createRenderPass() {

      std::vector<VkAttachmentDescription> attachmentDescriptions;
      for (auto& attachment : attachments)
        attachmentDescriptions.push_back(attachment.description);

      std::vector<VkAttachmentReference> colorReferences;
      VkAttachmentReference depthReference = {};
      bool hasColor = false;
      bool hasDepth = false;

      u32 attachmentIndex = 0;
      for (auto& attachment : attachments) {

        if (attachment.isDepthStencil()) {

          assert(!hasDepth); // Only one depth attachment
          depthReference.attachment = attachmentIndex;
          depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
          hasDepth = true;
        }
        else {
          colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
          hasColor = true;
        }

        ++attachmentIndex;
      }

      VkSubpassDescription subpassDescription = {};
      subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      if (hasColor) {

        subpassDescription.colorAttachmentCount = static_cast<u32>(colorReferences.size());
        subpassDescription.pColorAttachments = colorReferences.data();
      }

      if(hasDepth)
        subpassDescription.pDepthStencilAttachment = &depthReference;

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
      VK_CHECK(vkCreateRenderPass(vulkanState->device, &renderPassCreateInfo, nullptr, &renderPass));


      std::vector<VkImageView> attachmentViews;
      for (auto attachment : attachments)
        attachmentViews.push_back(attachment.view);

      u32 maxLayers = 0;
      for (auto attachment : attachments) {

        if (attachment.subresourceRange.layerCount > maxLayers) {
          maxLayers = attachment.subresourceRange.layerCount;
        }
      }

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.pAttachments = attachmentViews.data();
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
      framebufferInfo.width = width;
      framebufferInfo.height = height;
      framebufferInfo.layers = maxLayers;
      VK_CHECK(vkCreateFramebuffer(vulkanState->device, &framebufferInfo, nullptr, &framebuffer));


      return VK_SUCCESS;
    }


    u32 width;
    u32 height;
    VkFramebuffer framebuffer;
    VkRenderPass renderPass;
    VkSampler sampler;
    std::vector<vk::FramebufferAttachment> attachments;

   private:

    vk::VulkanState* vulkanState;
  };

}

#endif // _RI_VULKAN_FRAMEBUFFER_