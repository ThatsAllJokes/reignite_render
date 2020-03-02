#version 450

layout(binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 proj;
  mat4 view;
  vec3 camPos;
} ubo;

layout (binding = 1) uniform UBOShared {
	vec4 lights[4];
} uboParams;

layout(binding = 2) uniform sampler2D texSampler;

layout(push_constant) uniform PushConsts {
	float r;
	float g;
	float b;
	float roughness;
	float metallic;
} material;

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;

//#define ROUGHNESS_PATTERN 1

vec3 materialcolor() {

  return vec3(material.r, material.g, material.b);
}

float D_GGX(float dotNH, float roughness) { // Normal distribution

  float alpha = roughness * roughness;
  float alpha2 = alpha * alpha;
  float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
  return (alpha2)/(PI * denom * denom);
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness) { // Geometric Shadowing 

  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;
  float GL = dotNL / (dotNL * (1.0 - k) + k);
  float GV = dotNV / (dotNV * (1.0 - k) + k);
  return GL * GV;
}

vec3 F_Schlick(float cosTheta, float metallic) { // Fresnel

  vec3 F0 = mix(vec3(0.04), materialcolor(), metallic); // material * specular
  vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
  return F;
}

vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness) { // Specular BRDF

  // Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float dotLH = clamp(dot(L, H), 0.0, 1.0);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);

  vec3 lightColor = vec3(1.0);
	vec3 color = vec3(0.0); 

  if(dotNL > 0.0) {
    
    float rroughness = max(0.05, roughness); 
    float D = D_GGX(dotNH, roughness); // Normal distribution (of the microfacets)
    float G = G_SchlicksmithGGX(dotNL, dotNV, roughness); // Geometric shadowing term (micorfacets shadowing)
    vec3 F = F_Schlick(dotNV, metallic); // F = Fresnel factor (Reflectance depending on angle of incidence)

    vec3 spec = D * F * G / (4.0 * dotNL * dotNV);
		color += spec * dotNL * lightColor;
  }

  return color;
}

void main() {

  vec3 N = normalize(inNormal);
  vec3 V = normalize(ubo.camPos - inWorldPos);

  float roughness = material.roughness;

#ifdef ROUGHNESS_PATTERN
  roughness = max(roughness, step(fract(inWorldPos.y * 2.02), 0.5));
#endif

  vec3 Lo = vec3(0.0);
	for (int i = 0; i < uboParams.lights.length(); i++) {
		vec3 L = normalize(uboParams.lights[i].xyz - inWorldPos);
  	Lo += BRDF(L, V, N, material.metallic, roughness);
	};

	vec3 color = materialcolor() * 0.02;
	color += Lo;

	color = pow(color, vec3(0.4545)); // gamma correction/HDR

  outColor = vec4(color, 1.0);
}