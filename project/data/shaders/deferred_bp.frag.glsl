#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0, set = 0) uniform sampler2D samplerposition;
layout (binding = 1, set = 0) uniform sampler2D samplerNormal;
layout (binding = 2, set = 0) uniform sampler2D samplerAlbedo;
layout (binding = 3, set = 0) uniform sampler2D samplerRoughness;
layout (binding = 4, set = 0) uniform sampler2D samplerMetallic;

layout (binding = 6) uniform sampler2DArray samplerShadowMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

#define LIGHT_COUNT 3
#define SHADOW_FACTOR 0.25
#define AMBIENT_LIGHT 0.1
#define USE_PCF

struct Light {
	vec4 position;
  vec4 target;
	vec4 color;
	float radius;
  mat4 view;
};

layout (binding = 5) uniform UBO {
	vec4 viewPos;
	Light lights[LIGHT_COUNT];
  int numbLights;
  int useShadows;
} ubo;

struct PushcConstants {
  float r;
  float g;
  float b;
  float roughness;
  float metallic;
} material;

const float PI = 3.14159265359;

float textureProj(vec4 P, float layer, vec2 offset)
{
	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	shadowCoord.st = shadowCoord.st * 0.5 + 0.5;
	
	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
	{
		float dist = texture(samplerShadowMap, vec3(shadowCoord.st + offset, layer)).r;
		if (shadowCoord.w > 0.0 && dist < shadowCoord.z) 
		{
			shadow = SHADOW_FACTOR;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc, float layer)
{
	ivec2 texDim = textureSize(samplerShadowMap, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, layer, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}


void main() 
{
	// Get G-Buffer values
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedo = texture(samplerAlbedo, inUV);
	
  float shadow = 0.0;
	vec3 fragColor = albedo.rgb * AMBIENT_LIGHT;

	vec3 N = normalize(normal);

	for(int i = 0; i < LIGHT_COUNT; ++i) {
    
		vec3 L = ubo.lights[i].position.xyz - fragPos; // Vector to light

		float dist = length(L); // Distance from light to fragment position
    L = normalize(L);

		vec3 V = normalize(ubo.viewPos.xyz - fragPos); // Viewer to fragment
		
    float lightCosInnerAngle = cos(radians(15.0));
		float lightCosOuterAngle = cos(radians(25.0));
		float lightRange = 100.0;

		vec3 dir = normalize(ubo.lights[i].position.xyz - ubo.lights[i].target.xyz);

		// Attenuation
		float cosDir = dot(L, dir);
		float spotEffect = smoothstep(lightCosOuterAngle, lightCosInnerAngle, cosDir);
		float heightAttenuation = smoothstep(lightRange, 0.0f, dist);

		// Diffuse part
		float NdotL = max(0.0, dot(N, L));
		vec3 diff = vec3(NdotL);

		// Specular part
		vec3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(R, V));
		vec3 spec = vec3(pow(NdotR, 16.0) * albedo.a * 2.5);

		fragColor += vec3((diff + spec) * spotEffect * heightAttenuation) * ubo.lights[i].color.rgb * albedo.rgb;
	}

  if (ubo.useShadows > 0) {

		for(int i = 0; i < LIGHT_COUNT; ++i) {

			vec4 shadowClip	= ubo.lights[i].view * vec4(fragPos, 1.0);

			float shadowFactor;
			#ifdef USE_PCF
				shadowFactor= filterPCF(shadowClip, i);
			#else
				shadowFactor = textureProj(shadowClip, i, vec2(0.0));
			#endif

			fragColor *= shadowFactor;
		}
	}
   
  outFragcolor.rgb = fragColor;
}