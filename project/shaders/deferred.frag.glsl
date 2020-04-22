#version 450

layout (binding = 1) uniform sampler2D samplerposition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout (binding = 4) uniform UBO 
{
	Light lights[6];
	vec4 viewPos;
} ubo;

// 1.0f, 0.765557f, 0.336057f), 0.1f, 1.0f
struct PushcConstants {
  float r;
  float g;
  float b;
  float roughness;
  float metallic;
} material;

const float PI = 3.14159265359;

vec4 albedo;
vec3 materialcolor() {

  return vec3(material.r, material.g, material.b);
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
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = max(clamp(dot(N, V), 0.0, 1.0), 0.0001);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);

	vec3 color = vec3(0.0); 

  float rroughness = max(0.05, roughness); 
  float D = Distrib_GGX(dotNH, roughness);                // Normal distribution (of the microfacets)
  float G = Geo_SchlickSmithGGX(dotNL, dotNV, roughness); // Geometric shadowing term (micorfacets shadowing)
  vec3 F = Fresnel_Schlick(dotNV, metallic);              // F = Fresnel factor (Reflectance depending on angle of incidence)

  vec3 spec = D * G * F / (4.0 * dotNL * dotNV);
	color += spec * dotNL * ubo.lights[it].color;

  return color;
}

vec3 colorLinear(vec3 colorVector) {

    vec3 linearColor = pow(colorVector.rgb, vec3(2.2f));
    return linearColor;
}

void main() {

  // init temp test values
  material.r = 0.1;
  material.g = 0.1;
  material.b = 0.1;
  material.roughness = 0.8;
  material.metallic = 0.1;

	// Get G-Buffer values
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec3 albedo = colorLinear(texture(samplerAlbedo, inUV).rgb);
	
	#define lightCount 6
	#define ambient 0.0
	
	vec3 N = normalize(normal);
	vec3 V = normalize(-ubo.viewPos.xyz); 	// Viewer to fragment

  float roughness = material.roughness;

	vec3 fragcolor = albedo * ambient; // Ambient part
	for(int i = 0; i < lightCount; ++i) {

		vec3 L = normalize(ubo.lights[i].position.xyz - fragPos); // Vector to light
		fragcolor += BRDF(L, V, N, material.metallic, roughness, i);
	}
  
  vec3 color = albedo * 0.02;
  color += fragcolor;
   
  color = pow(color, vec3(0.4545)); // gamma correction/HDR

  outFragcolor = vec4(color, 1.0);	
}