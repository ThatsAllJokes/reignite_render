#ifndef _RI_TOOLS_
#define _RI_TOOLS_ 1

#include <string>

#include "core.h"

#include "GfxResources/geometry_resource.h"


namespace Reignite {
namespace Tools {

  bool LoadGltfFile(std::string filename, GeometryResource& geometry);

  bool LoadObjFile(std::string filename, GeometryResource& geometry);

}} // end of Reignite::Tools namespace

#endif // _RI_TOOLS_
