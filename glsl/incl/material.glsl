
struct Material
{
  float Shine;

  float AmbientOcclusion;

  bool UseAlbedoMap;
  vec3 Albedo;
  sampler2D AlbedoMap;

  bool UseMetallicMap;
  float Metallic;
  sampler2D MetallicMap;

  bool UseRoughnessMap;
  float Roughness;
  sampler2D RoughnessMap;
};