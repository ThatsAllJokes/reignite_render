#ifndef _VULKAN_IMPL_
#define _VULKAN_IMPL_ 1

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <array>
#include <chrono>
#include <vector>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <volk.h>

#include "../basic_types.h"
#include "../log.h"

#define VK_CHECK(call) \
  do { \
    VkResult result_ = call; \
    assert(result_ == VK_SUCCESS); \
  }while(0)

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif // ARRAYSIZE

//#define EXE_ROUTE 1

struct Vertex {
  float position[3];
  float normal[3];
  float texcoord[2];

  //Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 texc) : position(pos), normal(norm), texcoord(texc) {}

  static VkVertexInputBindingDescription getBindingDescription() {

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;                             // binding number (id?)
    bindingDescription.stride = sizeof(Vertex);                 // distance between two elements (bytes)
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // next data entries could be moved between vertex or instance

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0; // where this attribute gets its data
    attributeDescriptions[0].location = 0; // where this attribute is binded
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // attribute data (size & type)
    attributeDescriptions[0].offset = offsetof(Vertex, position); // number of bytes since start of per-vertex data

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texcoord);

    return attributeDescriptions;
  }
};

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

struct Image {
  VkImage image;
  VkImageView imageView;
  VkDeviceMemory imageMemory;
  uint32_t width, height;
  uint32_t mipLevels;
};

struct Swapchain {

  VkSwapchainKHR swapchain;

  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;
  std::vector<VkFramebuffer> framebuffers;

  uint32_t width, height;
  uint32_t imageCount;
};

struct Buffer {
  VkBuffer buffer;
  VkDeviceMemory bufferMemory;
};

VkInstance createInstance();

VkBool32 debugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
  size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

VkDebugReportCallbackEXT registerDebugCallback(VkInstance instance);

uint32_t getGraphicsFamilyIndex(VkPhysicalDevice physicalDevice);

bool supportsPresentation(VkPhysicalDevice physicalDevice, uint32_t familyIndex);

VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, uint32_t physicalDeviceCount,
  VkSampleCountFlagBits& msaaSamples);

VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t familyIndex);

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);

VkFormat getSwapchainFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR surfaceCaps, 
  uint32_t familyIndex, VkFormat format, uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain);

VkSemaphore createSemaphore(VkDevice device);

VkCommandPool createCommandPool(VkDevice device, uint32_t familyIndex);

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
  VkImageTiling tiling, VkFormatFeatureFlags features);

VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

VkRenderPass createRenderPass(VkDevice device, VkPhysicalDevice physicalDevice,
  VkFormat format, VkSampleCountFlagBits msaaSamples);

VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass renderPass,
  VkImageView imageView, VkImageView depthImageView, VkImageView colorImageView,
  uint32_t width, uint32_t height);

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format,
  VkImageAspectFlags aspectFlags, uint32_t mipLevels);

VkShaderModule loadShader(VkDevice device, const char* path);

VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device);

VkPipeline createGraphicsPipeline(VkDevice device, VkPipelineCache pipelineCache, VkRenderPass renderPass,
  VkShaderModule vs, VkShaderModule fs, VkPipelineLayout layout, VkSampleCountFlagBits msaaSamples);

VkImageMemoryBarrier imageBarrier(VkImage image,
  VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
  VkImageLayout oldLayout, VkImageLayout newLayout);

void createSwapchain(Swapchain& result, VkPhysicalDevice physicalDevice, VkDevice device,
  VkSurfaceKHR surface, uint32_t familyIndex, VkFormat format, VkRenderPass renderPass,
  VkImageView depthImageView, VkImageView colorImageView, VkSwapchainKHR oldSwapchain = 0);

void destroySwapchain(VkDevice device, const Swapchain& swapchain);

// TODO: Check uniform buffers recreation when modifying swapchain
// TODO: Check description pool recreation when modifying swapchain
void resizeSwapchainIfNecessary(Swapchain& result, VkPhysicalDevice physicalDevice,
  VkDevice device, VkSurfaceKHR surface, uint32_t familyIndex, VkFormat format,
  VkRenderPass renderPass, VkImageView depthImageView);

void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, Buffer& buffer,
  VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

void destroyBuffer(VkDevice device, const Buffer& buffer);

VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool,
  VkQueue graphicsQueue);

void copyBuffer(VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
  VkCommandPool commandPool, VkQueue graphicsQueue);

Buffer createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
  const std::vector<Vertex>& vertices, VkCommandPool commandPool, VkQueue graphicsQueue);

Buffer createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
  const std::vector<uint16_t>& indices, VkCommandPool commandPool, VkQueue graphicsQueue);

void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice,
  Swapchain& swapChain, std::vector<Buffer>& uniformBuffers);

VkDescriptorPool createDescriptorPool(VkDevice device, uint32_t maxDescriptors, uint32_t maxSamplers);

VkDescriptorSet createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
  VkDescriptorSetLayout descriptorSetLayout, Buffer& uniformBuffer,
  VkImageView textureImageView, VkSampler textureSampler);

void updateUniformBuffers(VkDevice device, std::vector<Buffer>& uniformBuffers, uint32_t currentImage);

std::vector<VkCommandBuffer> createCommandBuffer(VkDevice device, VkCommandPool commandPool, uint32_t imageCount);

void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
  VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
  Buffer& buffer, Image& texture);

void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
  uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
  VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

void DestroyImage(VkDevice device, Image& image);

void generateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
  VkQueue graphicsQueue, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight,
  uint32_t mipLevels);

Image createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
  VkQueue graphicsQueue);

VkImageView createTextureImageView(VkDevice device, VkImage image, VkFormat format, uint32_t mipLevels);

VkSampler createTextureSampler(VkDevice device, uint32_t mipLevels);

bool HasStencilComponent(VkFormat format);

void CreateDepthResources(VkDevice device, VkPhysicalDevice physicalDevice,
  const Swapchain& swapchain, Image& depthImage, VkSampleCountFlagBits msaaSamples);

void CreateColorResources(VkDevice device, VkPhysicalDevice physicalDevice, const Swapchain& swapchain,
  Image& colorImage, VkFormat swapchainImageFormat, VkSampleCountFlagBits msaaSamples);

#endif // _VULKAN_IMPL_

