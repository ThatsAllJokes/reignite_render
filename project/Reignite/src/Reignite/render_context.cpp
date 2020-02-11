#include "render_context.h"

#include "display_list.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Vulkan/vulkan_impl.h"

#include "Components/transform_component.h"
#include "Components/camera_component.h"

#include "Commands/render_cube_demo_command.h"

namespace Reignite {

  struct State {

    std::string title;
    u16 width;
    u16 height;

    GLFWwindow* window;

    std::vector<u32> indices;

    CameraComponent camera;
    std::vector<TransformComponent> transforms;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  struct Reignite::RenderContext::Data {

    RenderContextParams params;

    // Render state
    bool render_should_close;

    // Vulkan

    u32 frameCounter;
    u32 lastFPS;
    // std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;

    VkDebugReportCallbackEXT debugCallback;

    VkInstance instance;

    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount;

    VkPhysicalDevice physicalDevice;
    //VkPhysicalDeviceProperties physicalDeviceProperties;
    //VkPhysicalDeviceFeatures physicalDeviceFeatures;
    //VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    uint32_t familyIndex;

    VkDevice device;

    VkSurfaceKHR surface;

    VkBool32 presentSupported;

    VkFormat swapchainFormat;

    VkSemaphore acquireSemaphore;
    VkSemaphore releaseSemaphore;

    VkQueue queue;

    VkRenderPass renderPass;
    VkShaderModule triangleVS;
    VkShaderModule triangleFS;

    VkPipelineCache pipelineCache;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout triangleLayout;
    VkPipeline trianglePipeline;

    Swapchain swapchain;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    Image colorImage;
    Image depthImage;
    Image texture;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkSampler textureSampler;

    Buffer vertexBuffer;
    Buffer indexBuffer;
    std::vector<Buffer> uniformBuffers;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
  };

  const std::vector<Vertex> vertices = {
    // TOP
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 0
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 1
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 2
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 3
    // BOTTOM
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 4
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 5
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 6
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 7
    // FRONT
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 8
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 9
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 10
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 11
    // BACK
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 12
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 13
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 14
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 15
    // LEFT
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 16
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 17
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 18
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 19
    // RIGHT
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 20
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 21
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 22
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 23
  };

  const std::vector<uint16_t> indices{
    0, 1, 2, 2, 3, 0, // Top
    4, 6, 5, 6, 4, 7, // Bottom
    8, 9, 10, 10, 11, 8, // Front
    12, 13, 14, 14, 15, 12, // Back
    16, 18, 17, 18, 16, 19, // Left
    20, 21, 22, 22, 23, 20, // Right
  };

  Reignite::RenderContext::RenderContext(const std::shared_ptr<State> s) {

    data = new Data();

    this->state = s;
    initialize(state); // TODO: This should be variable in the future
  }

  Reignite::RenderContext::~RenderContext() {

    shutdown();

    delete data;
  }

  void Reignite::RenderContext::submitDisplayList(Reignite::DisplayList* displayList) {

    VK_CHECK(vkResetCommandPool(data->device, data->commandPool, 0));

    for (u32 i = 0; i < data->swapchain.imageCount; ++i) {

      VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

      VK_CHECK(vkBeginCommandBuffer(data->commandBuffers[i], &beginInfo));

      //VkImageMemoryBarrier renderBeginBarrier = imageBarrier(data->swapchain.images[imageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      //vkCmdPipelineBarrier(data->commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderBeginBarrier);

      VkClearColorValue clearColor = { 48.f / 255.f, 10.f / 255.f, 36.f / 255.f, 1 };
      std::array<VkClearValue, 2> clearValues = {};
      clearValues[0].color = clearColor;
      clearValues[1].depthStencil = { 1.0f, 0 };

      VkRenderPassBeginInfo passbeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
      passbeginInfo.renderPass = data->renderPass;
      passbeginInfo.framebuffer = data->swapchain.framebuffers[i];
      passbeginInfo.renderArea.extent.width = data->swapchain.width;
      passbeginInfo.renderArea.extent.height = data->swapchain.height;
      passbeginInfo.renderArea.offset = { 0, 0 };
      passbeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
      passbeginInfo.pClearValues = clearValues.data();

      vkCmdBeginRenderPass(data->commandBuffers[i], &passbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkViewport viewport = { 0, float(data->swapchain.height), float(data->swapchain.width), -float(data->swapchain.height), 0, 1 };
      VkRect2D scissor = { { 0, 0 }, { data->swapchain.width, data->swapchain.height } };

      vkCmdSetViewport(data->commandBuffers[i], 0, 1, &viewport);
      vkCmdSetScissor(data->commandBuffers[i], 0, 1, &scissor);

      vkCmdBindPipeline(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->trianglePipeline);

      VkBuffer vertexBuffers = { data->vertexBuffer.buffer };
      VkDeviceSize offset[] = { 0 };
      vkCmdBindVertexBuffers(data->commandBuffers[i], 0, 1, &vertexBuffers, offset);
      vkCmdBindIndexBuffer(data->commandBuffers[i], data->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

      vkCmdBindDescriptorSets(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->triangleLayout, 0, 1,
        &data->descriptorSets[0], 0, nullptr);

      vkCmdDrawIndexed(data->commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

      vkCmdBindDescriptorSets(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->triangleLayout, 0, 1,
        &data->descriptorSets[1], 0, nullptr);

      vkCmdDrawIndexed(data->commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

      vkCmdEndRenderPass(data->commandBuffers[i]);

      //VkImageMemoryBarrier renderEndBarrier = imageBarrier(data->swapchain.images[imageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
      //vkCmdPipelineBarrier(data->commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderEndBarrier);

      VK_CHECK(vkEndCommandBuffer(data->commandBuffers[i]));
    }
  }

  void Reignite::RenderContext::draw() {

    // TODO: Check resize functionality
    //resizeSwapchainIfNecessary(data->swapchain, data->physicalDevice, data->device, data->surface, data->familyIndex, data->swapchainFormat, data->renderPass, data->depthImage.imageView);

    uint32_t imageIndex = 0;
    VK_CHECK(vkAcquireNextImageKHR(data->device, data->swapchain.swapchain, ~0ull, data->acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

    updateUniformBuffers(data->device, data->uniformBuffers, imageIndex);

    VkSemaphore waitSemaphores[] = { data->acquireSemaphore };
    VkSemaphore signalSemaphores[] = { data->releaseSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &data->commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VK_CHECK(vkQueueSubmit(data->queue, 1, &submitInfo, VK_NULL_HANDLE));

    VkSwapchainKHR swapChains[] = { data->swapchain.swapchain };

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    VK_CHECK(vkQueuePresentKHR(data->queue, &presentInfo));

    VK_CHECK(vkDeviceWaitIdle(data->device));
  }

  void Reignite::RenderContext::initialize(const std::shared_ptr<State> state, const RenderContextParams& params) {

    data->params = params;

    VK_CHECK(volkInitialize());

    data->instance = createInstance();
    assert(data->instance);

    volkLoadInstance(data->instance);

    data->debugCallback = registerDebugCallback(data->instance);

    data->physicalDeviceCount = sizeof(data->physicalDevices) / sizeof(data->physicalDevices[0]);
    VK_CHECK(vkEnumeratePhysicalDevices(data->instance, &data->physicalDeviceCount, data->physicalDevices));

    data->physicalDevice = pickPhysicalDevice(data->physicalDevices, data->physicalDeviceCount, data->msaaSamples);
    assert(data->physicalDevice);

    data->familyIndex = getGraphicsFamilyIndex(data->physicalDevice);
    assert(data->familyIndex != VK_QUEUE_FAMILY_IGNORED);

    data->device = createDevice(data->instance, data->physicalDevice, data->familyIndex);
    assert(data->device);

    data->surface = createSurface(data->instance, state->window);
    assert(data->surface);

    data->presentSupported = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(data->physicalDevice, data->familyIndex, data->surface, &data->presentSupported));
    assert(data->presentSupported);

    data->swapchainFormat = getSwapchainFormat(data->physicalDevice, data->surface);

    data->acquireSemaphore = createSemaphore(data->device);
    assert(data->acquireSemaphore);

    data->releaseSemaphore = createSemaphore(data->device);
    assert(data->releaseSemaphore);

    data->queue = 0;
    vkGetDeviceQueue(data->device, data->familyIndex, 0, &data->queue);

    data->renderPass = createRenderPass(data->device, data->physicalDevice, data->swapchainFormat, data->msaaSamples);
    assert(data->renderPass);

    data->triangleVS = loadShader(data->device, "../shaders/basic.vert.spv");
    assert(data->triangleVS);

    data->triangleFS = loadShader(data->device, "../shaders/basic.frag.spv");
    assert(data->triangleFS);

    data->pipelineCache = 0; // TODO: This is critical for performance!

    data->descriptorSetLayout = createDescriptorSetLayout(data->device);
    assert(data->descriptorSetLayout);

    data->triangleLayout = createPipelineLayout(data->device, data->descriptorSetLayout);
    assert(data->triangleLayout);

    data->trianglePipeline = createGraphicsPipeline(data->device, data->pipelineCache,
      data->renderPass, data->triangleVS, data->triangleFS, data->triangleLayout, data->msaaSamples);
    assert(data->trianglePipeline);

    CreateColorResources(data->device, data->physicalDevice, data->swapchain, data->colorImage,
      data->swapchainFormat, data->msaaSamples);

    CreateDepthResources(data->device, data->physicalDevice, data->swapchain, data->depthImage,
      data->msaaSamples);

    createSwapchain(data->swapchain, data->physicalDevice, data->device, data->surface,
      data->familyIndex, data->swapchainFormat, data->renderPass, data->depthImage.imageView, data->colorImage.imageView);

    data->commandPool = createCommandPool(data->device, data->familyIndex);
    assert(data->commandPool);
    
    data->commandBuffers = createCommandBuffer(data->device, data->commandPool, data->swapchain.imageCount);
    //assert(data->commandBuffer);

    data->texture = createTextureImage(data->device, data->physicalDevice, data->commandPool, data->queue);
    assert(data->texture.image);
    assert(data->texture.imageMemory);

    data->texture.imageView = createTextureImageView(data->device, data->texture.image, VK_FORMAT_R8G8B8A8_UNORM, data->texture.mipLevels);
    assert(data->texture.imageView);

    data->textureSampler = createTextureSampler(data->device, data->texture.mipLevels);
    assert(data->textureSampler);

    data->vertexBuffer = createVertexBuffer(data->device, data->physicalDevice, vertices, data->commandPool, data->queue);
    assert(data->vertexBuffer.buffer);
    assert(data->vertexBuffer.bufferMemory);

    data->indexBuffer = createIndexBuffer(data->device, data->physicalDevice, indices, data->commandPool, data->queue);
    assert(data->indexBuffer.buffer);
    assert(data->indexBuffer.bufferMemory);

    createUniformBuffers(data->device, data->physicalDevice, data->swapchain, data->uniformBuffers);

    data->descriptorPool = createDescriptorPool(data->device, 5, 2);
    assert(data->descriptorPool);

    data->descriptorSets.resize(2);
    for (size_t i = 0; i < data->descriptorSets.size(); ++i) {

      data->descriptorSets[i] = createDescriptorSets(data->device, data->descriptorPool,
        data->descriptorSetLayout, data->uniformBuffers[i], data->texture.imageView, data->textureSampler);
        assert(data->descriptorSets[i]);
    }
  }

  void Reignite::RenderContext::shutdown() {

    data->render_should_close = true;

    VK_CHECK(vkDeviceWaitIdle(data->device)); // Wait for possible running events from the loop to finish

    vkDestroyCommandPool(data->device, data->commandPool, 0);

    DestroyImage(data->device, data->colorImage);
    DestroyImage(data->device, data->depthImage);
    destroySwapchain(data->device, data->swapchain);

    for (size_t i = 0; i < data->swapchain.images.size(); ++i)
      destroyBuffer(data->device, data->uniformBuffers[i]);

    vkDestroyDescriptorPool(data->device, data->descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(data->device, data->descriptorSetLayout, nullptr);

    vkDestroySampler(data->device, data->textureSampler, nullptr);

    DestroyImage(data->device, data->texture);

    destroyBuffer(data->device, data->vertexBuffer);
    destroyBuffer(data->device, data->indexBuffer);

    vkDestroyPipeline(data->device, data->trianglePipeline, 0);
    vkDestroyPipelineLayout(data->device, data->triangleLayout, 0);

    vkDestroyShaderModule(data->device, data->triangleVS, 0);
    vkDestroyShaderModule(data->device, data->triangleFS, 0);
    vkDestroyRenderPass(data->device, data->renderPass, 0);

    vkDestroySemaphore(data->device, data->acquireSemaphore, 0);
    vkDestroySemaphore(data->device, data->releaseSemaphore, 0);

    vkDestroySurfaceKHR(data->instance, data->surface, 0);

    // TODO: Window may need to be destroyed here

    vkDestroyDevice(data->device, 0);

    // Physical device memory is managed by the instance. Don't need to be destroyed manually.

#ifdef _DEBUG
    vkDestroyDebugReportCallbackEXT(data->instance, data->debugCallback, 0);
#endif

    vkDestroyInstance(data->instance, 0);
  }

}