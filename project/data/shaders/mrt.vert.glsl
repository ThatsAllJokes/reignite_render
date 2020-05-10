#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;
layout (location = 4) in vec3 inTangent;

layout (binding = 0, set = 1) uniform UBOView {
	mat4 projection;
	mat4 view;
} view;

layout (binding = 0, set = 2) uniform Models {
  mat4 matrix;
} model;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec3 outWorldPos;
layout (location = 4) out vec3 outTangent;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {

	vec4 tmpPos = inPos; // + view.instancePos[gl_InstanceIndex];

	gl_Position = view.projection * view.view * model.matrix * tmpPos;
	
	outUV = inUV;
	outUV.t = 1.0 - outUV.t;

	// Vertex position in world space
	outWorldPos = vec3(model.matrix * tmpPos);
	// GL to Vulkan coord space
	outWorldPos.y = -outWorldPos.y;
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(model.matrix)));
	outNormal = mNormal * normalize(inNormal);	
	outTangent = mNormal * normalize(inTangent);
	
	// Currently just vertex color
	outColor = inColor;
}
