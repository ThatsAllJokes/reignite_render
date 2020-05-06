#include "tools.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "log.h"

#include "Vulkan/vulkan_impl.h"


#define EXE_PATH 0

const std::string Reignite::Tools::GetAssetPath() {
#if EXE_PATH
  return "./../../../../project/data/";
#else
  return "./../data/";
#endif
}

bool Reignite::Tools::LoadTextureFile(std::string filename, s32& width, s32& height, void** data) {

  s32 texChannels;
  *data = stbi_load(filename.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
  assert(*data);

  return true;
}

void Reignite::Tools::FreeTextureData(void* textureData) {

  if (nullptr == textureData) {
    RI_ERROR("Pointer does not hold any texture information");
    return;
  }

  stbi_image_free(textureData);
}

bool Reignite::Tools::LoadGltfFile(std::string filename, GeometryResource& geometry) {

  bool store_original_json_for_extras_and_extensions = false;

  tinygltf::Model model;
  tinygltf::TinyGLTF gltf_ctx;
  std::string err;
  std::string warn;

  std::string ext = "gltf";

  gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(store_original_json_for_extras_and_extensions);

  bool ret = false;
  if (ext.compare("glb") == 0) {
    ret = gltf_ctx.LoadBinaryFromFile(&model, &err, &warn, filename.c_str());
  }
  else {
    ret = gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, filename.c_str());
  }

  if (!warn.empty()) { printf("Warn: %s\n", warn.c_str()); }

  if (!err.empty()) { printf("Err: %s\n", err.c_str()); }

  if (!ret) {
    printf("Failed to parse glTF\n");
    return false;
  }

  // gltf parsing

  return true;
}

bool Reignite::Tools::LoadObjFile(std::string filename, GeometryResource& geometry) {

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());

  if (!warn.empty())
    RI_WARN("WARNING: {0}\n", warn.c_str());

  if (!err.empty()) {
    RI_ERROR("ERROR: {0}\n", err.c_str());
    return false;
  }

  bool containsUV = false;

  // obj parsing TODO: Implement vertex deduplication
  for(const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {

      Vertex vertex = {};
      vec3f current_vertex = vec3f(attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2]);
      vec3f normalized_vertex = glm::normalize(current_vertex);

      vertex.position[0] = normalized_vertex.x * 0.5f;
      vertex.position[1] = normalized_vertex.y * 0.5f;
      vertex.position[2] = normalized_vertex.z * 0.5f;

      vertex.normal[0] = attrib.normals[3 * index.normal_index + 0];
      vertex.normal[1] = attrib.normals[3 * index.normal_index + 1];
      vertex.normal[2] = attrib.normals[3 * index.normal_index + 2];

      if (attrib.texcoords.size() != 0) {
        containsUV = true;
        vertex.texcoord[0] = attrib.texcoords[2 * index.texcoord_index + 0];
        vertex.texcoord[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
      }

      geometry.vertices.push_back(vertex);
      geometry.indices.push_back((u32)geometry.indices.size());
    }
  }

  geometry.vertexSize = (u32)geometry.vertices.size();
  geometry.indicesSize = (u32)geometry.indices.size();

  if (!containsUV)
    return true;

  // calculate tangents from obj received data
  for (u32 i = 0; i < geometry.indices.size(); i += 3) {

    u32 i0 = geometry.indices[i];
    u32 i1 = geometry.indices[i + 1];
    u32 i2 = geometry.indices[i + 2];

    vec3f v0(
      geometry.vertices[i0].position[0],
      geometry.vertices[i0].position[1],
      geometry.vertices[i0].position[2]);

    vec3f v1(
      geometry.vertices[i1].position[0],
      geometry.vertices[i1].position[1],
      geometry.vertices[i1].position[2]);

    vec3f v2(
      geometry.vertices[i2].position[0],
      geometry.vertices[i2].position[1],
      geometry.vertices[i2].position[2]);

    vec2f uv0(geometry.vertices[i0].texcoord[0], geometry.vertices[i0].texcoord[1]);
    vec2f uv1(geometry.vertices[i1].texcoord[0], geometry.vertices[i1].texcoord[1]);
    vec2f uv2(geometry.vertices[i2].texcoord[0], geometry.vertices[i2].texcoord[1]);

    vec3f deltaPos1 = v1 - v0;
    vec3f deltaPos2 = v2 - v0;

    vec2f deltaUV1 = uv1 - uv0;
    vec2f deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    if (std::isnan(r) || std::isinf(r))
      r = 0.0f;

    vec3f tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    //vec3f bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

    geometry.vertices[i0].tangent[0] = tangent.x;
    geometry.vertices[i0].tangent[1] = tangent.y;
    geometry.vertices[i0].tangent[2] = tangent.z;

    geometry.vertices[i1].tangent[0] = tangent.x;
    geometry.vertices[i1].tangent[1] = tangent.y;
    geometry.vertices[i1].tangent[2] = tangent.z;

    geometry.vertices[i2].tangent[0] = tangent.x;
    geometry.vertices[i2].tangent[1] = tangent.y;
    geometry.vertices[i2].tangent[2] = tangent.z;
  }

  return true;
}

bool Reignite::Tools::GenerateTerrain(GeometryResource& geometry, 
  u32 width, u32 length, void(*HeigthFunction)()) {

  float auxSize = 1.0f;
  std::vector<Vertex> currentVertices(width * length);

  for (u32 j = 0; j < length; ++j) {
    for (u32 i = 0; i < width; ++i) {

      u32 vertex_index = j * width + i;

      currentVertices[vertex_index].position[0] = (width / 2) - (i * auxSize);
      currentVertices[vertex_index].position[1] = 0.0f;
      currentVertices[vertex_index].position[2] = (width / 2) - (j * auxSize);

      currentVertices[vertex_index].normal[0] = 0.0f;
      currentVertices[vertex_index].normal[1] = 1.0f;
      currentVertices[vertex_index].normal[2] = 0.0f;

      currentVertices[vertex_index].texcoord[0] = (float)j;
      currentVertices[vertex_index].texcoord[1] = (float)i;
    
      currentVertices[vertex_index].color[0] = 0.0f;
      currentVertices[vertex_index].color[1] = 0.0f;
      currentVertices[vertex_index].color[2] = 0.0f;
    }
  }

  geometry.vertices = currentVertices;
  geometry.vertexSize = (u32)geometry.vertices.size();

  geometry.indices.resize(6 * width * length);
  for (u32 j = 0; j < length - 1; ++j) {
    for (u32 i = 0; i < width - 1; ++i) {

      geometry.indices[(i * 6 * length) + (j * 6) + 0] = (j * width + i) + width;
      geometry.indices[(i * 6 * length) + (j * 6) + 1] = (j * width + i) + 0;
      geometry.indices[(i * 6 * length) + (j * 6) + 2] = (j * width + i) + 1;

      geometry.indices[(i * 6 * length) + (j * 6) + 3] = (j * width + i) + 1;
      geometry.indices[(i * 6 * length) + (j * 6) + 4] = (j * width + i) + width + 1;
      geometry.indices[(i * 6 * length) + (j * 6) + 5] = (j * width + i) + width;
    }
  }

  geometry.indicesSize = (u32)geometry.indices.size();

  // Tangent calculation
  for (u32 i = 0; i < geometry.indices.size(); i += 3) {

    u32 i0 = geometry.indices[i];
    u32 i1 = geometry.indices[i + 1];
    u32 i2 = geometry.indices[i + 2];

    vec3f v0(
      geometry.vertices[i0].position[0], 
      geometry.vertices[i0].position[1], 
      geometry.vertices[i0].position[2]);
    
    vec3f v1(
      geometry.vertices[i1].position[0], 
      geometry.vertices[i1].position[1], 
      geometry.vertices[i1].position[2]);
    
    vec3f v2(
      geometry.vertices[i2].position[0], 
      geometry.vertices[i2].position[1], 
      geometry.vertices[i2].position[2]);

    vec2f uv0(geometry.vertices[i0].texcoord[0], geometry.vertices[i0].texcoord[1]);
    vec2f uv1(geometry.vertices[i1].texcoord[0], geometry.vertices[i1].texcoord[1]);
    vec2f uv2(geometry.vertices[i2].texcoord[0], geometry.vertices[i2].texcoord[1]);

    vec3f deltaPos1 = v1 - v0;
    vec3f deltaPos2 = v2 - v0;

    vec2f deltaUV1 = uv1 - uv0;
    vec2f deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    if (std::isnan(r) || std::isinf(r)) 
      r = 0.0f;

    vec3f tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    //vec3f bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

    geometry.vertices[i0].tangent[0] = tangent.x;
    geometry.vertices[i0].tangent[1] = tangent.y;
    geometry.vertices[i0].tangent[2] = tangent.z;

    geometry.vertices[i1].tangent[0] = tangent.x;
    geometry.vertices[i1].tangent[1] = tangent.y;
    geometry.vertices[i1].tangent[2] = tangent.z;

    geometry.vertices[i2].tangent[0] = tangent.x;
    geometry.vertices[i2].tangent[1] = tangent.y;
    geometry.vertices[i2].tangent[2] = tangent.z;
  }

  return true;
}