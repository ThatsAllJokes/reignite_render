#include "render_context.h"

#include "display_list.h"

#include "Vulkan/vulkan_impl.h"

#include "Components/transform_component.h"
#include "Components/render_component.h"
#include "Components/light_component.h"
#include "Components/camera_component.h"

#include "GfxResources/geometry_resource.h"
#include "GfxResources/material_resource.h" 


namespace Reignite {

  struct State {

    std::string title;
    u16 width;
    u16 height;

    GLFWwindow* window;

    struct Entity {
      std::vector<s32> entity;
      std::vector<s32> parent;
    } entities;

    CameraComponent camera;
    std::vector<TransformComponent> transform_components;
    std::vector<RenderComponent> render_components;
    std::vector<LightComponent> light_components;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  struct Reignite::RenderContext::Data {

    RenderContextParams params;
    std::vector<Geometry> geometries;
    std::vector<Material> materials;

    // Render state
    bool render_should_close;

    mat4f view_matrix;
    vec3f camera_position;
    mat4f projection_matrix;

    std::vector<mat4f> model_list;

    u32 frameCounter;
    u32 lastFPS;
    // std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;

    // Vulkan
    VkInstance instance;
    VkDebugReportCallbackEXT debugCallback;

    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount;
    
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkCommandPool commandPool;

    uint32_t familyIndex;
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
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout triangleLayout;
    VkPipeline trianglePipeline;

    Swapchain swapchain;

    std::vector<VkCommandBuffer> commandBuffers;

    Image colorImage;
    Image depthImage;
    Image texture;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkSampler textureSampler;
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

  u32 Reignite::RenderContext::createGeometryResource(GeometryEnum geometry, std::string path) {

    Geometry current_geometry;
    switch (geometry) {
    case kGeometryEnum_Square: 
      current_geometry = GeometryResourceSquare();
      break;
    case kGeometryEnum_Cube:
      current_geometry = GeometryResourceCube();
      break;
    case kGeometryEnum_Load: 
      current_geometry = GeometryResourceLoadObj(path);
      break;
    }

    current_geometry.device = data->device;

    current_geometry.vertexBuffer = createVertexBuffer(data->device, data->physicalDevice, current_geometry.vertices, data->commandPool, data->queue);
    assert(current_geometry.vertexBuffer.buffer);
    assert(current_geometry.vertexBuffer.bufferMemory);

    current_geometry.indexBuffer = createIndexBuffer(data->device, data->physicalDevice, current_geometry.indices, data->commandPool, data->queue);
    assert(current_geometry.indexBuffer.buffer);
    assert(current_geometry.indexBuffer.bufferMemory);

    data->geometries.push_back(current_geometry);
    return static_cast<u32>(data->geometries.size() - 1);
  }

  u32 Reignite::RenderContext::createMaterialResource() {

    MaterialResource current_material("Gold", glm::vec3(1.0f, 0.765557f, 0.336057f), 0.1f, 1.0f);
    current_material.device = data->device;

    current_material.uniformBuffer = createUniformBuffer(data->device, data->physicalDevice);
    current_material.lightParams = createUniformBufferParams(data->device, data->physicalDevice);
    current_material.descriptorSet = createDescriptorSets(data->device, data->descriptorPool, 
      data->descriptorSetLayout, current_material.uniformBuffer, current_material.lightParams, 
      data->texture.imageView, data->textureSampler);

    data->materials.push_back(current_material);
    return static_cast<u32>(data->materials.size() - 1);
  }

  void Reignite::RenderContext::setRenderInfo() {
  
    data->view_matrix = state->camera.view_mat;
    data->camera_position = state->camera.position;
    data->projection_matrix = state->camera.projection_mat;

    for (u32 i = 0; i < state->transform_components.size(); ++i) {
      data->model_list.push_back(state->transform_components[i].global);
    }
  }

  void Reignite::RenderContext::submitDisplayList() {

    VK_CHECK(vkResetCommandPool(data->device, data->commandPool, 0));

    for (u32 i = 0; i < data->swapchain.imageCount; ++i) {

      VkCommandBufferBeginInfo beginInfo = vk::initializers::CommandBufferBeginInfo();
      //beginInfo.flags

      VK_CHECK(vkBeginCommandBuffer(data->commandBuffers[i], &beginInfo));

      //VkImageMemoryBarrier renderBeginBarrier = imageBarrier(data->swapchain.images[imageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      //vkCmdPipelineBarrier(data->commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderBeginBarrier);

      VkClearColorValue clearColor = { 48.f / 255.f, 10.f / 255.f, 36.f / 255.f, 1 };
      std::array<VkClearValue, 2> clearValues = {};
      clearValues[0].color = clearColor;
      clearValues[1].depthStencil = { 1.0f, 0 };

      VkRenderPassBeginInfo passbeginInfo = vk::initializers::RenderPassBeginInfo();
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

      std::array<VkBuffer, 2> vertexBuffers = { data->geometries[0].vertexBuffer.buffer, data->geometries[1].vertexBuffer.buffer };
      std::array<VkBuffer, 2> indexBuffers = { data->geometries[0].indexBuffer.buffer, data->geometries[1].indexBuffer.buffer };
      VkDeviceSize offset[] = { 0 };

      for (u32 j = 0; j < 1; ++j) {

        vkCmdBindVertexBuffers(data->commandBuffers[i], 0, 1, &vertexBuffers[j], offset);
        vkCmdBindIndexBuffer(data->commandBuffers[i], indexBuffers[j], 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->triangleLayout, 0, 1,
          &data->materials[j].descriptorSet, 0, nullptr);

        vkCmdPushConstants(data->commandBuffers[i], data->triangleLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MaterialResource::PushBlock), &data->materials[j].params);

        vkCmdDrawIndexed(data->commandBuffers[i], static_cast<uint32_t>(data->geometries[j].indices.size()), 1, 0, 0, 0);
      }

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

    updateUniformBuffers();

    VkSemaphore waitSemaphores[] = { data->acquireSemaphore };
    VkSemaphore signalSemaphores[] = { data->releaseSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo = vk::initializers::SubmitInfo();
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

  void RenderContext::updateUniformBuffers() {

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    for (u32 i = 0; i < 1; ++i) {

      // ubo update -->
      UniformBufferObject ubo = {};
      ubo.model = data->model_list[i];
      ubo.proj = data->projection_matrix;
      ubo.view = data->view_matrix;
      ubo.cam_pos = data->camera_position;
      //ubo.proj[1][1] *= -1; // TODO: I already compensate de Y axis in the viewport. Is that correct?

      MapUniformBuffer(data->device, data->materials[i].uniformBuffer, &ubo, sizeof(ubo));
      
      // ubo lights update ->
      const float p = 10.0f;
      UBOParams uboParams = {};
      uboParams.lights[0] = glm::vec4(-p, 0.0f, -p, 1.0f);
      uboParams.lights[1] = glm::vec4(-p, 0.0f, p, 1.0f);
      uboParams.lights[2] = glm::vec4(p, 0.0f, p, 1.0f);
      uboParams.lights[3] = glm::vec4(p, 0.0f, -p, 1.0f);

      MapUniformBuffer(data->device, data->materials[i].lightParams, &uboParams, sizeof(uboParams));
    }

  }

  void Reignite::RenderContext::initialize(const std::shared_ptr<State> s, const RenderContextParams& params) {

    state = s;
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

#ifdef EXE_ROUTE
    data->triangleVS = loadShader(data->device, "../../../../project/shaders/pbr.vert.spv");
    data->triangleFS = loadShader(data->device, "../../../../project/shaders/pbr.frag.spv");
#else
    data->triangleVS = loadShader(data->device, "../shaders/pbr.vert.spv");
    data->triangleFS = loadShader(data->device, "../shaders/pbr.frag.spv");
#endif
    assert(data->triangleFS);
    assert(data->triangleVS);

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

    data->descriptorPool = createDescriptorPool(data->device, 5, 2);
    assert(data->descriptorPool);

#ifdef EXE_ROUTE
    createGeometryResource(kGeometryEnum_Load, "../../../../project/models/geosphere.obj");
    createGeometryResource(kGeometryEnum_Load, "../../../../project/models/box.obj");
#else
    createGeometryResource(kGeometryEnum_Load, "../models/geosphere.obj");
    createGeometryResource(kGeometryEnum_Load, "../models/box.obj");
#endif

    //createMaterialResource();
    createMaterialResource();
  }

  void Reignite::RenderContext::shutdown() {

    data->render_should_close = true;

    VK_CHECK(vkDeviceWaitIdle(data->device)); // Wait for possible running events from the loop to finish

    vkDestroyCommandPool(data->device, data->commandPool, 0);

    DestroyImage(data->device, data->colorImage);
    DestroyImage(data->device, data->depthImage);
    destroySwapchain(data->device, data->swapchain);

    for (size_t i = 0; i < data->materials.size(); ++i)
      DestroyMaterial(data->materials[i]);

    vkDestroyDescriptorPool(data->device, data->descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(data->device, data->descriptorSetLayout, nullptr);

    vkDestroySampler(data->device, data->textureSampler, nullptr);

    DestroyImage(data->device, data->texture);

    for (u32 i = 0; i < data->geometries.size(); ++i)
      DestroyGeometry(data->geometries[i]);

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