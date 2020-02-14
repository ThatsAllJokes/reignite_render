#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTexcoord;

layout(binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 proj;
  mat4 view;
  vec3 camPos;
} ubo;

layout (location = 0) out vec3 outWorlPos;
layout (location = 1) out vec3 outNormal;

layout (push_constant) uniform PushConts {
  vec3 objPos;
} pushConsts;

out gl_PerVertex {

  vec4 gl_Position;
};

void main() {

  vec3 locPos = vec3(ubo.model * vec4(inPosition, 1.0));
  outWorlPos = locPos + pushConsts.objPos;
  outNormal = mat3(ubo.model) * inNormal;
  gl_Position = ubo.proj * ubo.view * vec4(outWorlPos, 1.0);
}