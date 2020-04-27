#include "geometry_resource.h"

#include "../tools.h"


void CreateGeometry(Geometry& geometry) {
  geometry.device = 0;
  geometry.vertexBuffer = { 0, 0 };
  geometry.indexBuffer = { 0, 0 };
}

void DestroyGeometry(Geometry& geometry) {

  geometry.vertices.clear();
  geometry.indices.clear();
  destroyBuffer(geometry.device, geometry.vertexBuffer);
  destroyBuffer(geometry.device, geometry.indexBuffer);
}

Geometry GeometryResourceLoadObj(std::string file) {

  Geometry obj_geometry = {};
  Reignite::Tools::LoadObjFile(file, obj_geometry);
 
  return obj_geometry;
}

Geometry GeometryResourceCube() {

  return GeometryResourceTextureCube();
}

Geometry GeometryResourceSquare() {

  const std::vector<Vertex> square_vertices = {
    {{-0.5f,  0.0f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.5f,  0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.0f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f,  0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
  };

  const std::vector<u32> square_indices = { 0, 1, 2, 2, 3, 0 };

  Geometry square = {};
  square.vertices = square_vertices;
  square.indices = square_indices;

  return square;
}

Geometry GeometryResourceTextureCube() {

  const std::vector<Vertex> cube_vertices = {
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

  const std::vector<u32> cube_indices{
    0, 1, 2, 2, 3, 0, // Top
    4, 6, 5, 6, 4, 7, // Bottom
    8, 9, 10, 10, 11, 8, // Front
    12, 13, 14, 14, 15, 12, // Back
    16, 18, 17, 18, 16, 19, // Left
    20, 21, 22, 22, 23, 20, // Right
  };

  Geometry cube = {};
  cube.vertices = cube_vertices;
  cube.indices = cube_indices;

  return cube;
}

Geometry GeometryTerrain() {

  Geometry obj_geometry = {};
  Reignite::Tools::GenerateTerrain(obj_geometry, 12, 12);

  return obj_geometry;
}