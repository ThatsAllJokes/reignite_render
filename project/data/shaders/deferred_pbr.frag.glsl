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
	float radius;
  mat4 view;
};

layout (binding = 5, set = 0) uniform UBO {
	vec4 viewPos;
	Light lights[3];
  int numbLights;
  int useShadows;
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

vec3 albedo;
vec3 materialcolor() {

  return vec3(albedo.r, albedo.g, albedo.b);
}

float Distrib_GGX(float NdotH, float roughness) { // Normal distribution

  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;

  float NdotH2 = NdotH * NdotH;

  return (alpha2) / (PI * (NdotH2 * (alpha2 - 1.0f) + 1.0f) * (NdotH2 * (alpha2 - 1.0f) + 1.0f));
}

float Geo_SchlickSmithGGX(float NdotL, float NdotV, float roughness) { // Geometric Shadowing 

  float NdotL2 = NdotL * NdotL;
  float NdotV2 = NdotV * NdotV;
  float kRough2 = roughness * roughness + 0.0001f;

  float ggxL = (2.0f * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0f - NdotL2)));
  float ggxV = (2.0f * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0f - NdotV2)));

  return ggxL * ggxV;
}

vec3 Fresnel_Schlick(float cosTheta, float metallic) { // Fresnel

  vec3 F0 = mix(vec3(0.04), materialcolor(), metallic); // material * specular // TODO: this should be done with the albedo buffer
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness, int it) { // Specular BRDF

  // Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNL = max(clamp(dot(N, L), 0.0, 1.0), 0.0001);
	float dotNH = max(clamp(dot(N, H), 0.0, 1.0), 0.0001);
	float dotNV = max(clamp(dot(N, V), 0.0, 1.0), 0.0001);
	float dotLH = max(clamp(dot(L, H), 0.0, 1.0), 0.0001);

	vec3 color = vec3(0.0); 

  float rroughness = max(roughness * material.roughness, 0.05);
  float mmetallic = metallic * material.metallic;

  float D = Distrib_GGX(dotNH, rroughness);                // Normal distribution (of the microfacets)
  float G = Geo_SchlickSmithGGX(dotNL, dotNV, rroughness); // Geometric shadowing term (micorfacets shadowing)
  vec3 F = Fresnel_Schlick(dotNV, mmetallic);              // F = Fresnel factor (Reflectance depending on angle of incidence)

  vec3 spec = D * G * F / (4.0 * dotNL * dotNV);
	color += spec * dotNL * ubo.lights[it].color.xyz;

  return color;
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
	albedo = texture(samplerAlbedo, inUV).rgb * vec3(material.r, material.g, material.b);
  float roughness = texture(samplerRoughness, inUV).r;
  float metallic = texture(samplerMetallic, inUV).r;

	vec3 fragcolor = albedo.rgb * AMBIENT_LIGHT;
	
	vec3 N = normalize(normal);
	vec3 V = normalize(-ubo.viewPos.xyz); 	// Viewer to fragment

  float lightCosInnerAngle = cos(radians(15.0));
	float lightCosOuterAngle = cos(radians(25.0));
	float lightRange = 100.0;

	for(int i = 0; i < LIGHT_COUNT; ++i) {

		vec3 L = normalize(ubo.lights[i].position.xyz - fragPos); // Vector to light
    vec3 H = normalize (V + L);

    float dist = length(ubo.lights[i].position.xyz - fragPos);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = ubo.lights[i].color.xyz * attenuation;

	  float dotNL = max(dot(N, L), 0.0);
	  float dotNH = max(dot(N, H), 0.0);
	  float dotNV = max(dot(N, V), 0.0);

    float rroughness = max(roughness * material.roughness, 0.05);
    float mmetallic = metallic * material.metallic;

    float D = Distrib_GGX(dotNH, rroughness);                // Normal distribution (of the microfacets)
    float G = Geo_SchlickSmithGGX(dotNL, dotNV, rroughness); // Geometric shadowing term (micorfacets shadowing)
    vec3 F = Fresnel_Schlick(dotNV, mmetallic);              // F = Fresnel factor (Reflectance depending on angle of incidence)

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 spec = D * G * F / max((4.0 * dotNL * dotNV), 0.001);
	  fragcolor += (kD * albedo / PI + spec) * dotNL * radiance;
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

      fragcolor *= shadowFactor;
    }
  }

  //vec3 color = fragcolor;
  //color = color / (color + vec3(1.0));
  //color = pow(color, vec3(1.0/2.2)); // gamma correction/HDR

  outFragcolor = vec4(fragcolor, 1.0);	
}