
static const shader_uniform_lights SetupLightUniform(const GLuint programId)
{
  HOKI_WARN(programId >= HOKI_OGL_NO_ID);
  shader_uniform_lights result = {};
  result.DirLight.DirectionId =
    glGetUniformLocation(programId, "uDirLight.Direction");
  result.DirLight.ColorId = glGetUniformLocation(programId, "uDirLight.Color");
  result.DirLight.AmbientId =
    glGetUniformLocation(programId, "uDirLight.Ambient");
  result.DirLight.DiffuseId =
    glGetUniformLocation(programId, "uDirLight.Diffuse");
  result.DirLight.SpecularId =
    glGetUniformLocation(programId, "uDirLight.Specular");

  HOKI_WARN(result.DirLight.DirectionId >= HOKI_OGL_NO_ID);
  HOKI_WARN(result.DirLight.ColorId >= HOKI_OGL_NO_ID);
  HOKI_WARN(result.DirLight.AmbientId >= HOKI_OGL_NO_ID);
  HOKI_WARN(result.DirLight.DiffuseId >= HOKI_OGL_NO_ID);
  HOKI_WARN(result.DirLight.SpecularId >= HOKI_OGL_NO_ID);

  for (size_t i = 0; i < MAX_LIGHTS; i++) {
    std::string name = "uPointLights[" + std::to_string(i) + "]";
    result.PointLights[i].PositionId =
      glGetUniformLocation(programId, (name + ".Position").c_str());
    result.PointLights[i].ColorId =
      glGetUniformLocation(programId, (name + ".Color").c_str());
    result.PointLights[i].AmbientId =
      glGetUniformLocation(programId, (name + ".Ambient").c_str());
    result.PointLights[i].DiffuseId =
      glGetUniformLocation(programId, (name + ".Diffuse").c_str());
    result.PointLights[i].SpecularId =
      glGetUniformLocation(programId, (name + ".Specular").c_str());
    result.PointLights[i].ConstantId =
      glGetUniformLocation(programId, (name + ".Constant").c_str());
    result.PointLights[i].LinearId =
      glGetUniformLocation(programId, (name + ".Linear").c_str());
    result.PointLights[i].QuadraticId =
      glGetUniformLocation(programId, (name + ".Quadratic").c_str());

    HOKI_WARN(result.PointLights[i].PositionId >= HOKI_OGL_NO_ID);
    HOKI_WARN(result.PointLights[i].AmbientId >= HOKI_OGL_NO_ID);
    HOKI_WARN(result.PointLights[i].DiffuseId >= HOKI_OGL_NO_ID);
    HOKI_WARN(result.PointLights[i].SpecularId >= HOKI_OGL_NO_ID);
    HOKI_WARN(result.PointLights[i].ConstantId >= HOKI_OGL_NO_ID);
    HOKI_WARN(result.PointLights[i].LinearId >= HOKI_OGL_NO_ID);
    HOKI_WARN(result.PointLights[i].QuadraticId >= HOKI_OGL_NO_ID);
  }
  result.NextAvailablePointLight = 0;

  return result;
}

static const shader_uniform_material SetupMaterialUniform(
  const GLuint programId)
{
  HOKI_WARN(programId >= HOKI_OGL_NO_ID);
  shader_uniform_material result = {};
  result.ShineId = glGetUniformLocation(programId, "uMaterial.Shine");

  result.AlbedoId = glGetUniformLocation(programId, "uMaterial.Albedo");
  result.AlbedoMapId = glGetUniformLocation(programId, "uMaterial.AlbedoMap");
  result.UseAlbedoMapId =
    glGetUniformLocation(programId, "uMaterial.UseAlbedoMap");

  result.MetallicId = glGetUniformLocation(programId, "uMaterial.Metallic");
  result.MetallicMapId =
    glGetUniformLocation(programId, "uMaterial.MetallicMap");
  result.UseMetallicMapId =
    glGetUniformLocation(programId, "uMaterial.UseMetallicMap");

  result.RoughnessId = glGetUniformLocation(programId, "uMaterial.Roughness");
  result.RoughnessMapId =
    glGetUniformLocation(programId, "uMaterial.RoughnessMap");
  result.UseRoughnessMapId =
    glGetUniformLocation(programId, "uMaterial.UseRoughnessMap");

  HOKI_WARN(result.ShineId >= HOKI_OGL_NO_ID);
  HOKI_WARN(result.AlbedoId >= HOKI_OGL_NO_ID);
  HOKI_WARN(result.MetallicId >= HOKI_OGL_NO_ID);
  HOKI_WARN(result.RoughnessId >= HOKI_OGL_NO_ID);

  return result;
}

static const shader_uniform SetupUniform(const GLuint programId,
                                         const char* name,
                                         const shader_uniform_type type)
{
  HOKI_WARN(programId >= HOKI_OGL_NO_ID);
  shader_uniform result = {};
  result.Id = glGetUniformLocation(programId, name);
  result.Type = type;
#if HOKI_DEV
  result.Name = (char*)name;
#endif
  HOKI_ASSERT_NO_OPENGL_ERRORS();
  HOKI_WARN_MESSAGE(result.Id >= HOKI_OGL_NO_ID, name, 0);

  return result;
}

static void SetUniform(const shader_uniform uniform,
                       const mat4x4* mat4,
                       const size_t count)
{
  HOKI_ASSERT(uniform.Type == SHADER_UNIFORM_MAT4 ||
              uniform.Type == SHADER_UNIFORM_MAT4_ARRAY);
#ifdef GL_ES_VERSION_3_0
  if (uniform.Type == SHADER_UNIFORM_MAT4_ARRAY) {
    glUniform4fv(uniform.Id, (GLsizei)(4 * count), (GLfloat*)mat4->S);
  } else {
    glUniformMatrix4fv(uniform.Id, (GLsizei)count, false, (GLfloat*)mat4->S);
  }
#else
  glUniformMatrix4fv(uniform.Id, (GLsizei)count, false, (GLfloat*)mat4->S);
#endif
  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetUniform(const shader_uniform uniform, const int value)
{
  HOKI_ASSERT(uniform.Type == SHADER_UNIFORM_INT);
  glUniform1i(uniform.Id, value);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetUniform(const shader_uniform uniform,
                       const v2* vec2,
                       const size_t count)
{
  HOKI_ASSERT(uniform.Type == SHADER_UNIFORM_VEC2);
  glUniform2fv(uniform.Id, (GLsizei)count, vec2->E);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetUniform(const shader_uniform uniform,
                       const v3* vec3,
                       const size_t count)
{
  HOKI_ASSERT(uniform.Type == SHADER_UNIFORM_VEC3);
  glUniform3fv(uniform.Id, (GLsizei)count, vec3->E);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetUniform(const shader_uniform uniform,
                       const v4* vec4,
                       const size_t count)
{
  HOKI_ASSERT(uniform.Type == SHADER_UNIFORM_VEC4);
  glUniform4fv(uniform.Id, (GLsizei)count, vec4->E);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetUniform(const shader_uniform uniform, const bool value)
{
  HOKI_ASSERT(uniform.Type == SHADER_UNIFORM_BOOL);
  glUniform1i(uniform.Id, value ? 1 : 0);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetUniform(shader_uniform_lights& uniform, const game_light* light)
{
  switch (light->LightType) {
    case LIGHT_TYPE_DIRECTION:
      glUniform3fv(
        uniform.DirLight.DirectionId, (GLsizei)1, (GLfloat*)light->Vector.E);
      glUniform3fv(uniform.DirLight.ColorId, 1, (GLfloat*)light->Color.E);
      glUniform1f(uniform.DirLight.AmbientId, light->Ambient);
      glUniform1f(uniform.DirLight.DiffuseId, light->Diffuse);
      glUniform1f(uniform.DirLight.SpecularId, light->Specular);
      break;

    case LIGHT_TYPE_POINT: {
      size_t idx = (uniform.NextAvailablePointLight++) % MAX_LIGHTS;
      glUniform3fv(uniform.PointLights[idx].PositionId,
                   (GLsizei)1,
                   (GLfloat*)light->Vector.E);
      glUniform3fv(
        uniform.PointLights[idx].ColorId, 1, (GLfloat*)light->Color.E);
      glUniform1f(uniform.PointLights[idx].AmbientId, light->Ambient);
      glUniform1f(uniform.PointLights[idx].DiffuseId, light->Diffuse);
      glUniform1f(uniform.PointLights[idx].SpecularId, light->Specular);
      glUniform1f(uniform.PointLights[idx].ConstantId, light->Constant);
      glUniform1f(uniform.PointLights[idx].LinearId, light->Linear);
      glUniform1f(uniform.PointLights[idx].QuadraticId, light->Quadratic);
    } break;

    default:
      HOKI_ASSERT_MESSAGE(false, "Undefined light type");
  }

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetUniform(const shader_uniform_material uniform,
                       const Asset::material* const material,
                       const int32_t albedoMapId,
                       const int32_t roughnessMapId,
                       const int32_t metallicMapId,
                       const int32_t samplerStartIndex,
                       const size_t count)
{
  HOKI_ASSERT(count == 1);
  glUniform1f(uniform.ShineId, material->Shine);
  glUniform3fv(uniform.AlbedoId, 1, &material->Albedo.E[0]);

  int32_t textureIndex = samplerStartIndex;
  if (albedoMapId != HOKI_OGL_INVALID_ID) {
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, albedoMapId);
    glUniform1i(uniform.AlbedoMapId, textureIndex);
    textureIndex++;
  }
  glUniform1i(uniform.UseAlbedoMapId,
              (albedoMapId != HOKI_OGL_INVALID_ID) ? 1 : 0);

  glUniform1f(uniform.MetallicId, material->Metallic);
  if (metallicMapId != HOKI_OGL_INVALID_ID) {
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, metallicMapId);
    glUniform1i(uniform.MetallicMapId, textureIndex);
    textureIndex++;
  }
  glUniform1i(uniform.UseMetallicMapId,
              (metallicMapId != HOKI_OGL_INVALID_ID) ? 1 : 0);

  glUniform1f(uniform.RoughnessId, material->Roughness);
  if (roughnessMapId != HOKI_OGL_INVALID_ID) {
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, roughnessMapId);
    glUniform1i(uniform.RoughnessMapId, textureIndex);
    textureIndex++;
  }
  glUniform1i(uniform.UseRoughnessMapId,
              (roughnessMapId != HOKI_OGL_INVALID_ID) ? 1 : 0);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void SetSamplerUniform(const shader_uniform uniform, GLint value)
{
  HOKI_ASSERT(uniform.Type == SHADER_UNIFORM_SAMPLER2D);
  glUniform1i(uniform.Id, value);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void CreateShader(const ogl_shader_program program,
                         render_context& context)
{
  renderable* renderableVertexShader =
    (renderable*)get(*context.RenderableStore, (uintptr_t)program.VertexShader);
  renderable* renderableFragmentShader = (renderable*)get(
    *context.RenderableStore, (uintptr_t)program.FragmentShader);

  renderableVertexShader->RenderId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(
    renderableVertexShader->RenderId, 1, &program.VertexShader->Memory, NULL);
  glCompileShader(renderableVertexShader->RenderId);

#if HOKI_DEV && !GL_ES_VERSION_3_0
  // apply the name, -1 means NULL terminated
  glObjectLabel(GL_SHADER,
                renderableVertexShader->RenderId,
                -1,
                program.VertexShader->Name);
  HOKI_ASSERT_NO_OPENGL_ERRORS();
#endif

  int vertexCompilationSuccess;
  char assertMessageBuffer[4 * 512];
  char infoLog[3 * 512];
  glGetShaderiv(renderableVertexShader->RenderId,
                GL_COMPILE_STATUS,
                &vertexCompilationSuccess);
  if (!vertexCompilationSuccess) {
    glGetShaderInfoLog(renderableVertexShader->RenderId, 512, NULL, infoLog);
    snprintf(assertMessageBuffer,
             sizeof(assertMessageBuffer),
             "ERROR: Vertex shader (%s) compilation failed: %s\n",
             program.VertexShader->Name,
             infoLog);
    HOKI_ASSERT_MESSAGE(vertexCompilationSuccess, assertMessageBuffer);
  }

  renderableFragmentShader->RenderId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(renderableFragmentShader->RenderId,
                 1,
                 &program.FragmentShader->Memory,
                 NULL);
  glCompileShader(renderableFragmentShader->RenderId);

#if HOKI_DEV && !GL_ES_VERSION_3_0
  // apply the name, -1 means NULL terminated
  glObjectLabel(GL_SHADER,
                renderableFragmentShader->RenderId,
                -1,
                program.FragmentShader->Name);
  HOKI_ASSERT_NO_OPENGL_ERRORS();
#endif

  int fragmentCompilationSuccess;
  glGetShaderiv(renderableFragmentShader->RenderId,
                GL_COMPILE_STATUS,
                &fragmentCompilationSuccess);
  if (fragmentCompilationSuccess) {
    glGetShaderInfoLog(renderableFragmentShader->RenderId, 512, NULL, infoLog);
    snprintf(assertMessageBuffer,
             sizeof(assertMessageBuffer),
             "ERROR: Fragment shader (%s) compilation failed: %s\n",
             program.FragmentShader->Name,
             infoLog);
    HOKI_ASSERT_MESSAGE(fragmentCompilationSuccess, assertMessageBuffer, 0);
  }

  GLuint programId = glCreateProgram();

#if HOKI_DEV && !GL_ES_VERSION_3_0
  // apply the name, -1 means NULL terminated
  glObjectLabel(GL_PROGRAM, programId, -1, debug_to_string(program.Type));
  HOKI_ASSERT_NO_OPENGL_ERRORS();
#endif

  glAttachShader(programId, renderableVertexShader->RenderId);
  glAttachShader(programId, renderableFragmentShader->RenderId);
  glLinkProgram(programId);

  int programLinkSuccess;
  glGetProgramiv(programId, GL_LINK_STATUS, &programLinkSuccess);
  if (!programLinkSuccess) {
    glGetProgramInfoLog(programId, 512, NULL, infoLog);
    snprintf(assertMessageBuffer,
             sizeof(assertMessageBuffer),
             "ERROR: Shader linking failed: %s\n",
             infoLog);
    HOKI_ASSERT_MESSAGE(programLinkSuccess, assertMessageBuffer, 0);
  }

  glDeleteShader(renderableVertexShader->RenderId);
  glDeleteShader(renderableFragmentShader->RenderId);

  glUseProgram(programId);
  switch (program.Type) {
#if HOKI_DEV
    case Asset::SHADER_TYPE_WEIGHTS:
      context.WeightedShaderProgram = programId;
      break;
#endif
    case Asset::SHADER_TYPE_SIMPLE:
      context.SimpleShader = {};
      context.SimpleShader.Id = programId;
      context.SimpleShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.SimpleShader.ViewProjection =
        SetupUniform(programId, "uViewProjection", SHADER_UNIFORM_MAT4);
      context.SimpleShader.ObjectColor =
        SetupUniform(programId, "uObjectColor", SHADER_UNIFORM_VEC3);
      break;

    case Asset::SHADER_TYPE_SIMPLE_TEXTURED:
      context.SimpleTexturedShader = {};
      context.SimpleTexturedShader.Id = programId;
      context.SimpleTexturedShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.SimpleTexturedShader.ViewProjection =
        SetupUniform(programId, "uViewProjection", SHADER_UNIFORM_MAT4);
      context.SimpleTexturedShader.TextureDiffuse1 =
        SetupUniform(programId, "uTexture_diffuse1", SHADER_UNIFORM_SAMPLER2D);
      break;

    case Asset::SHADER_TYPE_ANIMATED_MESH:
      context.AnimatedMeshShader = {};
      context.AnimatedMeshShader.Id = programId;
      context.AnimatedMeshShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.AnimatedMeshShader.ViewProjection =
        SetupUniform(programId, "uViewProjection", SHADER_UNIFORM_MAT4);
      context.AnimatedMeshShader.HasBones =
        SetupUniform(programId, "uHasBones", SHADER_UNIFORM_BOOL);
      context.AnimatedMeshShader.Bones =
        SetupUniform(programId, "uBones", SHADER_UNIFORM_MAT4_ARRAY);
      context.AnimatedMeshShader.LightSpaceMatrix =
        SetupUniform(programId, "uLightSpaceMatrix", SHADER_UNIFORM_MAT4);
      context.AnimatedMeshShader.CameraPos =
        SetupUniform(programId, "uCameraPos", SHADER_UNIFORM_VEC3);
      context.AnimatedMeshShader.Lights = SetupLightUniform(programId);
      context.AnimatedMeshShader.Material = SetupMaterialUniform(programId);
      context.AnimatedMeshShader.TextureDiffuse1 =
        SetupUniform(programId, "uTexture_diffuse1", SHADER_UNIFORM_SAMPLER2D);
      context.AnimatedMeshShader.TextureNormal1 =
        SetupUniform(programId, "uTexture_normal1", SHADER_UNIFORM_SAMPLER2D);
      context.AnimatedMeshShader.ShadowMap =
        SetupUniform(programId, "uShadowMap", SHADER_UNIFORM_SAMPLER2D);
      break;

    case Asset::SHADER_TYPE_PBR:
      context.PbrShader = {};
      context.PbrShader.Id = programId;
      context.PbrShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.PbrShader.ViewProjection =
        SetupUniform(programId, "uViewProjection", SHADER_UNIFORM_MAT4);
      context.PbrShader.HasBones =
        SetupUniform(programId, "uHasBones", SHADER_UNIFORM_BOOL);
      context.PbrShader.LightSpaceMatrix =
        SetupUniform(programId, "uLightSpaceMatrix", SHADER_UNIFORM_MAT4);
      context.PbrShader.Bones =
        SetupUniform(programId, "uBones", SHADER_UNIFORM_MAT4_ARRAY);
      context.PbrShader.CameraPos =
        SetupUniform(programId, "uCameraPos", SHADER_UNIFORM_VEC3);
      context.PbrShader.Lights = SetupLightUniform(programId);
      context.PbrShader.Material = SetupMaterialUniform(programId);
      context.PbrShader.TextureDiffuse1 =
        SetupUniform(programId, "uTexture_diffuse1", SHADER_UNIFORM_SAMPLER2D);
      context.PbrShader.ShadowMap =
        SetupUniform(programId, "uShadowMap", SHADER_UNIFORM_SAMPLER2D);
      break;

    case Asset::SHADER_TYPE_PBR_INSTANCED:
      context.PbrInstancedShader = {};
      context.PbrInstancedShader.Id = programId;
      context.PbrInstancedShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.PbrInstancedShader.ViewProjection =
        SetupUniform(programId, "uViewProjection", SHADER_UNIFORM_MAT4);
      context.PbrInstancedShader.HasBones =
        SetupUniform(programId, "uHasBones", SHADER_UNIFORM_BOOL);
      context.PbrInstancedShader.LightSpaceMatrix =
        SetupUniform(programId, "uLightSpaceMatrix", SHADER_UNIFORM_MAT4);
      context.PbrInstancedShader.Bones =
        SetupUniform(programId, "uBones", SHADER_UNIFORM_MAT4_ARRAY);
      context.PbrInstancedShader.CameraPos =
        SetupUniform(programId, "uCameraPos", SHADER_UNIFORM_VEC3);
      context.PbrInstancedShader.Lights = SetupLightUniform(programId);
      context.PbrInstancedShader.Material = SetupMaterialUniform(programId);
      context.PbrInstancedShader.TextureDiffuse1 =
        SetupUniform(programId, "uTexture_diffuse1", SHADER_UNIFORM_SAMPLER2D);
      context.PbrInstancedShader.ShadowMap =
        SetupUniform(programId, "uShadowMap", SHADER_UNIFORM_SAMPLER2D);
      context.PbrInstancedShader.InstanceCount =
        SetupUniform(programId, "uInstanceCount", SHADER_UNIFORM_INT);
      context.PbrInstancedShader.InstanceSpacing =
        SetupUniform(programId, "uInstanceSpacing", SHADER_UNIFORM_VEC3);
      break;

    case Asset::SHADER_TYPE_TEXT:
      context.TextShader = {};
      context.TextShader.Id = programId;
      context.TextShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.TextShader.ViewProjection =
        SetupUniform(programId, "uProjection", SHADER_UNIFORM_MAT4);
      context.TextShader.TextColor =
        SetupUniform(programId, "uTextColor", SHADER_UNIFORM_VEC3);
      context.TextShader.CharacterTexture =
        SetupUniform(programId, "uCharacterTexture", SHADER_UNIFORM_SAMPLER2D);
      break;

    case Asset::SHADER_TYPE_SHADOW:
      context.ShadowShader = {};
      context.ShadowShader.Id = programId;
      context.ShadowShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.ShadowShader.ViewProjection =
        SetupUniform(programId, "uViewProjection", SHADER_UNIFORM_MAT4);
      context.ShadowShader.Bones =
        SetupUniform(programId, "uBones", SHADER_UNIFORM_MAT4_ARRAY);
      context.ShadowShader.CameraPos =
        SetupUniform(programId, "uCameraPos", SHADER_UNIFORM_VEC3);
      context.ShadowShader.HasBones =
        SetupUniform(programId, "uHasBones", SHADER_UNIFORM_BOOL);

      break;

    case Asset::SHADER_TYPE_FILLED:
      context.FillRevealShader = {};
      context.FillRevealShader.Id = programId;
      break;

    case Asset::SHADER_TYPE_UI:
      context.UIShader = {};
      context.UIShader.ModelMatrix =
        SetupUniform(programId, "uModelMatrix", SHADER_UNIFORM_MAT4);
      context.UIShader.ViewProjection =
        SetupUniform(programId, "uViewProjection", SHADER_UNIFORM_MAT4);
      context.UIShader.TextureCoordOffset =
        SetupUniform(programId, "uTextureOffset", SHADER_UNIFORM_VEC2);
      context.UIShader.TextureDiffuse1 =
        SetupUniform(programId, "uTexture_diffuse1", SHADER_UNIFORM_SAMPLER2D);
      context.UIShader.Id = programId;
      break;

    default:
      HOKI_ASSERT(false);
      break;
  }
}