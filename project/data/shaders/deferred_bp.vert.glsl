#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0, set = 0) uniform Matrices {
  mat4 projection;
  mat4 view;
} matrices;

layout (binding = 0, set = 1) uniform Model {
  mat4 matrix;
} model;

layout (location = 0) out vec2 outUV;

vec2 uv[6] = vec2[](
  vec2(0.0, 0.0),
  vec2(1.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 1.0),
  vec2(0.0, 1.0),
  vec2(0.0, 0.0)
);

vec2 positions[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(1.0, 1.0),
  vec2(1.0, 1.0),
  vec2(-1.0, 1.0),
  vec2(-1.0, -1.0)
);

out gl_PerVertex {

	vec4 gl_Position;
};

void main() {

	outUV = vec2(uv[gl_VertexIndex]);
	gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
}
