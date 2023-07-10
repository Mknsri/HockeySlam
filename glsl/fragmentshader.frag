#include "glsl_header.h"
#if GLES
GLSL_PREPROCESS(version 300 es)
precision highp float;
#else
GLSL_PREPROCESS(version 330 core)
#endif

const float kPi = 3.14159265;
const float kShininess = 16.0;

#include "light_inc.glsl"

GLSL_PREPROCESS(define NR_POINT_LIGHTS 4)
uniform PointLight uPointLights[NR_POINT_LIGHTS];
const float totalLightCount = 5.0;

struct Material
{
  float Shine;
  bool UseAlbedoMap;
  vec3 Albedo;
  sampler2D AlbedoMap;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform vec3 uCameraPos;
uniform DirLight uDirLight;
uniform Material uMaterial;
uniform sampler2D uShadowMap;

out vec4 FragColor;

#include "shadow_mapping_inc.glsl"

const float MAGIC_AMBIENT = 30.0f;
const float MAGIC_DIFFUSE = 30.0f;
const float MAGIC_SPEC = 30.0f;

float calculateSpecular(vec3 lightDir, vec3 viewDir, vec3 normal)
{
  float spec = 0.0;

  const bool useBlinn = true;
  if (useBlinn) {

    const float kEnergyConservation = (8.0 + kShininess) / (8.0 * kPi);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = kEnergyConservation *
           pow(max(dot(viewDir, halfwayDir), 0.0), uMaterial.Shine);
  } else {

    const float kEnergyConservation = (2.0 + kShininess) / (2.0 * kPi);
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = kEnergyConservation *
           pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.Shine);
  }

  return spec;
}

vec3 calculateDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.Direction);

  float diff = max(dot(normal, lightDir), 0.0);

  float spec = calculateSpecular(lightDir, viewDir, normal);

  float ambient = light.Ambient * MAGIC_AMBIENT;
  float diffuse = light.Diffuse * MAGIC_DIFFUSE * diff;
  float specular = light.Specular * MAGIC_SPEC * spec;
  return (ambient)*light.Color;
}

vec3 calculatePointLight(PointLight light,
                         vec3 normal,
                         vec3 fragPos,
                         vec3 viewDir)
{
  vec3 lightDir = normalize(light.Position - viewDir);

  float diff = max(dot(normal, lightDir), 0.0);

  float spec = calculateSpecular(lightDir, viewDir, normal);

  float distance = length(light.Position - fragPos);
#if 0
  float attenuation = 1.0 / (distance * distance);
#else
  float attenuation = 1.0 / (light.Constant + light.Linear * distance +
                             light.Quadratic * (distance * distance));
#endif

  float ambient = light.Ambient * MAGIC_AMBIENT * diff;
  float diffuse = light.Diffuse * MAGIC_DIFFUSE * diff;
  float specular = light.Specular * MAGIC_SPEC * spec;
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return (ambient * (diffuse + specular)) * light.Color;
}

void main()
{
  vec4 albedoA = uMaterial.UseAlbedoMap
                   ? texture(uMaterial.AlbedoMap, TexCoords)
                   : vec4(uMaterial.Albedo, 1.0);
  vec3 albedo = pow(albedoA.rgb, vec3(2.2));

  vec3 N = Normal;

  vec3 viewDir = normalize(uCameraPos - FragPos);
  vec3 lightDir = normalize(-uDirLight.Direction);
  vec3 lightContribution = calculateDirLight(uDirLight, N, viewDir);

  for (int i = 0; i < NR_POINT_LIGHTS; i++) {
    lightContribution +=
      calculatePointLight(uPointLights[i], N, FragPos, viewDir);
  }

  lightContribution = lightContribution / totalLightCount;

  float shadowAmount =
    shadowCalculation(FragPosLightSpace, N, uDirLight.Direction);
  vec4 result = vec4(albedo * lightContribution, 1.0);
  // HDR tonemapping
  result = result / (result + vec4(1.0));
  // gamma correct
  result = pow(result, vec4(1.0 / 2.2)) - shadowAmount;

  FragColor = vec4(result.rgb, albedoA.a);
}
