#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec3 inTangent;

layout (binding = 0, set = 1) uniform UBOView {
	mat4 projection;
	mat4 view;
} view;

layout (binding = 0, set = 1) uniform Models {
  mat4 matrix;
} model;

layout (location = 0) out vec3 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	outUV = vec3(inUV.st, inNormal.z);
	gl_Position = view.projection * view.view * vec4(inPos.xyz, 1.0);
}
