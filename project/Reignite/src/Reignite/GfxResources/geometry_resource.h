#ifndef _RI_GEOMETRY_RESOURCE_
#define _RI_GEOMETRY_RESOURCE_ 1

#include <vector>

#include "../basic_types.h"

#include "../Vulkan/vulkan_impl.h"


namespace Reignite {

  struct GeometryResource {
    std::vector<Vertex> vertices;
    std::vector<u16> indices;
  };
}

typedef Reignite::GeometryResource Geometry;

void CreateGeometry(Geometry& geometry);

void DestroyGeometry(Geometry& geometry);

Geometry GeometryResourceCube();

Geometry GeometryResourceSquare();

Geometry GeometryResourceTextureCube();

#endif // _CAMERA_COMPONENT_