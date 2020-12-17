#version 450

#define LIGHT_COUNT 3

layout (triangles, invocations = LIGHT_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0, set = 0) uniform UBO0 {
  mat4 model;
} ubo0;

layout (binding = 0, set = 1) uniform UBO {
	mat4 mvp[LIGHT_COUNT];
	vec4 instancePos[3];
} ubo;

layout (location = 0) in int inInstanceIndex[];

void main() {

	vec4 instancedPos = ubo0.model[3]; //ubo.instancePos[inInstanceIndex[0]]; 
	for (int i = 0; i < gl_in.length(); i++) {

		gl_Layer = gl_InvocationID;
		vec4 tmpPos = gl_in[i].gl_Position + instancedPos;
		gl_Position = ubo.mvp[gl_InvocationID] * tmpPos;
		EmitVertex();
	}

	EndPrimitive();
}
