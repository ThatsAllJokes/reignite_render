#include "tools.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "log.h"


#define EXE_PATH 0

const std::string Reignite::Tools::GetAssetPath() {
#if EXE_PATH
  return "./../../../../project/";
#else
  return "./../";
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

      vertex.texcoord[0] = attrib.texcoords[2 * index.texcoord_index + 0];
      vertex.texcoord[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];

      geometry.vertices.push_back(vertex);
      geometry.indices.push_back(geometry.indices.size());
    }
  }

  return true;
}