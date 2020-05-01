#ifndef _RI_GEOMETRY_RESOURCE_
#define _RI_GEOMETRY_RESOURCE_ 1

#include <vector>
#include <string>

#include "../basic_types.h"

#include "../Vulkan/vulkan_impl.h"
#include "../Vulkan/vulkan_state.h"
#include "../Vulkan/vulkan_buffer.h"


namespace Reignite {

  struct GeometryResource {

    void init();
    void destroy();

    bool loadObj(std::string file);
    bool loadTerrain(u32 width, u32 lenght);

    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    u32 vertexSize;
    u32 indicesSize;

    vk::VulkanState* state;
    vk::Buffer vertexBuffer;
    vk::Buffer indexBuffer;
  };
}

#endif // _CAMERA_COMPONENT_