
struct DirLight
{
  vec3 Direction;
  vec3 Color;
  float Ambient;
  float Diffuse;
  float Specular;
};

struct PointLight
{
  vec3 Position;

  float Constant;
  float Linear;
  float Quadratic;

  vec3 Color;
  float Ambient;
  float Diffuse;
  float Specular;
};