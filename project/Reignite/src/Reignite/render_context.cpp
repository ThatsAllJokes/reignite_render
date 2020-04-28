#include "render_context.h"

#include "display_list.h"

#include "tools.h"

#include "window.h"
#include "component_system.h"

#include "Vulkan/vulkan_overlay.h"
#include "Vulkan/vulkan_impl.h"
#include "Vulkan/vulkan_texture.h"
#include "Vulkan/vulkan_state.h"
#include "Vulkan/vulkan_buffer.h"

#include "Components/transform_component.h"
#include "Components/render_component.h"
#include "Components/light_component.h"
#include "Components/camera_component.h"

#include "GfxResources/geometry_resource.h"
#include "GfxResources/material_resource.h" 


namespace Reignite {

  struct State {

    float frameTimer;

    std::shared_ptr<Window> window;
    Input* input;

    vec2f mousePos;
    struct {
      bool left = false;
      bool right = false;
      bool middle = false;
    } mouseButtons;

    std::shared_ptr<ComponentSystem> compSystem;
  };

  struct Reignite::RenderContext::Data {

    RenderContextParams params;
    std::vector<Geometry> geometries;
    std::vector<Material> materials;

    // Render state
    bool render_should_close = false;
    bool deferred_debug_display = false;
    bool display_skybox = true;

    mat4f view_matrix;
    vec3f camera_position;
    mat4f projection_matrix;

    std::vector<mat4f> model_list;

    // Vulkan base
    VkInstance instance;

    VkDebugReportCallbackEXT debugCallback;

    VkPhysicalDevice physicalDevices[16];
    std::string physicalDeviceNames[16];
    u32 physicalDeviceCount;

    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VkPhysicalDeviceFeatures enabledFeatures = {};

    std::vector<const char*> enabledDeviceExtensions;
    std::vector<const char*> enabledInstanceExtensions;

    void* deviceCreatepNextChain = nullptr;

    VkDevice device;

    VkQueue queue;

    VkFormat depthFormat;

    VkCommandPool commandPool;
    
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
    VkSubmitInfo submitInfo;
    
    std::vector<VkCommandBuffer> commandBuffers;
    
    VkRenderPass renderPass;
    
    std::vector<VkFramebuffer> framebuffers;
    
    uint32_t currentBuffer = 0;
    
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    
    std::vector<VkShaderModule> shaderModules;
    
    VkPipelineCache pipelineCache;
    
    VulkanSwapchain swapchain;

    struct {
      VkSemaphore presentComplete;  // swapchain image presentation
      VkSemaphore renderComplete;   // commandBuffer submission and execution
    } semaphores;
    
    std::vector<VkFence> waitFences;

    vk::VulkanState* vulkanState;

    struct {
      VkImage image;
      VkDeviceMemory memory;
      VkImageView view;
    } depthStencil;

    // Deferred pipeline

    struct {
      struct {
        vk::Texture2D colorMap;
        vk::Texture2D normalMap;
        vk::Texture2D roughness;
        vk::Texture2D metallic;
      } model;
      struct {
        vk::Texture2D colorMap;
        vk::Texture2D normalMap;
        vk::Texture2D roughness;
        vk::Texture2D metallic;
      } floor;
    } textures;

    struct {
      VkPipelineVertexInputStateCreateInfo inputState;
      std::vector<VkVertexInputBindingDescription> bindingDescriptions;
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    struct {
      mat4f projection;
      mat4f model;
      mat4f view;
      vec4f instancePos[3];
    } uboVS, uboOffscreenVS;

    struct {
      mat4f projection;
      mat4f model;
    } skyboxUboVS;

    struct Light {
      vec4f position;
      vec3f color;
      float radius;
    };

    struct {
      Light lights[6];
      vec4f viewPos;
    } uboFragmentLights;

    struct {
      vk::Buffer vsFullScreen;
      vk::Buffer vsOffscreen;
      vk::Buffer fsLights;
      vk::Buffer skybox;
    } uniformBuffers;

    struct {
      VkPipeline deferred;
      VkPipeline offscreen;
      VkPipeline debug;
      VkPipeline skybox;
    } pipelines;

    struct {
      VkPipelineLayout deferred;
      VkPipelineLayout offscreen;
      VkPipelineLayout skybox;
    } pipelineLayouts;

    struct {
      VkDescriptorSet model;
      VkDescriptorSet floor;
      VkDescriptorSet skybox;
    } descriptorSets;

    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    VkDescriptorSetLayout skyboxDescriptorSetLayout;

    struct FrameBuffer {
      int32_t width, height;
      VkFramebuffer frameBuffer;
      FrameBufferAttachment position;
      FrameBufferAttachment normal;
      FrameBufferAttachment albedo;
      FrameBufferAttachment roughness;
      FrameBufferAttachment metallic;
      FrameBufferAttachment depth;
      VkRenderPass renderPass;
    } offScreenFrameBuf;

    VkSampler colorSampler;
    VkCommandBuffer offScreenCmdBuffer = VK_NULL_HANDLE;
    VkSemaphore offScreenSemaphore = VK_NULL_HANDLE;

    vk::Buffer tmp_vertices;
    vk::Buffer tmp_indices;
    u32 tmp_indexCount;

    vk::Overlay overlay;
    vk::TextureCubeMap cubeMap;
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
    case kGeometryEnum_Terrain: 
      current_geometry = GeometryTerrain();
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
    /*current_material.descriptorSet = createDescriptorSets(data->device, data->descriptorPool, 
      data->descriptorSetLayout, current_material.uniformBuffer, current_material.lightParams, 
      data->texture.imageView, data->textureSampler);*/

    data->materials.push_back(current_material);
    return static_cast<u32>(data->materials.size() - 1);
  }

  void Reignite::RenderContext::setRenderInfo() {
  
    //data->view_matrix = state->compSystem->camera.view;
    //data->camera_position = state->camera.position;
    //data->projection_matrix = state->camera.projection;

    /*for (u32 i = 0; i < state->transform_components.size(); ++i) {
      data->model_list.push_back(state->transform_components->transformComponents.global[i]);
    }*/
  }

  void Reignite::RenderContext::buildDeferredCommands() {

    if (data->offScreenCmdBuffer == VK_NULL_HANDLE) {
      
      VkCommandBufferAllocateInfo cmdBuffAllocateInfo =
        vk::initializers::CommandBufferAllocateInfo(
          data->commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

      VkCommandBuffer cmdBuffer;
      VK_CHECK(vkAllocateCommandBuffers(data->device, &cmdBuffAllocateInfo, &cmdBuffer));
      data->offScreenCmdBuffer = cmdBuffer;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = vk::initializers::SemaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(data->device, &semaphoreCreateInfo, nullptr, &data->offScreenSemaphore));
  
    VkCommandBufferBeginInfo cmdBufferInfo = vk::initializers::CommandBufferBeginInfo();

    std::array<VkClearValue, 6> clearValues;
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[3].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[4].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[5].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vk::initializers::RenderPassBeginInfo();
    renderPassBeginInfo.renderPass = data->offScreenFrameBuf.renderPass;
    renderPassBeginInfo.framebuffer = data->offScreenFrameBuf.frameBuffer;
    renderPassBeginInfo.renderArea.extent.width = data->offScreenFrameBuf.width;
    renderPassBeginInfo.renderArea.extent.height = data->offScreenFrameBuf.height;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    VK_CHECK(vkBeginCommandBuffer(data->offScreenCmdBuffer, &cmdBufferInfo));

    vkCmdBeginRenderPass(data->offScreenCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = vk::initializers::Viewport(
      (float)data->offScreenFrameBuf.width, (float)data->offScreenFrameBuf.height, 0.0f, 1.0f);
    vkCmdSetViewport(data->offScreenCmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = vk::initializers::Rect2D(
      data->offScreenFrameBuf.width, data->offScreenFrameBuf.height, 0, 0);
    vkCmdSetScissor(data->offScreenCmdBuffer, 0, 1, &scissor);

    VkDeviceSize offsets[1] = { 0 };

    // Skybox
    if (data->display_skybox) {

      vkCmdBindDescriptorSets(data->offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelineLayouts.skybox, 0, 1, &data->descriptorSets.skybox, 0, NULL);
      vkCmdBindVertexBuffers(data->offScreenCmdBuffer, 0, 1, &data->geometries[1].vertexBuffer.buffer, offsets);
      vkCmdBindIndexBuffer(data->offScreenCmdBuffer, data->geometries[1].indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
      vkCmdBindPipeline(data->offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelines.skybox);
      vkCmdDrawIndexed(data->offScreenCmdBuffer, (u32)data->geometries[1].indices.size(), 1, 0, 0, 0);
    }

    vkCmdBindPipeline(data->offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelines.offscreen);

    // Background
    vkCmdBindDescriptorSets(data->offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelineLayouts.offscreen, 0, 1, &data->descriptorSets.floor, 0, NULL);
    vkCmdBindVertexBuffers(data->offScreenCmdBuffer, 0, 1, &data->geometries[0].vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(data->offScreenCmdBuffer, data->geometries[0].indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(data->offScreenCmdBuffer, (u32)data->geometries[0].indices.size(), 1, 0, 0, 0);

    // Object
    vkCmdBindDescriptorSets(data->offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelineLayouts.offscreen, 0, 1, &data->descriptorSets.model, 0, NULL);
    vkCmdBindVertexBuffers(data->offScreenCmdBuffer, /*VERTEX_BUFFER_BIND_ID*/0, 1, &data->geometries[2].vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(data->offScreenCmdBuffer, data->geometries[2].indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(data->offScreenCmdBuffer, (u32)data->geometries[2].indices.size(), 1, 0, 0, 0);

    vkCmdEndRenderPass(data->offScreenCmdBuffer);

    VK_CHECK(vkEndCommandBuffer(data->offScreenCmdBuffer));
  }

  void Reignite::RenderContext::buildCommandBuffers() {

    VkCommandBufferBeginInfo cmdBufferInfo = vk::initializers::CommandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.2f, 0.2f, 0.2f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vk::initializers::RenderPassBeginInfo();
    renderPassBeginInfo.renderPass = data->renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = state->window->width();
    renderPassBeginInfo.renderArea.extent.height = state->window->height();
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (s32 i = 0; i < data->commandBuffers.size(); ++i) {
    
      renderPassBeginInfo.framebuffer = data->framebuffers[i];

      VK_CHECK(vkBeginCommandBuffer(data->commandBuffers[i], &cmdBufferInfo));

      vkCmdBeginRenderPass(data->commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkViewport viewport = vk::initializers::Viewport((float)state->window->width(), (float)state->window->height(), 0.0f, 1.0f);
      vkCmdSetViewport(data->commandBuffers[i], 0, 1, &viewport);

      VkRect2D scissor = vk::initializers::Rect2D(state->window->width(), state->window->height(), 0, 0);
      vkCmdSetScissor(data->commandBuffers[i], 0, 1, &scissor);

      VkDeviceSize offsets[1] = { 0 };

      if (data->deferred_debug_display) {

        vkCmdBindDescriptorSets(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelineLayouts.deferred, 0, 1, &data->descriptorSet, 0, NULL);
        vkCmdBindPipeline(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelines.debug);
        vkCmdBindVertexBuffers(data->commandBuffers[i], /*VERTEX_BUFFER_BIND_ID*/0, 1, &data->tmp_vertices.buffer, offsets);
        vkCmdBindIndexBuffer(data->commandBuffers[i], data->tmp_indices.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(data->commandBuffers[i], data->tmp_indexCount, 1, 0, 0, 1);
        // Move viewport to display final composition in lower right corner
        viewport.x = viewport.width * 0.5f;
        viewport.y = viewport.height * 0.5f;
        viewport.width = viewport.width * 0.5f;
        viewport.height = viewport.height * 0.5f;
        vkCmdSetViewport(data->commandBuffers[i], 0, 1, &viewport);
      }

      vkCmdBindDescriptorSets(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelineLayouts.deferred, 0, 1, &data->descriptorSet, 0, NULL);
      vkCmdBindPipeline(data->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data->pipelines.deferred);
      //vkCmdBindVertexBuffers(data->commandBuffers[i], /*VERTEX_BUFFER_BIND_ID*/0, 1, &data->tmp_vertices.buffer, offsets);
      //vkCmdBindIndexBuffer(data->commandBuffers[i], data->tmp_indices.buffer, 0, VK_INDEX_TYPE_UINT32);
      vkCmdDraw(data->commandBuffers[i], 6, 1, 0, 0);

      // Draw UI call should be here
      {
        const VkViewport viewport = vk::initializers::Viewport((float)state->window->width(), (float)state->window->height(), 0.0f, 1.0f);
        const VkRect2D scissor = vk::initializers::Rect2D(state->window->width(), state->window->height(), 0, 0);
        vkCmdSetViewport(data->commandBuffers[i], 0, 1, &viewport);
        vkCmdSetScissor(data->commandBuffers[i], 0, 1, &scissor);

        data->overlay.draw(data->commandBuffers[i]);
      }
      // UI draw calls

      vkCmdEndRenderPass(data->commandBuffers[i]);

      VK_CHECK(vkEndCommandBuffer(data->commandBuffers[i]));
    }

  }

  void Reignite::RenderContext::draw() {

    updateUniformBufferDeferredMatrices();
    updateUniformBufferDeferredLights();

    // Imgui setup
    {
      ImGuiIO& io = ImGui::GetIO();
      io.DisplaySize = ImVec2((float)state->window->width(), (float)state->window->height());
      io.DeltaTime = state->frameTimer;

      io.MousePos = ImVec2(state->mousePos.x, state->mousePos.y);
      io.MouseDown[0] = state->mouseButtons.left;
      io.MouseDown[1] = state->mouseButtons.right;
    }

    // prepare frame
    {
      VkResult result = data->swapchain.acquireNextImage(
        data->semaphores.presentComplete, &data->currentBuffer);

      if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
        // TODO: window resize
      }
      else {
        VK_CHECK(result);
      }
    }

    // submiting config
    {
      data->submitInfo.pWaitSemaphores = &data->semaphores.presentComplete;
      data->submitInfo.pSignalSemaphores = &data->offScreenSemaphore;

      data->submitInfo.commandBufferCount = 1;
      data->submitInfo.pCommandBuffers = &data->offScreenCmdBuffer;
      VK_CHECK(vkQueueSubmit(data->queue, 1, &data->submitInfo, VK_NULL_HANDLE));

      data->submitInfo.pWaitSemaphores = &data->offScreenSemaphore;
      data->submitInfo.pSignalSemaphores = &data->semaphores.renderComplete;

      data->submitInfo.pCommandBuffers = &data->commandBuffers[data->currentBuffer];
      VK_CHECK(vkQueueSubmit(data->queue, 1, &data->submitInfo, VK_NULL_HANDLE));
    }

    // submit frame
    {
      VkResult result = data->swapchain.queuePresent(data->queue, 
        data->currentBuffer, data->semaphores.renderComplete);

      if (!((result = VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
          // window resize
          return;
        }
        else {
          VK_CHECK(result);
        }
      }

      VK_CHECK(vkQueueWaitIdle(data->queue));
    }

    // update overlay
    {
      ImGuiIO& io = ImGui::GetIO();

      io.DisplaySize = ImVec2((float)state->window->width(), (float)state->window->height());
      io.DeltaTime = state->frameTimer;

      io.MousePos = ImVec2(state->mousePos.x, state->mousePos.y);
      io.MouseDown[0] = state->mouseButtons.left;
      io.MouseDown[1] = state->mouseButtons.right;

      ImGui::NewFrame();

      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::SetNextWindowPos(ImVec2(10, 10));
      ImGui::Begin("Test UI", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
      ImGui::TextUnformatted(state->window->title().c_str());
      ImGui::TextUnformatted(data->physicalDeviceNames[0].c_str());
      //ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

      ImGui::PushItemWidth(110.0f * data->overlay.scale);
    
      //OnUpdateUIOverlay(&UIOverlay);
      bool res = ImGui::Checkbox("Show render targets", &data->deferred_debug_display);
      if (res) { 
        data->overlay.updated = true;
        buildCommandBuffers();
        updateUniformBuffersScreen();
      }

      res = ImGui::Checkbox("Display skybox", &data->display_skybox);
      if (res) {
        data->overlay.updated = true;
        buildCommandBuffers();
        buildDeferredCommands();
        updateUniformBuffersScreen();
      }


      ImGui::PopItemWidth();

      ImGui::End();
      ImGui::PopStyleVar();

      ImGui::ShowDemoWindow();

      ImGui::Render();

      if (data->overlay.update() || data->overlay.updated) {
        buildCommandBuffers();
        data->overlay.updated = false;
      }
    }

  }

  void RenderContext::updateUniformBuffersScreen() {

    setRenderInfo();

    if(data->deferred_debug_display) {

      data->uboVS.projection = glm::ortho(0.0f, 2.0f, 0.0f, 2.0f, -1.0f, 1.0f);
    }
    else {

      data->uboVS.projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
    }

    data->uboVS.model = mat4f(1.0f);
    memcpy(data->uniformBuffers.vsFullScreen.mapped, &data->uboVS, sizeof(data->uboVS));

    mat4f viewMatrix = glm::mat4(1.0f);
    data->skyboxUboVS.projection = glm::perspective(glm::radians(60.0f), (float)state->window->width() / (float)state->window->height(), 0.001f, 256.0f);

    data->skyboxUboVS.model = glm::mat4(1.0f);
    data->skyboxUboVS.model = viewMatrix * glm::translate(data->skyboxUboVS.model, glm::vec3(0, 0, 0));
    data->skyboxUboVS.model = glm::rotate(data->skyboxUboVS.model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    data->skyboxUboVS.model = glm::rotate(data->skyboxUboVS.model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    data->skyboxUboVS.model = glm::rotate(data->skyboxUboVS.model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    memcpy(data->uniformBuffers.skybox.mapped, &data->skyboxUboVS, sizeof(data->skyboxUboVS));
  }

  void RenderContext::updateUniformBufferDeferredMatrices() {

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float timer = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


    data->uboOffscreenVS.projection = state->compSystem->projection();
    data->uboOffscreenVS.view = state->compSystem->view();
    data->uboOffscreenVS.model = glm::mat4(1.0f);

    memcpy(data->uniformBuffers.vsOffscreen.mapped, &data->uboOffscreenVS, sizeof(data->uboOffscreenVS));
  }

  void RenderContext::updateUniformBufferDeferredLights() {

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float timer = 0.1f * std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    // White
    data->uboFragmentLights.lights[0].position = glm::vec4(0.0f, 4.0f, 0.0f, 0.0f);
    data->uboFragmentLights.lights[0].color = glm::vec3(1.0f);
    data->uboFragmentLights.lights[0].radius = 35.0f;
    // Red
    data->uboFragmentLights.lights[1].position = glm::vec4(6.0f, 0.0f, 0.0f, 0.0f);
    data->uboFragmentLights.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f); //glm::vec3(1.0f, 0.0f, 0.0f);
    data->uboFragmentLights.lights[1].radius = 35.0f;
    // Blue
    data->uboFragmentLights.lights[2].position = glm::vec4(2.0f, 1.0f, 0.0f, 0.0f);
    data->uboFragmentLights.lights[2].color = glm::vec3(1.5f); //glm::vec3(0.0f, 0.0f, 2.5f);
    data->uboFragmentLights.lights[2].radius = 5.0f;
    // Yellow
    data->uboFragmentLights.lights[3].position = glm::vec4(0.0f, 0.9f, 0.5f, 0.0f);
    data->uboFragmentLights.lights[3].color = glm::vec3(1.5f); //glm::vec3(1.0f, 1.0f, 0.0f);
    data->uboFragmentLights.lights[3].radius = 2.0f;
    // Green
    data->uboFragmentLights.lights[4].position = glm::vec4(0.0f, 0.5f, 0.0f, 0.0f);
    data->uboFragmentLights.lights[4].color = glm::vec3(1.5f); //glm::vec3(0.0f, 1.0f, 0.2f);
    data->uboFragmentLights.lights[4].radius = 5.0f;
    // Yellow
    data->uboFragmentLights.lights[5].position = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    data->uboFragmentLights.lights[5].color = glm::vec3(1.5f); //glm::vec3(1.0f, 0.7f, 0.3f);
    data->uboFragmentLights.lights[5].radius = 25.0f;

    data->uboFragmentLights.lights[0].position.x = sin(glm::radians(360.0f * timer)) * 5.0f;
    data->uboFragmentLights.lights[0].position.z = cos(glm::radians(360.0f * timer)) * 5.0f;

    data->uboFragmentLights.lights[1].position.x = 4.0f + sin(glm::radians(360.0f * timer) + 45.0f) * 2.0f;
    //data->uboFragmentLights.lights[1].position.z = 0.0f + cos(glm::radians(360.0f * timer) + 45.0f) * 2.0f;

    data->uboFragmentLights.lights[2].position.x = 4.0f + sin(glm::radians(360.0f * timer)) * 2.0f;
    data->uboFragmentLights.lights[2].position.z = 0.0f + cos(glm::radians(360.0f * timer)) * 2.0f;

    data->uboFragmentLights.lights[4].position.x = 0.0f + sin(glm::radians(360.0f * timer + 90.0f)) * 5.0f;
    data->uboFragmentLights.lights[4].position.z = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

    data->uboFragmentLights.lights[5].position.x = 0.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 10.0f;
    data->uboFragmentLights.lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * timer - 45.0f)) * 10.0f;

    // Current view position
    data->uboFragmentLights.viewPos = glm::vec4(state->compSystem->viewPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

    memcpy(data->uniformBuffers.fsLights.mapped, &data->uboFragmentLights, sizeof(data->uboFragmentLights));

  }

  void RenderContext::loadResources() {
    /*
    data->textures.model.colorMap.loadFromFile(Reignite::Tools::GetAssetPath() + "textures/stonefloor01_color_bc3_unorm.ktx", VK_FORMAT_BC3_UNORM_BLOCK, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.model.normalMap.loadFromFile(Reignite::Tools::GetAssetPath() + "textures/stonefloor01_normal_bc3_unorm.ktx", VK_FORMAT_BC3_UNORM_BLOCK, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.colorMap.loadFromFile(Reignite::Tools::GetAssetPath() + "textures/stonefloor01_color_bc3_unorm.ktx", VK_FORMAT_BC3_UNORM_BLOCK, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.normalMap.loadFromFile(Reignite::Tools::GetAssetPath() + "textures/stonefloor01_normal_bc3_unorm.ktx", VK_FORMAT_BC3_UNORM_BLOCK, data->device, data->physicalDevice, data->commandPool, data->queue);
    */

    data->textures.model.colorMap.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_albedo.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.model.normalMap.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_normal.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.model.roughness.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_roughness.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.model.metallic.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_metallic.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
/*
    data->textures.floor.colorMap.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_albedo.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.normalMap.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_normal.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.roughness.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_roughness.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.metallic.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Marble_SlabWhite_1K_metallic.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    */
    
    data->textures.floor.colorMap.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Metal_BronzePolished_1K_albedo.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.normalMap.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Metal_BronzePolished_1K_normal.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.roughness.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Metal_BronzePolished_1K_roughness.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    data->textures.floor.metallic.loadFromFileSTB(Reignite::Tools::GetAssetPath() + "textures/TexturesCom_Metal_BronzePolished_1K_metallic.jpg", VK_FORMAT_R8G8B8A8_SRGB, data->device, data->physicalDevice, data->commandPool, data->queue);
    

    std::string filename;
    VkFormat format;
    if (data->deviceFeatures.textureCompressionBC) {
      filename = "textures/cubemap_yokohama_bc3_unorm.ktx";
      format = VK_FORMAT_BC2_UNORM_BLOCK;
    }
    else if (data->deviceFeatures.textureCompressionASTC_LDR) {
      filename = "textures/cubemap_yokohama_astc_8x8_unorm.ktx";
      format = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    }
    else if (data->deviceFeatures.textureCompressionETC2) {
      filename = "textures/cubemap_yokohama_etc2_unorm.ktx";
      format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    }
    else {
      assert(!"Compressed texture format not supported");
    }

    data->cubeMap.loadFromFile(Reignite::Tools::GetAssetPath() + filename, 
      format, data->vulkanState, data->queue);
  }

  void Reignite::RenderContext::initialize(const std::shared_ptr<State> s, const RenderContextParams& params) {

    state = s;
    data->params = params;

    VK_CHECK(volkInitialize());

    // init vulkan

    data->instance = createInstance();
    assert(data->instance);

    volkLoadInstance(data->instance);

    data->debugCallback = registerDebugCallback(data->instance);

    data->physicalDeviceCount = sizeof(data->physicalDevices) / sizeof(data->physicalDevices[0]);
    VK_CHECK(vkEnumeratePhysicalDevices(data->instance, &data->physicalDeviceCount, data->physicalDevices));

    VkSampleCountFlagBits filler;
    data->physicalDevice = pickPhysicalDevice(data->physicalDevices, data->physicalDeviceNames, data->physicalDeviceCount, filler);
    vkGetPhysicalDeviceProperties(data->physicalDevice, &data->deviceProperties);
    vkGetPhysicalDeviceFeatures(data->physicalDevice, &data->deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(data->physicalDevice, &data->deviceMemoryProperties);

    if (data->deviceFeatures.samplerAnisotropy) {
      data->enabledFeatures.samplerAnisotropy = VK_TRUE;
    }

    if (data->deviceFeatures.textureCompressionBC) {
      data->enabledFeatures.textureCompressionBC = VK_TRUE;
    }
    else if (data->deviceFeatures.textureCompressionASTC_LDR) {
      data->enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
    }
    else if (data->deviceFeatures.textureCompressionETC2) {
      data->enabledFeatures.textureCompressionETC2 = VK_TRUE;
    }

    data->vulkanState = new vk::VulkanState(data->physicalDevice);
    VkResult res = data->vulkanState->createDevice(data->enabledFeatures, data->enabledDeviceExtensions, data->deviceCreatepNextChain);
    if (res != VK_SUCCESS) {
      assert(res == VK_SUCCESS);
      return;
    }

    data->device = data->vulkanState->device;

    vkGetDeviceQueue(data->device, data->vulkanState->queueFamilyIndices.graphics, 0, &data->queue);

    VkBool32 validDepthFormat = vk::tools::GetSupportedDepthFormat(data->physicalDevice, &data->depthFormat);
    assert(validDepthFormat);

    data->swapchain.connect(data->instance, data->physicalDevice, data->device);

    VkSemaphoreCreateInfo semaphoreCreateInfo = vk::initializers::SemaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(data->device, &semaphoreCreateInfo, nullptr, &data->semaphores.presentComplete));
    VK_CHECK(vkCreateSemaphore(data->device, &semaphoreCreateInfo, nullptr, &data->semaphores.renderComplete));

    data->submitInfo = vk::initializers::SubmitInfo();
    data->submitInfo.pWaitDstStageMask = &data->submitPipelineStages;
    data->submitInfo.waitSemaphoreCount = 1;
    data->submitInfo.pWaitSemaphores = &data->semaphores.presentComplete;
    data->submitInfo.signalSemaphoreCount = 1;
    data->submitInfo.pSignalSemaphores = &data->semaphores.renderComplete;

    // init swapchain
    data->swapchain.initSurface((void*)GetModuleHandle(0), (void*)glfwGetWin32Window((GLFWwindow*)state->window->currentWindow()));

    // create command pool
    VkCommandPoolCreateInfo cmdPoolInfo = vk::initializers::CommandPoolCreateInfo();
    cmdPoolInfo.queueFamilyIndex = data->swapchain.queuedNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(data->device, &cmdPoolInfo, nullptr, &data->commandPool));

    // setup swapchain
    u32 auxWidth =  (u32)state->window->width();
    u32 auxHeight = (u32)state->window->height(); // TODO: Modify window size to u32 type
    data->swapchain.create(&auxWidth, &auxHeight);

    // create command buffers
    data->commandBuffers.resize(data->swapchain.imageCount);
    VkCommandBufferAllocateInfo cmdBuffAllocateInfo =
      vk::initializers::CommandBufferAllocateInfo(data->commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<u32>(data->commandBuffers.size()));
    VK_CHECK(vkAllocateCommandBuffers(data->device, &cmdBuffAllocateInfo, data->commandBuffers.data()));

    // create sync primitives
    VkFenceCreateInfo fenceCreateInfo =
      vk::initializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    data->waitFences.resize(data->commandBuffers.size());
    for (auto& fence : data->waitFences)
      VK_CHECK(vkCreateFence(data->device, &fenceCreateInfo, nullptr, &fence));

    // setup depth stencil?
    {
      VkImageCreateInfo imageCreateInfo = {};
      imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
      imageCreateInfo.format = data->depthFormat;
      imageCreateInfo.extent = { auxWidth, auxHeight, 1 };
      imageCreateInfo.mipLevels = 1;
      imageCreateInfo.arrayLayers = 1;
      imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

      VK_CHECK(vkCreateImage(data->device, &imageCreateInfo, nullptr, &data->depthStencil.image));

      VkMemoryRequirements memoryReqs = {};
      vkGetImageMemoryRequirements(data->device, data->depthStencil.image, &memoryReqs);

      VkMemoryAllocateInfo memAlloc = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
      memAlloc.allocationSize = memoryReqs.size;
      memAlloc.memoryTypeIndex = data->vulkanState->getMemoryType(memoryReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      VK_CHECK(vkAllocateMemory(data->device, &memAlloc, nullptr, &data->depthStencil.memory));
      VK_CHECK(vkBindImageMemory(data->device, data->depthStencil.image, data->depthStencil.memory, 0));

      VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
      imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      imageViewCreateInfo.image = data->depthStencil.image;
      imageViewCreateInfo.format = data->depthFormat;
      imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
      imageViewCreateInfo.subresourceRange.levelCount = 1;
      imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
      imageViewCreateInfo.subresourceRange.layerCount = 1;
      imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

      if (data->depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }
      VK_CHECK(vkCreateImageView(data->device, &imageViewCreateInfo, nullptr, &data->depthStencil.view));
    }

    // setup renderPass
    {
      std::vector<VkAttachmentDescription> attachments(2);
      attachments[0].format = data->swapchain.colorFormat;
      attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
      attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      attachments[1].format = data->depthFormat;
      attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
      attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      std::vector<VkAttachmentReference> colorReference = {};
      colorReference.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

      std::vector<VkAttachmentReference> depthReference = {};
      depthReference.push_back({ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

      VK_CHECK(CreateRenderPass(data->device, data->renderPass, 
        colorReference, depthReference, attachments));
    }

    // create Pipeline cache
    {
      VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
      VK_CHECK(vkCreatePipelineCache(data->device, &pipelineCacheCreateInfo, nullptr, &data->pipelineCache));
    }

    // setup framebuffer
    {
      std::vector<VkImageView> attachments(2);
      attachments[1] = data->depthStencil.view;

      data->framebuffers.resize(data->swapchain.imageCount);
      for (u32 i = 0; i < data->framebuffers.size(); i++) {
      
        attachments[0] = data->swapchain.buffers[i].view;
        VK_CHECK(CreateFramebuffer(data->device, data->framebuffers[i], data->renderPass,
          state->window->width(), state->window->height(), attachments));
      }
    }

    // setup overlay
    {
      data->overlay.state = data->vulkanState;
      data->overlay.queue = data->queue;
      data->overlay.shaders = {
        loadShader(data->device, Reignite::Tools::GetAssetPath() + "shaders/ui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        loadShader(data->device, Reignite::Tools::GetAssetPath() + "shaders/ui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
      };
      data->overlay.prepareResources();
      data->overlay.preparePipeline(data->pipelineCache, data->renderPass);
    }

    // Deferred features initialization ->
    
    // load resources
    loadResources();

    // Generate Quads
    {
      std::vector<Vertex> vertexBuffer;

      float x = 0.0f;
      float y = 0.0f;
      for(u32 i = 0; i < 3; ++i) {
    
        vertexBuffer.push_back({ { x + 1.0f, y + 1.0f, 0.0f }, { 0.0f, 0.0f, (float)i }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } });
        vertexBuffer.push_back({ { x,        y + 1.0f, 0.0f }, { 0.0f, 0.0f, (float)i }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } });
        vertexBuffer.push_back({ { x,        y,        0.0f }, { 0.0f, 0.0f, (float)i }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } });
        vertexBuffer.push_back({ { x + 1.0f, y,        0.0f }, { 0.0f, 0.0f, (float)i }, { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } });
    
        x += 1.0f;
        if (x > 1.0f) {
          x = 0.0f;
          y += 1.0f;
        }
      }

      VK_CHECK(data->vulkanState->createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer.size() * sizeof(Vertex),
        &data->tmp_vertices.buffer,
        &data->tmp_vertices.memory,
        vertexBuffer.data()));

      std::vector<u32> indexBuffer = { 0, 1, 2, 2, 3, 0 };
      for (u32 i = 0; i < 3; ++i) {

        u32 indices[6] = { 0, 1, 2, 2, 3, 0 };
        for (auto index : indices) {

          indexBuffer.push_back(i * 4 + index);
        }
      }

      data->tmp_indexCount = static_cast<uint32_t>(indexBuffer.size());

      VK_CHECK(data->vulkanState->createBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBuffer.size() * sizeof(uint32_t),
        &data->tmp_indices.buffer,
        &data->tmp_indices.memory,
        indexBuffer.data()));
    }

    // Setup Vertex Descriptions
    {
      data->vertices.bindingDescriptions = Vertex::getBindingDescription();
      data->vertices.attributeDescriptions = Vertex::getAttributeDescription();
      data->vertices.inputState = vk::initializers::PipelineVertexInputStateCreateInfo();
      data->vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(Vertex::getBindingDescription().size());
      data->vertices.inputState.pVertexBindingDescriptions = data->vertices.bindingDescriptions.data();
      data->vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescription().size());
      data->vertices.inputState.pVertexAttributeDescriptions = data->vertices.attributeDescriptions.data();
    }

    // Prepare Offscreen Framebuffer (G-Buffer)
    {
      data->offScreenFrameBuf.width = 2048;
      data->offScreenFrameBuf.height = 2048;

      CreateFramebufferAttachment(data->device, data->physicalDevice, VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &data->offScreenFrameBuf.position, 
        data->offScreenFrameBuf.width, data->offScreenFrameBuf.height);

      CreateFramebufferAttachment(data->device, data->physicalDevice, VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &data->offScreenFrameBuf.normal, 
        data->offScreenFrameBuf.width, data->offScreenFrameBuf.height);

      CreateFramebufferAttachment(data->device, data->physicalDevice, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &data->offScreenFrameBuf.albedo, 
        data->offScreenFrameBuf.width, data->offScreenFrameBuf.height);

      CreateFramebufferAttachment(data->device, data->physicalDevice, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &data->offScreenFrameBuf.roughness,
        data->offScreenFrameBuf.width, data->offScreenFrameBuf.height);

      CreateFramebufferAttachment(data->device, data->physicalDevice, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &data->offScreenFrameBuf.metallic,
        data->offScreenFrameBuf.width, data->offScreenFrameBuf.height);

      VkFormat attDepthFormat;
      VkBool32 validDepthFormat = vk::tools::GetSupportedDepthFormat(data->physicalDevice, &attDepthFormat);
      assert(validDepthFormat);
      
      CreateFramebufferAttachment(data->device, data->physicalDevice, VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &data->offScreenFrameBuf.depth,
        data->offScreenFrameBuf.width, data->offScreenFrameBuf.height);

      std::vector<VkAttachmentDescription> attachmentDescs(6);
      for (u32 i = 0; i < 6; ++i) {

        attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        if (i == 5)
        {
          attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
          attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        else
        {
          attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
          attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
      }

      attachmentDescs[0].format = data->offScreenFrameBuf.position.format;
      attachmentDescs[1].format = data->offScreenFrameBuf.normal.format;
      attachmentDescs[2].format = data->offScreenFrameBuf.albedo.format;
      attachmentDescs[3].format = data->offScreenFrameBuf.roughness.format;
      attachmentDescs[4].format = data->offScreenFrameBuf.metallic.format;
      attachmentDescs[5].format = data->offScreenFrameBuf.depth.format;

      std::vector<VkAttachmentReference> colorReferences = {};
      colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
      colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
      colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
      colorReferences.push_back({ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
      colorReferences.push_back({ 4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

      std::vector<VkAttachmentReference> defDepthReference = {};
      defDepthReference.push_back({ 5, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

      VK_CHECK(CreateRenderPass(data->device, data->offScreenFrameBuf.renderPass,
        colorReferences, defDepthReference, attachmentDescs));

      std::vector<VkImageView> attachments(6);
      attachments[0] = data->offScreenFrameBuf.position.view;
      attachments[1] = data->offScreenFrameBuf.normal.view;
      attachments[2] = data->offScreenFrameBuf.albedo.view;
      attachments[3] = data->offScreenFrameBuf.roughness.view;
      attachments[4] = data->offScreenFrameBuf.metallic.view;
      attachments[5] = data->offScreenFrameBuf.depth.view;

      VK_CHECK(CreateFramebuffer(data->device, data->offScreenFrameBuf.frameBuffer,
        data->offScreenFrameBuf.renderPass, data->offScreenFrameBuf.width,
        data->offScreenFrameBuf.height, attachments));

      VK_CHECK(CreateSampler(data->device, data->colorSampler));
    }

    // Prepare UniformBuffers
    {
      VK_CHECK(data->vulkanState->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(data->uboVS), &data->uniformBuffers.vsFullScreen));

      VK_CHECK(data->vulkanState->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(data->uboOffscreenVS), &data->uniformBuffers.vsOffscreen));

      VK_CHECK(data->vulkanState->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(data->uboFragmentLights), &data->uniformBuffers.fsLights));

      VK_CHECK(data->vulkanState->createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(data->skyboxUboVS), &data->uniformBuffers.skybox));

      VK_CHECK(data->uniformBuffers.vsFullScreen.map());
      VK_CHECK(data->uniformBuffers.vsOffscreen.map());
      VK_CHECK(data->uniformBuffers.fsLights.map());
      VK_CHECK(data->uniformBuffers.skybox.map());

      data->uboOffscreenVS.instancePos[0] = glm::vec4(0.0f);
      data->uboOffscreenVS.instancePos[1] = glm::vec4(-4.0f, 0.0, -4.0f, 0.0f);
      data->uboOffscreenVS.instancePos[2] = glm::vec4(4.0f, 0.0, -4.0f, 0.0f);

      updateUniformBuffersScreen();
      updateUniformBufferDeferredMatrices();
      updateUniformBufferDeferredLights();
    }
    
    // Setup DescriptorSetLayout
    {
      std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
      {
        // Binding 0 : Vertex shader uniform buffer
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          VK_SHADER_STAGE_VERTEX_BIT,
          0),
        // Binding 1 : Position texture target / Scene colormap
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          1),
        // Binding 2 : Normals texture target
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          2),
        // Binding 3 : Albedo texture target
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          3),
        // Binding 4 : Roughness texture
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          4),
        // Binding 5 : Metalllic texture
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          5),
        // Binding 6 : Fragment shader uniform buffer
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          6),
      };

      VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vk::initializers::DescriptorSetLayoutCreateInfo(
          setLayoutBindings.data(),
          static_cast<uint32_t>(setLayoutBindings.size()));

      VK_CHECK(vkCreateDescriptorSetLayout(data->device, &descriptorLayout, nullptr, &data->descriptorSetLayout));

      VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vk::initializers::PipelineLayoutCreateInfo(&data->descriptorSetLayout, 1);

      VK_CHECK(vkCreatePipelineLayout(data->device, &pPipelineLayoutCreateInfo, nullptr, &data->pipelineLayouts.deferred));
      VK_CHECK(vkCreatePipelineLayout(data->device, &pPipelineLayoutCreateInfo, nullptr, &data->pipelineLayouts.offscreen));
    
      // skybox
      std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings2 = {

        // Binding 0 : Vertex shader uniform buffer
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          VK_SHADER_STAGE_VERTEX_BIT,
          0),
        // Binding 1 : Fragment shader image sampler
        vk::initializers::DescriptorSetLayoutBinding(
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          1)
      };

      descriptorLayout =
        vk::initializers::DescriptorSetLayoutCreateInfo(
          setLayoutBindings2.data(),
          (u32)setLayoutBindings2.size());

      VK_CHECK(vkCreateDescriptorSetLayout(data->device, &descriptorLayout, nullptr, &data->skyboxDescriptorSetLayout));

      pPipelineLayoutCreateInfo =
        vk::initializers::PipelineLayoutCreateInfo(
          &data->skyboxDescriptorSetLayout,
          1);

      VK_CHECK(vkCreatePipelineLayout(data->device, &pPipelineLayoutCreateInfo, nullptr, &data->pipelineLayouts.skybox));
    }

    // Prepare Pipelines
    {
      std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentState = {
        vk::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
      };

      VkPipelineDepthStencilStateCreateInfo depthStencilState =
        vk::initializers::PipelineDepthStencilStateCreateInfo(
          VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

      VkPipelineVertexInputStateCreateInfo emptyInputState = vk::initializers::PipelineVertexInputStateCreateInfo();

      VK_CHECK(CreateGraphicsPipeline(data->device, data->pipelineLayouts.deferred, data->renderPass,
        "deferred_bp", data->pipelineCache, data->pipelines.deferred, emptyInputState, blendAttachmentState, 
        depthStencilState, VK_FRONT_FACE_CLOCKWISE));
      
      VK_CHECK(CreateGraphicsPipeline(data->device, data->pipelineLayouts.deferred, data->renderPass,
        "debug", data->pipelineCache, data->pipelines.debug, data->vertices.inputState, blendAttachmentState, 
        depthStencilState, VK_FRONT_FACE_CLOCKWISE));

      std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = {
        vk::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
        vk::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
        vk::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
        vk::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
        vk::initializers::PipelineColorBlendAttachmentState(0xf, VK_FALSE)
      };

      VK_CHECK(CreateGraphicsPipeline(data->device, data->pipelineLayouts.offscreen, data->offScreenFrameBuf.renderPass,
        "mrt", data->pipelineCache, data->pipelines.offscreen, data->vertices.inputState, blendAttachmentStates, depthStencilState,
        VK_FRONT_FACE_CLOCKWISE));

      // skybox
      depthStencilState = vk::initializers::PipelineDepthStencilStateCreateInfo(
        VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

      VkVertexInputBindingDescription vertexInputBinding =
        vk::initializers::VertexInputBindingDescription(0, 32, VK_VERTEX_INPUT_RATE_VERTEX);

      std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
        vk::initializers::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					        // Location 0: Position
        vk::initializers::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),	// Location 1: Normal
      };

      VkPipelineVertexInputStateCreateInfo vertexInputState = vk::initializers::PipelineVertexInputStateCreateInfo();
      vertexInputState.vertexBindingDescriptionCount = 1;
      vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
      vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
      vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

      VK_CHECK(CreateGraphicsPipeline(data->device, data->pipelineLayouts.skybox, data->offScreenFrameBuf.renderPass,
        "skybox", data->pipelineCache, data->pipelines.skybox, vertexInputState, blendAttachmentStates, depthStencilState,
        VK_FRONT_FACE_COUNTER_CLOCKWISE));
    }

    // Setup DescriptorPool
    {
      std::vector<VkDescriptorPoolSize> poolSizes = {
        vk::initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8),
        vk::initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 9)
      };

      VK_CHECK(CreateDescriptorPool(data->device, data->descriptorPool, poolSizes));
    }

    // Setup DescriptorSet
    {
      VkDescriptorSetAllocateInfo allocInfo =
        vk::initializers::DescriptorSetAllocateInfo(
          data->descriptorPool, &data->descriptorSetLayout, 1);

      VkDescriptorImageInfo texDescriptorPosition =
        vk::initializers::DescriptorImageInfo(data->colorSampler,
          data->offScreenFrameBuf.position.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      VkDescriptorImageInfo texDescriptorNormal =
        vk::initializers::DescriptorImageInfo(data->colorSampler,
          data->offScreenFrameBuf.normal.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      VkDescriptorImageInfo texDescriptorAlbedo =
        vk::initializers::DescriptorImageInfo(data->colorSampler,
          data->offScreenFrameBuf.albedo.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      VkDescriptorImageInfo texDescriptorRoughness =
        vk::initializers::DescriptorImageInfo(data->colorSampler,
          data->offScreenFrameBuf.roughness.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      VkDescriptorImageInfo texDescriptorMetallic =
        vk::initializers::DescriptorImageInfo(data->colorSampler,
          data->offScreenFrameBuf.metallic.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      VK_CHECK(vkAllocateDescriptorSets(data->device, &allocInfo, &data->descriptorSet));

      std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0 : Vertex shader uniform buffer
        vk::initializers::WriteDescriptorSet(data->descriptorSet,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &data->uniformBuffers.vsFullScreen.descriptor),
        // Binding 1 : Position texture target
        vk::initializers::WriteDescriptorSet(data->descriptorSet,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptorPosition),
        // Binding 2 : Normals texture target
        vk::initializers::WriteDescriptorSet(data->descriptorSet,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptorNormal),
        // Binding 3 : Albedo texture target
        vk::initializers::WriteDescriptorSet(data->descriptorSet,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &texDescriptorAlbedo),
        // Binding 4 : Roughness texture target
        vk::initializers::WriteDescriptorSet(data->descriptorSet,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &texDescriptorRoughness),
        // Binding 5 : metallic texture target
        vk::initializers::WriteDescriptorSet(data->descriptorSet,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &texDescriptorMetallic),
        // Binding 6 : Fragment shader uniform buffer
        vk::initializers::WriteDescriptorSet(data->descriptorSet,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6, &data->uniformBuffers.fsLights.descriptor),
      };

      vkUpdateDescriptorSets(data->device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

      // Offscreen (scene)

      // Model
      VK_CHECK(vkAllocateDescriptorSets(data->device, &allocInfo, &data->descriptorSets.model));
      writeDescriptorSets =
      {
        // Binding 0: Vertex shader uniform buffer
        vk::initializers::WriteDescriptorSet(data->descriptorSets.model,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &data->uniformBuffers.vsOffscreen.descriptor),
        // Binding 1: Color map
        vk::initializers::WriteDescriptorSet(data->descriptorSets.model,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &data->textures.model.colorMap.descriptor),
        // Binding 2: Normal map
        vk::initializers::WriteDescriptorSet(data->descriptorSets.model,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &data->textures.model.normalMap.descriptor),

        vk::initializers::WriteDescriptorSet(data->descriptorSets.model,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &data->textures.model.roughness.descriptor),

        vk::initializers::WriteDescriptorSet(data->descriptorSets.model,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &data->textures.model.metallic.descriptor)
      };

      vkUpdateDescriptorSets(data->device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

      // Background
      VK_CHECK(vkAllocateDescriptorSets(data->device, &allocInfo, &data->descriptorSets.floor));
    
      writeDescriptorSets =
      {
        // Binding 0: Vertex shader uniform buffer
        vk::initializers::WriteDescriptorSet(data->descriptorSets.floor,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &data->uniformBuffers.vsOffscreen.descriptor),
        // Binding 1: Color map
        vk::initializers::WriteDescriptorSet(data->descriptorSets.floor,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &data->textures.floor.colorMap.descriptor),
        // Binding 2: Normal map
        vk::initializers::WriteDescriptorSet(data->descriptorSets.floor,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &data->textures.floor.normalMap.descriptor),

        vk::initializers::WriteDescriptorSet(data->descriptorSets.floor,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &data->textures.floor.roughness.descriptor),

        vk::initializers::WriteDescriptorSet(data->descriptorSets.floor,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &data->textures.floor.metallic.descriptor)
      };

      vkUpdateDescriptorSets(data->device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
    
      // Sky box descriptor set
      allocInfo = vk::initializers::DescriptorSetAllocateInfo(
        data->descriptorPool, &data->skyboxDescriptorSetLayout, 1);

      VkDescriptorImageInfo textureDescriptor =
        vk::initializers::DescriptorImageInfo(
          data->cubeMap.sampler,
          data->cubeMap.view,
          data->cubeMap.imageLayout);

      VK_CHECK(vkAllocateDescriptorSets(data->device, &allocInfo, &data->descriptorSets.skybox));

      writeDescriptorSets =
      {
        // Binding 0 : Vertex shader uniform buffer
        vk::initializers::WriteDescriptorSet(
          data->descriptorSets.skybox,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          0,
          &data->uniformBuffers.skybox.descriptor),
        // Binding 1 : Fragment shader cubemap sampler
        vk::initializers::WriteDescriptorSet(
          data->descriptorSets.skybox,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          1,
          &textureDescriptor)
      };
      vkUpdateDescriptorSets(data->device, (u32)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
    }

    // Creation of engine graphic resources ->

    createGeometryResource(kGeometryEnum_Load, Reignite::Tools::GetAssetPath() + "models/geosphere.obj");
    createGeometryResource(kGeometryEnum_Load, Reignite::Tools::GetAssetPath() + "models/box.obj");
    createGeometryResource(kGeometryEnum_Terrain);

    //createMaterialResource();
    //createMaterialResource();

    buildCommandBuffers();
    buildDeferredCommands();
  }

  void Reignite::RenderContext::shutdown() {

    VK_CHECK(vkDeviceWaitIdle(data->device)); // Wait for possible running events from the loop to finish

    vkDestroyCommandPool(data->device, data->commandPool, 0);

    //DestroyImage(data->device, data->colorImage);
    //DestroyImage(data->device, data->depthImage);
    //destroySwapchain(data->device, data->swapchain);

    for (size_t i = 0; i < data->materials.size(); ++i)
      DestroyMaterial(data->materials[i]);

    vkDestroyDescriptorPool(data->device, data->descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(data->device, data->descriptorSetLayout, nullptr);

    //vkDestroySampler(data->device, data->textureSampler, nullptr);

    //DestroyImage(data->device, data->texture);

    for (u32 i = 0; i < data->geometries.size(); ++i)
      DestroyGeometry(data->geometries[i]);

    //vkDestroyPipeline(data->device, data->trianglePipeline, 0);
    //vkDestroyPipelineLayout(data->device, data->triangleLayout, 0);

    //vkDestroyShaderModule(data->device, data->triangleVS, 0);
    //vkDestroyShaderModule(data->device, data->triangleFS, 0);
    vkDestroyRenderPass(data->device, data->renderPass, 0);

    //vkDestroySemaphore(data->device, data->acquireSemaphore, 0);
    //vkDestroySemaphore(data->device, data->releaseSemaphore, 0);

    //vkDestroySurfaceKHR(data->instance, data->surface, 0);

    // TODO: Window may need to be destroyed here

    vkDestroyDevice(data->device, 0);

    // Physical device memory is managed by the instance. Don't need to be destroyed manually.

#ifdef _DEBUG
    vkDestroyDebugReportCallbackEXT(data->instance, data->debugCallback, 0);
#endif

    vkDestroyInstance(data->instance, 0);

    data->render_should_close = true;
  }

}