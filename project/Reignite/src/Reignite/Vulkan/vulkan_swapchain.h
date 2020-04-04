#ifndef _RI_VULKAN_SWAPCHAIN_
#define _RI_VULKAN_SWAPCHAIN_ 1

#include <stdlib.h>
#include <vector>

#include <volk.h>

#include "../basic_types.h"

#include "vulkan_tools.h"


typedef struct _SwapchainBuffers {
  VkImage image;
  VkImageView view;
} SwapchainBuffer;

class VulkanSwapchain {
 public:

  void initSurface(void* platformHandle, void* platformWindow) {

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR createInfo = { 
      VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    createInfo.hinstance = (HINSTANCE)platformHandle;
    createInfo.hwnd = (HWND)platformWindow;
    vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
#else
#error Unsupported platform
#endif

    assert(surface);

    u32 queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
    assert(queueCount >= 1);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

    std::vector<VkBool32> supportsPresent(queueCount);
    for (u32 i = 0; i < queueCount; ++i)
      vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);

    u32 graphicsQueueNodeIndex = UINT32_MAX;
    u32 presentQueueNodeIndex = UINT32_MAX;
    for (u32 i = 0; i < queueCount; i++) {

      if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
        
        if (graphicsQueueNodeIndex == UINT32_MAX)
          graphicsQueueNodeIndex = i;

        if (supportsPresent[i] == VK_TRUE) {
          graphicsQueueNodeIndex = i;
          presentQueueNodeIndex = i;
          break;
        }
      }
    }

    if (presentQueueNodeIndex == UINT32_MAX) {

      for (uint32_t i = 0; i < queueCount; ++i) {

        if (supportsPresent[i] == VK_TRUE) {
          presentQueueNodeIndex = i;
          break;
        }
      }
    }

    if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
      exit(EXIT_FAILURE); // Could not find a graphics and/or presenting queue

    if(graphicsQueueNodeIndex != presentQueueNodeIndex)
      exit(EXIT_FAILURE); // Separate graphics and presenting queues are not supported

    queuedNodeIndex = graphicsQueueNodeIndex;

    u32 formatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, 
      surface, &formatCount, nullptr));
    assert(formatCount > 0);
  
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, 
      surface, &formatCount, surfaceFormats.data()));

    if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
      colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
      colorSpace = surfaceFormats[0].colorSpace;
    }
    else {

      bool found_B8G8R8A8_UNORM = false;
      for (auto&& surfaceFormat : surfaceFormats) {

        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
          colorFormat = surfaceFormat.format;
          colorSpace = surfaceFormat.colorSpace;
          found_B8G8R8A8_UNORM = true;
          break;
        }
      }

      if (!found_B8G8R8A8_UNORM) {
        colorFormat = surfaceFormats[0].format;
        colorSpace = surfaceFormats[0].colorSpace;
      }
    }

  }

  void connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
    this->instance = instance;
    this->physicalDevice = physicalDevice;
    this->device = device;
  }

  void create(u32* width, u32* height, bool vsync = false) {

    VkSwapchainKHR oldSwapchain = swapchain;

    VkSurfaceCapabilitiesKHR surfCaps;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCaps));

    u32 presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, 
      &presentModeCount, NULL));
    assert(presentModeCount > 0);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
      &presentModeCount, presentModes.data()));

    VkExtent2D swapchainExtent = {};
    if (surfCaps.currentExtent.width == (u32)-1) {
      swapchainExtent.width = *width;
      swapchainExtent.height = *height;
    }
    else {
      swapchainExtent = surfCaps.currentExtent;
      *width = surfCaps.currentExtent.width;
      *height = surfCaps.currentExtent.height;
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    if (!vsync) {

      for (size_t i = 0; i < presentModeCount; ++i) {

        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
          swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
          break;
        }

        if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && 
          (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {

          swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
      }
    }

    u32 desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
    if ((surfCaps.maxImageCount > 0) && 
      (desiredNumberOfSwapchainImages > surfCaps.maxImageCount)) {

      desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR preTransform;
    if(surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
      preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else {
      preTransform = surfCaps.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
    };

    for (auto& compositeAlphaFlag : compositeAlphaFlags) {
      if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
        compositeAlpha = compositeAlphaFlag;
        break;
      };
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = NULL;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = desiredNumberOfSwapchainImages;
    swapchainCreateInfo.imageFormat = colorFormat;
    swapchainCreateInfo.imageColorSpace = colorSpace;
    swapchainCreateInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = NULL;
    swapchainCreateInfo.presentMode = swapchainPresentMode;
    swapchainCreateInfo.oldSwapchain = oldSwapchain;
    swapchainCreateInfo.clipped = VK_TRUE; // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapchainCreateInfo.compositeAlpha = compositeAlpha;

    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
      swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
      swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VK_CHECK(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain));

    if (oldSwapchain != VK_NULL_HANDLE) {

      for (u32 i = 0; i < imageCount; ++i) {
        vkDestroyImageView(device, buffers[i].view, nullptr);
      }
      vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));

    images.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));

    buffers.resize(imageCount);
    for (u32 i = 0; i < imageCount; ++i) {
      VkImageViewCreateInfo colorAttachmentView = {};
      colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      colorAttachmentView.pNext = NULL;
      colorAttachmentView.format = colorFormat;
      colorAttachmentView.components = { VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
      colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      colorAttachmentView.subresourceRange.baseMipLevel = 0;
      colorAttachmentView.subresourceRange.levelCount = 1;
      colorAttachmentView.subresourceRange.baseArrayLayer = 0;
      colorAttachmentView.subresourceRange.layerCount = 1;
      colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
      colorAttachmentView.flags = 0;

      buffers[i].image = images[i];
      colorAttachmentView.image = buffers[i].image;

      VK_CHECK(vkCreateImageView(device, &colorAttachmentView, nullptr, &buffers[i].view));
    }
  }

  VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, u32* imageIndex) {

    return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
      presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
  }

  VkResult queuePresent(VkQueue queue, u32 imageIndex, 
    VkSemaphore waitSemaphore = VK_NULL_HANDLE) {

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    if (waitSemaphore != VK_NULL_HANDLE) {
      presentInfo.pWaitSemaphores = &waitSemaphore;
      presentInfo.waitSemaphoreCount = 1;
    }

    return vkQueuePresentKHR(queue, &presentInfo);
  }

  void destroy() {

    if (swapchain != VK_NULL_HANDLE) {

      for (u32 i = 0; i < imageCount; ++i)
        vkDestroyImageView(device, buffers[i].view, nullptr);

    }

    if (surface != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device, swapchain, nullptr);
      vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    swapchain = VK_NULL_HANDLE;
    surface = VK_NULL_HANDLE;
  }


  VkFormat colorFormat;
  VkColorSpaceKHR colorSpace;

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  u32 imageCount;
  std::vector<VkImage> images;
  std::vector<SwapchainBuffer> buffers;
  u32 queuedNodeIndex = UINT32_MAX;

 private:
 
  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice physicalDevice;
  VkSurfaceKHR surface;
};

#endif // _RI_VULKAN_SWAPCHAIN_

