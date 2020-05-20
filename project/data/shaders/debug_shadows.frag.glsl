#version 450

layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;
layout (binding = 3) uniform sampler2D samplerRoughness;
layout (binding = 4) uniform sampler2D samplerMetallic;
layout (binding = 6) uniform sampler2DArray samplerDepth;

layout (location = 0) in vec3 inUV;

layout (location = 0) out vec4 outFragColor;


float LinearizeDepth(float depth) {

  float near_z = 0.1;
  float far_z = 64.0;
  float z = depth;

  float num = (2.0 * near_z);
  float denom = (far_z + near_z - z * (far_z - near_z));

  return num / denom;
}

void main() {

  float depth = texture(samplerDepth, vec3(inUV)).r;
	outFragColor = vec4(vec3(1.0 - LinearizeDepth(depth)), 0.0);
}