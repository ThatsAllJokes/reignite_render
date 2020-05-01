#include "geometry_resource.h"

#include "../tools.h"


void Reignite::GeometryResource::init() {

  vertexSize = 0;
  indicesSize = 0;

  state = nullptr;
  vertexBuffer = {};
  indexBuffer = {};
}

void Reignite::GeometryResource::destroy() {

  vertices.clear();
  indices.clear();

  vertexBuffer.destroy();
  indexBuffer.destroy();
}

bool Reignite::GeometryResource::loadObj(std::string file) {

  GeometryResource auxGeometry = {};
  bool result = Reignite::Tools::LoadObjFile(file, auxGeometry);
  *this = auxGeometry;

  return result;
}

bool Reignite::GeometryResource::loadTerrain(u32 width, u32 length) {

  GeometryResource auxGeometry = {};
  bool result = Reignite::Tools::GenerateTerrain(auxGeometry, width, length);
  *this = auxGeometry;

  return result;
}