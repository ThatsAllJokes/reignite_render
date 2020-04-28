#version 450

layout (binding = 1) uniform sampler2D samplerposition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;
layout (binding = 4) uniform sampler2D samplerRoughness;
layout (binding = 5) uniform sampler2D samplerMetallic;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout (binding = 6) uniform UBO {
	Light lights[6];
	vec4 viewPos;
} ubo;

struct PushcConstants {
  float r;
  float g;
  float b;
  float roughness;
  float metallic;
} material;

const float PI = 3.14159265359;

void main() 
{
	// Get G-Buffer values
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedo = texture(samplerAlbedo, inUV);
	
	#define lightCount 6
	#define ambient 0.6
	
	vec3 fragcolor  = albedo.rgb * ambient; // Ambient part
	for(int i = 0; i < lightCount; ++i) {
    
		vec3 L = ubo.lights[i].position.xyz - fragPos; // Vector to light
		float dist = length(L); // Distance from light to fragment position

		vec3 V = normalize(ubo.viewPos.xyz - fragPos); // Viewer to fragment
		
		if(dist < ubo.lights[i].radius) {
			
			L = normalize(L); // Light to fragment

			// Attenuation
			float atten = ubo.lights[i].radius / (pow(dist, 2.0) + 1.0);

			// Diffuse part
			vec3 N = normalize(normal);
			float NdotL = max(0.0, dot(N, L));
			vec3 diff = ubo.lights[i].color * albedo.rgb * NdotL * atten;

			// Specular part
			// Specular map values are stored in alpha of albedo mrt
			vec3 R = reflect(-L, N);
			float NdotR = max(0.0, dot(R, V));
			vec3 spec = ubo.lights[i].color * albedo.a * pow(NdotR, 16.0) * atten;

			fragcolor += diff + spec;	
		}	
	}    	
   
  outFragcolor = vec4(fragcolor, 1.0);	
}