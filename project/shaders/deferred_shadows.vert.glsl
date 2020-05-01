#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inNormal;
layout (location = 2) in vec2 inUv;
layout (location = 3) in vec4 inColor;
layout (location = 4) in vec4 inTangent;

layout (location = 0) out int outInstanceIndex;

void main() {

	outInstanceIndex = gl_InstanceIndex;
	gl_Position = inPos;
}