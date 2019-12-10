#include "window.h"

#include "log.h"
#include "vulkan_impl.h"


namespace Reignite {

  struct State {

    std::string title;
    u16 width;
    u16 height;

    std::vector<u32> indices;

    std::vector<TransformComponent> transforms;
    std::vector<GeometryComponent> geometries;
    std::vector<MaterialComponent> materials;
    std::vector<RenderComponent> renders;
    CameraComponent camera;

    std::vector<Geometry> db_geometries;
    std::vector<Material> db_materials;
    std::vector<Texture> db_textures;

    State(const std::string& t = "Reignite Render",
      u16 w = 1280, u16 h = 720) : title(t), width(w), height(h) {}
  };

  static bool s_glfw_initialized = false;

  Window* Window::Create(const State* state) {
    
    return new Window(state);
  }

  Window::Window(const State* state) {

    init(state);
  }

  Window::~Window() {
  
    shutdown();
  }

  void Window::init(const State* p) {

    state = (State*)p; // TODO: Resolve this on the future

    RI_INFO("Creating window . . . {0}: ({1}, {2})", p->title, p->width, p->height);

    /*if(!s_glfw_initialized) {
    
      s16 success = glfwInit();
      assert(success);

      s_glfw_initialized = true;
    }

    window = glfwCreateWindow((s16)p.width, (s16)p.height, p.title.c_str(), nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &params);*/
  }

  void Window::shutdown() {

    //glfwDestroyWindow(window);
  }

  void Window::update() {

    //glfwPollEvents();
    //glfwSwapBuffers(window);

    int rc = glfwInit();
    assert(rc);

    VK_CHECK(volkInitialize());

    VkInstance instance = createInstance();
    assert(instance);

    volkLoadInstance(instance);

    VkDebugReportCallbackEXT debugCallback = registerDebugCallback(instance);

    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

    VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
    assert(physicalDevice);

    uint32_t familyIndex = getGraphicsFamilyIndex(physicalDevice);
    assert(familyIndex != VK_QUEUE_FAMILY_IGNORED);

    VkDevice device = createDevice(instance, physicalDevice, familyIndex);
    assert(device);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "vulkan_render", 0, 0);
    assert(window);

    VkSurfaceKHR surface = createSurface(instance, window);
    assert(surface);

    VkBool32 presentSupported = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &presentSupported));
    assert(presentSupported);

    VkFormat swapchainFormat = getSwapchainFormat(physicalDevice, surface);

    VkSemaphore acquireSemaphore = createSemaphore(device);
    assert(acquireSemaphore);

    VkSemaphore releaseSemaphore = createSemaphore(device);
    assert(releaseSemaphore);

    VkQueue queue = 0;
    vkGetDeviceQueue(device, familyIndex, 0, &queue);

    VkRenderPass renderPass = createRenderPass(device, swapchainFormat);
    assert(renderPass);

    VkShaderModule triangleVS = loadShader(device, "../shaders/triangle.vert.spv");
    assert(triangleVS);

    VkShaderModule triangleFS = loadShader(device, "../shaders/triangle.frag.spv");
    assert(triangleFS);

    // TODO: This is critical for performance!
    VkPipelineCache pipelineCache = 0;

    VkPipelineLayout triangleLayout = createPipelineLayout(device);
    assert(triangleLayout);

    VkPipeline trianglePipeline = createGraphicsPipeline(device, pipelineCache, renderPass, triangleVS, triangleFS, triangleLayout);
    assert(trianglePipeline);

    Swapchain swapchain;
    createSwapchain(swapchain, physicalDevice, device, surface, familyIndex, swapchainFormat, renderPass);

    VkCommandPool commandPool = createCommandPool(device, familyIndex);
    assert(commandPool);

    VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocateInfo.commandPool = commandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = 0;
    vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

    while (!glfwWindowShouldClose(window)) {

      glfwPollEvents();

      resizeSwapchainIfNecessary(swapchain, physicalDevice, device, surface, familyIndex, swapchainFormat, renderPass);

      uint32_t imageIndex = 0;
      VK_CHECK(vkAcquireNextImageKHR(device, swapchain.swapchain, ~0ull, acquireSemaphore, VK_NULL_HANDLE, &imageIndex));

      VK_CHECK(vkResetCommandPool(device, commandPool, 0));

      VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

      VkImageMemoryBarrier renderBeginBarrier = imageBarrier(swapchain.images[imageIndex], 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderBeginBarrier);

      VkClearColorValue color = { 48.f / 255.f, 10.f / 255.f, 36.f / 255.f, 1 };
      VkClearValue clearColor = { color };

      VkRenderPassBeginInfo passbeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
      passbeginInfo.renderPass = renderPass;
      passbeginInfo.framebuffer = swapchain.framebuffers[imageIndex];
      passbeginInfo.renderArea.extent.width = swapchain.width;
      passbeginInfo.renderArea.extent.height = swapchain.height;
      passbeginInfo.clearValueCount = 1;
      passbeginInfo.pClearValues = &clearColor;

      vkCmdBeginRenderPass(commandBuffer, &passbeginInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkViewport viewport = { 0, float(swapchain.height), float(swapchain.width), -float(swapchain.height), 0, 1 };
      VkRect2D scissor = { { 0, 0 }, { swapchain.width, swapchain.height } };

      vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
      vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
      vkCmdDraw(commandBuffer, 3, 1, 0, 0);

      vkCmdEndRenderPass(commandBuffer);

      VkImageMemoryBarrier renderEndBarrier = imageBarrier(swapchain.images[imageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &renderEndBarrier);

      VK_CHECK(vkEndCommandBuffer(commandBuffer));

      VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

      VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = &acquireSemaphore;
      submitInfo.pWaitDstStageMask = &submitStageMask;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffer;
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = &releaseSemaphore;

      VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

      VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores = &releaseSemaphore;
      presentInfo.swapchainCount = 1;
      presentInfo.pSwapchains = &swapchain.swapchain;
      presentInfo.pImageIndices = &imageIndex;

      VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));

      VK_CHECK(vkDeviceWaitIdle(device));
    }

    VK_CHECK(vkDeviceWaitIdle(device)); // Wait for possible running events from the loop to finish

    vkDestroyCommandPool(device, commandPool, 0);

    destroySwapchain(device, swapchain);

    vkDestroyPipeline(device, trianglePipeline, 0);
    vkDestroyPipelineLayout(device, triangleLayout, 0);

    vkDestroyShaderModule(device, triangleVS, 0);
    vkDestroyShaderModule(device, triangleFS, 0);
    vkDestroyRenderPass(device, renderPass, 0);

    vkDestroySemaphore(device, acquireSemaphore, 0);
    vkDestroySemaphore(device, releaseSemaphore, 0);

    vkDestroySurfaceKHR(instance, surface, 0);

    glfwDestroyWindow(window);

    vkDestroyDevice(device, 0);

    // Physical device memory is managed by the instance. Don't need to be destroyed manually.

#ifdef _DEBUG
    vkDestroyDebugReportCallbackEXT(instance, debugCallback, 0);
#endif

    vkDestroyInstance(instance, 0);
  }

  inline const u16 Window::getWidth() { return state->width; }

  inline const u16 Window::getHeight() { return state->height; }
}