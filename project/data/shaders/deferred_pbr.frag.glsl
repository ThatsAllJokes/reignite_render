#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0, set = 0) uniform sampler2D samplerposition;
layout (binding = 1, set = 0) uniform sampler2D samplerNormal;
layout (binding = 2, set = 0) uniform sampler2D samplerAlbedo;
layout (binding = 3, set = 0) uniform sampler2D samplerRoughness;
layout (binding = 4, set = 0) uniform sampler2D samplerMetallic;

struct Light {
	vec4 position;
  vec4 target;
	vec4 color;
	//float radius;
  mat4 view;
};

layout (binding = 5, set = 0) uniform UBO {
	vec4 viewPos;
	Light lights[3];
  //uint numbLights;
  uint useShadows;
} ubo;

layout (binding = 6, set = 0) uniform sampler2DArray samplerShadowMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

#define LIGHT_COUNT 3
#define SHADOW_FACTOR 0.25
#define AMBIENT_LIGHT 0.6
#define USE_PCF

struct PushcConstants {
  float r;
  float g;
  float b;
  float roughness;
  float metallic;
} material;

const float PI = 3.14159265359;


float textureProj(vec4 P, float layer, vec2 offset) {

	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	shadowCoord.st = shadowCoord.st * 0.5 + 0.5;
	
	if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) {

		float dist = texture(samplerShadowMap, vec3(shadowCoord.st + offset, layer)).r;
		if (shadowCoord.w > 0.0 && dist < shadowCoord.z) {

			shadow = SHADOW_FACTOR;
		}
	}

	return shadow;
}

float filterPCF(vec4 sc, float layer) {

	ivec2 texDim = textureSize(samplerShadowMap, 0).xy;
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++) {

		for (int y = -range; y <= range; y++)	{

			shadowFactor += textureProj(sc, layer, vec2(dx*x, dy*y));
			count++;
		}
	}

	return shadowFactor / count;
}


float DistributionGGX(vec3 N, vec3 H, float roughness) { // Normal distribution

  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;

  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float num = alpha2;
  float denom = (NdotH2 * (alpha2  - 1.0) + 1.0);
  denom = PI * denom * denom;

  return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {

  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float num = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) { // Geometric Shadowing 

  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);

  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) { // Fresnel

  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


void main() {

  // init temp test values
  material.r = 1.0;
  material.g = 1.0;
  material.b = 1.0;
  material.roughness = 0.0;
  material.metallic = 1.0;

	// Get G-Buffer values
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec3 albedo = pow(texture(samplerAlbedo, inUV).rgb, vec3(2.2)); // * vec3(material.r, material.g, material.b);
  float roughness = texture(samplerRoughness, inUV).r;
  float metallic = texture(samplerMetallic, inUV).r;

	vec3 N = normalize(normal);
	vec3 V = normalize(ubo.viewPos.xyz - fragPos); 	// Viewer to fragment
	
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 Lo = vec3(0.0);
	for(int i = 0; i < LIGHT_COUNT; ++i) {

		vec3 L = normalize(ubo.lights[i].position.xyz - fragPos); // Vector to light
    vec3 H = normalize (V + L);
    
    float dist = length(ubo.lights[i].position.xyz - fragPos);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = ubo.lights[i].color.xyz * attenuation;

	  float dotNL = max(dot(N, L), 0.0);
	  float dotNH = max(dot(N, H), 0.0);
	  float dotNV = max(dot(N, V), 0.0);

    float NDF = DistributionGGX(N, H, roughness);     // Normal distribution (of the microfacets)
    float G = GeometrySmith(N, V, L, roughness);      // Geometric shadowing term (micorfacets shadowing)
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0); // F = Fresnel factor (Reflectance depending on angle of incidence)

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001) * 3.0;

    float NdotL = max(dot(N, L), 0.0);
	  Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
  
  if(ubo.useShadows > 0) {

    for(int i = 0; i < LIGHT_COUNT; ++i) {
      
      vec4 shadowClip = ubo.lights[i].view * vec4(fragPos, 1.0);

      float shadowFactor;
      #ifdef USE_PCF
        shadowFactor = filterPCF(shadowClip, i);
      #else
        shadowFactor = textureProj(shadowClip, i vec2(0.0));
      #endif

      Lo *= shadowFactor;
    }
  }

  vec3 ambient = vec3(AMBIENT_LIGHT) * albedo;
  vec3 color = ambient + Lo;

  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  outFragcolor = vec4(color, 1.0);	
}