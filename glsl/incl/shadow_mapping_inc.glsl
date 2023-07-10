float testShadowMap(vec2 coordinates, float depth)
{
  return step(texture(uShadowMap, coordinates).r, depth);
}

float sampleShadowLinear(vec2 coordinates, vec2 texelSize, float depth)
{
  vec2 pixelCoord = coordinates / texelSize + vec2(0.5);
  vec2 pixelFract = fract(pixelCoord);
  vec2 startTexel = (pixelCoord - pixelFract) * texelSize;

  float bottomLeftTexel = testShadowMap(startTexel, depth);
  float bottomRightTexel =
    testShadowMap(startTexel + vec2(texelSize.x, 0.0), depth);
  float topLeftTexel =
    testShadowMap(startTexel + vec2(0.0, texelSize.y), depth);
  float topRightTexel = testShadowMap(startTexel + texelSize, depth);

  float mixLeft = mix(bottomLeftTexel, topLeftTexel, pixelFract.y);
  float mixRight = mix(bottomRightTexel, topRightTexel, pixelFract.y);

  return mix(mixLeft, mixRight, pixelFract.x);
}

float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;

  if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 ||
      projCoords.y > 1.0 || projCoords.z < 0.0 || projCoords.z > 1.0) {
    return 0.0;
  }

  float normalToLightDot = 1.0 - dot(normal, lightDir);
  const float shadowBiasConstant = 0.001;
  float bias = max(0.01 * normalToLightDot, 0.005);

  float closestDepth = texture(uShadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z - bias;

  vec2 poissonDisk[4] = vec2[](vec2(-0.94201624, -0.39906216),
                               vec2(0.94558609, -0.76890725),
                               vec2(-0.094184101, -0.92938870),
                               vec2(0.34495938, 0.29387760));

  float samples = 3.0;
  float samplesStart = (samples - 1.0) / 2.0;
  float shadow = 0.0;
  vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));
  for (float y = -samplesStart; y < samplesStart; y++) {
    for (float x = -samplesStart; x < samplesStart; x++) {
      // float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x,y) *
      // texelSize).r; shadow += currentDepth > pcfDepth ? 1.0 : 0.0; shadow +=
      // testShadowMap(projCoords.xy + vec2(x,y) * texelSize, currentDepth) *
      // normalToLightDot; shadow += sampleShadowLinear(projCoords.xy +
      // vec2(x,y), texelSize, currentDepth);
      shadow += sampleShadowLinear(projCoords.xy +
                                     (vec2(x, y) + poissonDisk[int(y)] / 700.0),
                                   texelSize,
                                   currentDepth);
    }
  }

  shadow /= (samples * samples);

  return clamp(shadow * abs(dot(normal, lightDir)), 0.0, 1.0);
}
