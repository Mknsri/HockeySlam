#include "glsl_header.h"
#if GLES
GLSL_PREPROCESS(version 300 es)
precision highp float;
#else
GLSL_PREPROCESS(version 330 core)
#endif

const float kPi = 3.14159265359;
const float kShininess = 16.0;

#include "light_inc.glsl"

#define NR_POINT_LIGHTS 4
uniform PointLight uPointLights[NR_POINT_LIGHTS];

#include "material.glsl"

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform vec3 uCameraPos;
uniform DirLight uDirLight;
uniform Material uMaterial;
uniform sampler2D uTexture_normal1;
uniform sampler2D uShadowMap;

out vec4 FragColor;

#include "shadow_mapping_inc.glsl"

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = kPi * denom * denom;

  return nom /
         max(denom,
             0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
  cosTheta = min(cosTheta, 1.0);
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
void main()
{
  vec3 N = Normal; // uMaterial.UseNormalMap ? texture(uTexture_normal1,
                   // TexCoords).xyz : Normal;
  vec3 V = normalize(-FragPos);

  vec4 albedoA = uMaterial.UseAlbedoMap
                   ? texture(uMaterial.AlbedoMap, TexCoords)
                   : vec4(uMaterial.Albedo, 1.0);

  vec3 albedo = pow(albedoA.rgb, vec3(2.2));
  float metallic = uMaterial.UseMetallicMap
                     ? texture(uMaterial.MetallicMap, TexCoords).r
                     : uMaterial.Metallic;
  float roughness = uMaterial.UseRoughnessMap
                      ? texture(uMaterial.RoughnessMap, TexCoords).g
                      : uMaterial.Roughness;

  // calculate reflectance at normal incidence; if dia-electric (like plastic)
  // use F0 of 0.04 and if it's a metal, use the albedo color as F0 (metallic
  // workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  for (int i = 0; i < NR_POINT_LIGHTS; i++) {
    // calculate per-light radiance
    vec3 L = normalize(uPointLights[i].Position - FragPos);
    vec3 H = normalize(V + L);
    float distance = length(L);
#if 0
    float attenuation = 1.0 / (distance * distance);
#else
    float attenuation =
      1.0 / (uPointLights[i].Constant + uPointLights[i].Linear * distance +
             uPointLights[i].Quadratic * (distance * distance));
#endif
    vec3 radiance = uPointLights[i].Color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular =
      nominator /
      max(denominator,
          0.001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * albedo + specular) * radiance *
          NdotL; // note that we already multiplied the BRDF by the Fresnel (kS)
                 // so we won't multiply by kS again
  }

  // ambient lighting
  vec3 ambient = uDirLight.Ambient * uDirLight.Color * albedo;

  vec3 color = (ambient + Lo);

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  float shadowAmount =
    shadowCalculation(FragPosLightSpace, N, uDirLight.Direction);

  color = pow(color, vec3(1.0 / 2.2)) - shadowAmount;

#if 1
  FragColor = vec4(color, albedoA.a);
#else
  FragColor = vec4(N, 1.0);
#endif
}
