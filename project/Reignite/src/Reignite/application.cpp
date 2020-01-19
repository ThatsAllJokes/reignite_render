#include "application.h"

#include "vulkan_impl.h"

#include "Components/transform_component.h"
#include "Components/camera_component.h"

struct Reignite::State {
  std::string title;
  u16 width;
  u16 height;

  std::vector<u32> indices; // vector that contains every "object" id

  CameraComponent camera;
  std::vector<TransformComponent> transforms;
  //std::vector<GeometryComponent> geometries;
  //std::vector<MaterialComponent> materials;
  //std::vector<RenderComponent> renders;

  //std::vector<Geometry> db_geometries;
  //std::vector<Material> db_materials;
  //std::vector<Texture> db_textures;

  State(const std::string& t = "Reignite Render",
    u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
};

struct Reignite::Application::GFXData {

  VkDebugReportCallbackEXT debugCallback;

  VkInstance instance;
  
  VkPhysicalDevice physicalDevices[16];
  uint32_t physicalDeviceCount;

  VkPhysicalDevice physicalDevice;
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

  Buffer vertexBuffer;
  Buffer indexBuffer;
  std::vector<Buffer> uniformBuffers;

  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
};

const std::vector<Vertex> vertices = {
  {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
  {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
  {{-0.5f,  0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

  {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
  {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
  {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
  {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
};

const std::vector<uint16_t> indices{
  0, 1, 2, 2, 3, 0, // Top
  4, 5, 1, 1, 0, 4, // Back
  1, 5, 6, 6, 2, 1, // Right
  4, 0, 3, 3, 7, 4, // Left
  3, 2, 6, 6, 7, 3, // Front
  4, 6, 5, 6, 4, 7, // Bottom
};

Reignite::Application::Application() {

  state = new State();
  data = new GFXData();

  window = std::unique_ptr<Window>(Window::Create(state));
  component_system = std::unique_ptr<ComponentSystem>(new ComponentSystem(state));

  is_running = true;
}

Reignite::Application::~Application() {

  delete state;
  delete data;
}

void Reignite::Application::Run() {

  initialize();

  VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocateInfo.commandPool = data->commandPool;
  allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocateInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer = 0;
  VK_CHECK(vkAllocateCommandBuffers(data->device, &allocateInfo, &commandBuffer));

  while (!window->closeWindow() && is_running) {

    window->update();

    component_system->update();

    resizeSwapchainIfNecessary(data->swapchain, data->physicalDevice, data->device, data->surface, data->familyIndex, data->swapchainFormat, data->renderPass);

    uint32_t imageIndex = 0;
    VK_CHECK(vkAcquireNextImageKHR(data->device, data->swapchain.swapchain, ~0ull, data->acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

    VK_CHECK(vkResetCommandPool(data->device, data->commandPool, 0));

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkImageMemoryBarrier renderBeginBarrier = imageBarrier(data->swapchain.images[imageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderBeginBarrier);

    VkClearColorValue color = { 48.f / 255.f, 10.f / 255.f, 36.f / 255.f, 1 };
    VkClearValue clearColor = { color };

    VkRenderPassBeginInfo passbeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    passbeginInfo.renderPass = data->renderPass;
    passbeginInfo.framebuffer = data->swapchain.framebuffers[imageIndex];
    passbeginInfo.renderArea.extent.width = data->swapchain.width;
    passbeginInfo.renderArea.extent.height = data->swapchain.height;
    passbeginInfo.clearValueCount = 1;
    passbeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &passbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0, float(data->swapchain.height), float(data->swapchain.width), -float(data->swapchain.height), 0, 1 };
    VkRect2D scissor = { { 0, 0 }, { data->swapchain.width, data->swapchain.height } };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->trianglePipeline);

    VkBuffer vertexBuffers = { data->vertexBuffer.buffer };
    VkDeviceSize offset[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffers, offset);
    vkCmdBindIndexBuffer(commandBuffer, data->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->triangleLayout, 0, 1,
      &data->descriptorSets[0], 0, nullptr);

    //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    VkImageMemoryBarrier renderEndBarrier = imageBarrier(data->swapchain.images[imageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderEndBarrier);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    updateUniformBuffer(data->device, data->uniformBuffers, imageIndex);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &data->acquireSemaphore;
    submitInfo.pWaitDstStageMask = &submitStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &data->releaseSemaphore;

    VK_CHECK(vkQueueSubmit(data->queue, 1, &submitInfo, VK_NULL_HANDLE));

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &data->releaseSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &data->swapchain.swapchain;
    presentInfo.pImageIndices = &imageIndex;

    VK_CHECK(vkQueuePresentKHR(data->queue, &presentInfo));

    VK_CHECK(vkDeviceWaitIdle(data->device));
  }

  VK_CHECK(vkDeviceWaitIdle(data->device)); // Wait for possible running events from the loop to finish

  shutdown();
}

void Reignite::Application::initialize() {

  VK_CHECK(volkInitialize());

  data->instance = createInstance();
  assert(data->instance);

  volkLoadInstance(data->instance);

  data->debugCallback = registerDebugCallback(data->instance);

  data->physicalDeviceCount = sizeof(data->physicalDevices) / sizeof(data->physicalDevices[0]);
  VK_CHECK(vkEnumeratePhysicalDevices(data->instance, &data->physicalDeviceCount, data->physicalDevices));

  data->physicalDevice = pickPhysicalDevice(data->physicalDevices, data->physicalDeviceCount);
  assert(data->physicalDevice);

  data->familyIndex = getGraphicsFamilyIndex(data->physicalDevice);
  assert(data->familyIndex != VK_QUEUE_FAMILY_IGNORED);

  data->device = createDevice(data->instance, data->physicalDevice, data->familyIndex);
  assert(data->device);

  data->surface = createSurface(data->instance, window->GetCurrentWindow());
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

  data->renderPass = createRenderPass(data->device, data->swapchainFormat);
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

  data->trianglePipeline = createGraphicsPipeline(data->device, data->pipelineCache, data->renderPass, data->triangleVS, data->triangleFS, data->triangleLayout);
  assert(data->trianglePipeline);

  createSwapchain(data->swapchain, data->physicalDevice, data->device, data->surface, data->familyIndex, data->swapchainFormat, data->renderPass);

  data->commandPool = createCommandPool(data->device, data->familyIndex);
  assert(data->commandPool);

  data->vertexBuffer = createVertexBuffer(data->device, data->physicalDevice, vertices, data->commandPool, data->queue);
  assert(data->vertexBuffer.buffer);
  assert(data->vertexBuffer.bufferMemory);

  data->indexBuffer = createIndexBuffer(data->device, data->physicalDevice, indices, data->commandPool, data->queue);
  assert(data->indexBuffer.buffer);
  assert(data->indexBuffer.bufferMemory);

  createUniformBuffers(data->device, data->physicalDevice, data->swapchain, data->uniformBuffers);

  data->descriptorPool = createDescriptorPool(data->device, data->swapchain);
  assert(data->descriptorPool);

  data->descriptorSets = createDescriptorSets(data->device, data->swapchain, data->descriptorPool, data->descriptorSetLayout, data->uniformBuffers);
  for (size_t i = 0; i < data->descriptorSets.size(); ++i)
    assert(data->descriptorSets[i]);
}

void Reignite::Application::shutdown() {

  vkDestroyCommandPool(data->device, data->commandPool, 0);

  destroySwapchain(data->device, data->swapchain);

  for (size_t i = 0; i < data->swapchain.images.size(); ++i)
    destroyBuffer(data->device, data->uniformBuffers[i]);

  vkDestroyDescriptorPool(data->device, data->descriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(data->device, data->descriptorSetLayout, nullptr);

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

  window.reset();

  vkDestroyDevice(data->device, 0);

  // Physical device memory is managed by the instance. Don't need to be destroyed manually.

#ifdef _DEBUG
  vkDestroyDebugReportCallbackEXT(data->instance, data->debugCallback, 0);
#endif

  vkDestroyInstance(data->instance, 0);
}



