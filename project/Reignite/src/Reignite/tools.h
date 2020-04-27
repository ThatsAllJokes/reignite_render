#ifndef _RI_TOOLS_
#define _RI_TOOLS_ 1

#include <string>

#include "core.h"

#include "GfxResources/geometry_resource.h"


namespace Reignite {
namespace Tools {

  const std::string GetAssetPath();

  bool LoadTextureFile(std::string filename, s32& width, s32& height, void** data);

  void FreeTextureData(void* textureData);

  bool LoadGltfFile(std::string filename, GeometryResource& geometry);

  bool LoadObjFile(std::string filename, GeometryResource& geometry);

  bool GenerateTerrain(GeometryResource& geometry, u32 width, u32 lenght, void(*HeigthFunction)() = nullptr);

}} // end of Reignite::Tools namespace

#endif // _RI_TOOLS_
