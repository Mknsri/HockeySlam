
static void SetupUIContext(UISystem::ui_context& uiContext,
                           game_window_info& windowInfo,
                           render_context& renderContext)
{
  static const float size = 1.0f;

  float scale, width, height;
  if (windowInfo.Height > windowInfo.Width) { // portrait
    scale = (float)windowInfo.Width / windowInfo.Height;
    width = size;
    height = size * scale;
  } else {
    scale = (float)windowInfo.Height / windowInfo.Width;
    width = size * scale;
    height = size;
  }
  uiContext.AspectScale = _v2(width, height);
  uiContext.ViewProjection = orthographic_matrix(
    0.0f, size, -uiContext.TopMargin, size - uiContext.TopMargin, -size, size);
}

// Different texture coords
static uint32_t BindUIRect(render_context& context)
{
  if (context.UIRectPrimitive != HOKI_OGL_NO_ID) {
    return context.UIRectPrimitive;
  }

  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  const GLfloat vertices[6][4] = { { 0.0f, 0.0f, 0.0, 0.0 },
                                   { 0.0f, 1.0f, 0.0, 1.0 },
                                   { 1.0f, 0.0f, 0.5, 0.0 },

                                   { 0.0f, 1.0f, 0.0, 1.0 },
                                   { 1.0f, 0.0f, 0.5, 0.0 },
                                   { 1.0f, 1.0f, 0.5, 1.0 } };

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

  // vertex texture coords
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(GLfloat) * 4,
                        (void*)(sizeof(GLfloat) * 2));

  context.UIRectPrimitive = VAO;
  return context.UIRectPrimitive;
}

static uint32_t BindUITexture(const Asset::texture& texture)
{
  uint32_t texobj;

  glGenTextures(1, &texobj);
  glBindTexture(GL_TEXTURE_2D, texobj);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  HOKI_ASSERT(texture.Type != Asset::TEXTURE_TYPE_INVALID);
  if (texture.Type == Asset::TEXTURE_TYPE_DDS) {
    const Asset::DDS_Image& image = *texture.DDSImage;
    Asset::DDS_Surface surface = (Asset::DDS_Surface)image.Surfaces[0];
    glCompressedTexImage2D(GL_TEXTURE_2D,
                           0,
                           image.Format,
                           surface.Width,
                           surface.Height,
                           0,
                           surface.Size,
                           surface.Pixels);

    for (uint32_t i = 0; i < image.SurfaceCount; i++) {
      Asset::DDS_Surface mipmapSurface = image.Surfaces[i + 1];

      glCompressedTexImage2D(GL_TEXTURE_2D,
                             i + 1,
                             image.Format,
                             mipmapSurface.Width,
                             mipmapSurface.Height,
                             0,
                             mipmapSurface.Size,
                             mipmapSurface.Pixels);
    }
  } else if (texture.Type == Asset::TEXTURE_TYPE_IMAGE) {
    const Asset::image_texture& image = *texture.Image;
#if HOKI_DEV && !GL_ES_VERSION_3_0
    // apply the name, -1 means NULL terminated
    glObjectLabel(GL_TEXTURE, texobj, -1, image.Path.Value);
#endif
    int nrComponents = image.Components;
    if (image.Memory) {
      GLenum format = GL_RED;
      if (nrComponents == 1) {
        format = GL_RED;
      } else if (nrComponents == 3) {
        format = GL_RGB;
      } else if (nrComponents == 4) {
        format = GL_RGBA;
      }

      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   format,
                   (int)image.PixelSize.X,
                   (int)image.PixelSize.Y,
                   0,
                   format,
                   GL_UNSIGNED_BYTE,
                   image.Memory);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  return texobj;
}

static void RenderButton(const UISystem::ui_button& button,
                         render_context context)
{
  UISystem::ui_context& uiContext = *button.Context;

  renderable* image =
    (renderable*)get(*context.RenderableStore, (uintptr_t)button.Image);
  if (image->RenderId == HOKI_OGL_NO_ID) {
    image->RenderId = BindUITexture(*button.Image);
  }

  mat4x4 modelMatrix = IDENTITY_MATRIX *
                       mat4x4_translate(_v3(button.Position, 0.0f)) *
                       mat4x4_scale(_v3(button.Size, 0.0f));

  SetUniform(context.UIShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.UIShader.ViewProjection, &uiContext.ViewProjection, 1);

  v2 offset = _v2(0.0f);

  v3 objectColor = _v3(1.0f);
  if (uiContext.FocusedId == button.Id) {
    objectColor = _v3(0.0f, 1.0f, 0.0f);
  }
  if (uiContext.HotId == button.Id) {
    objectColor = _v3(1.0f, 0.0f, 0.0f);
    offset.X = 0.5f;
  }
  if (uiContext.ActivatedId == button.Id) {
    objectColor = _v3(0.0f, 0.0f, 1.0f);
  }

  SetUniform(context.UIShader.TextureCoordOffset, &offset, 1);
  SetSamplerUniform(context.UIShader.TextureDiffuse1, 0);
  // SetUniform(context.UIShader.ObjectColor, &objectColor, 1);
  glBindVertexArray(BindUIRect(context));
  glBindTexture(GL_TEXTURE_2D, image->RenderId);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static void RenderToggle(const UISystem::ui_toggle& toggle,
                         render_context context)
{
  UISystem::ui_context& uiContext = *toggle.Context;

  renderable* image =
    (renderable*)get(*context.RenderableStore, (uintptr_t)toggle.Image);
  if (image->RenderId == HOKI_OGL_NO_ID) {
    image->RenderId = BindUITexture(*toggle.Image);
  }

  mat4x4 modelMatrix = IDENTITY_MATRIX *
                       mat4x4_translate(_v3(toggle.Position, 0.0f)) *
                       mat4x4_scale(_v3(toggle.Size, 0.0f));

  SetUniform(context.UIShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.UIShader.ViewProjection, &uiContext.ViewProjection, 1);

  v2 offset = _v2(0.0f);
  if (uiContext.HotId == toggle.Id || *toggle.On) {
    offset.X = 0.5f;
  }
  SetUniform(context.UIShader.TextureCoordOffset, &offset, 1);
  SetSamplerUniform(context.UIShader.TextureDiffuse1, 0);
  // SetUniform(context.UIShader.ObjectColor, &objectColor, 1);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(BindUIRect(context));
  glBindTexture(GL_TEXTURE_2D, image->RenderId);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static void RenderIcon(const UISystem::ui_icon& icon, render_context& context)
{
  UISystem::ui_context uiContext = *icon.Context;

  renderable* image =
    (renderable*)get(*context.RenderableStore, (uintptr_t)icon.Image);

  if (image->RenderId == HOKI_OGL_NO_ID) {
    image->RenderId = BindUITexture(*icon.Image);
  }

  v2 offset = hadamard_multiply(icon.Size, icon.Origin);
  v3 position = _v3(icon.Position - offset, 0.0f);
  mat4x4 modelMatrix = IDENTITY_MATRIX * mat4x4_translate(position) *
                       mat4x4_scale(_v3(icon.Size, 0.0f)) *
                       mat4x4_rotate(_v3(0.0f, 0.0f, icon.Angle));

  v2 noOffset = _v2(0.0f);
  SetUniform(context.UIShader.TextureCoordOffset, &noOffset, 1);
  SetUniform(context.UIShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.UIShader.ViewProjection, &uiContext.ViewProjection, 1);

  SetSamplerUniform(context.UIShader.TextureDiffuse1, 0);

  glBindVertexArray(BindRect(context));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, image->RenderId);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static void RenderJoystick(const UISystem::ui_joystick& joystick,
                           render_context context)
{
  UISystem::ui_context& uiContext = *joystick.Context;

  renderable* handle =
    (renderable*)get(*context.RenderableStore, (uintptr_t)joystick.Handle);
  renderable* background =
    (renderable*)get(*context.RenderableStore, (uintptr_t)joystick.Background);

  if (handle->RenderId == HOKI_OGL_NO_ID) {
    handle->RenderId = BindUITexture(*joystick.Handle);
    background->RenderId = BindUITexture(*joystick.Background);
  }

  mat4x4 modelMatrix = IDENTITY_MATRIX *
                       mat4x4_translate(_v3(joystick.Position, 0.0f)) *
                       mat4x4_scale(_v3(joystick.BackgroundSize, 0.0f));
  SetUniform(context.UIShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.UIShader.ViewProjection, &uiContext.ViewProjection, 1);

  SetSamplerUniform(context.UIShader.TextureDiffuse1, 0);

  SetUniform(context.UIShader.TextureCoordOffset, &V2_ZERO, 1);

  glBindVertexArray(BindUIRect(context));
  glBindTexture(GL_TEXTURE_2D, background->RenderId);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  float angle = (float)rad_to_deg(
    atan2(joystick.ControlPosition->X, -joystick.ControlPosition->Y));

  v2 offset = _v2(0.0f, 0.0f);
  const float deadzone = 0.2f;
  bool joystickOverDeadzone = fabs(joystick.ControlPosition->X) > deadzone ||
                              fabs(joystick.ControlPosition->Y) > deadzone;
  if (uiContext.HotId == joystick.Id && joystickOverDeadzone) {
    offset.X = 0.5f;
  }
  SetUniform(context.UIShader.TextureCoordOffset, &offset, 1);

  v2 centerForRotation = _v2(-0.5f);
  modelMatrix = IDENTITY_MATRIX *
                mat4x4_translate(_v3(
                  joystick.Position +
                    ((joystick.BackgroundSize - joystick.HandleSize) * 0.5f),
                  0.0f)) *
                mat4x4_scale(_v3(joystick.HandleSize, 0.0f)) *
                mat4x4_translate(_v3(-centerForRotation, 0.0f)) *
                mat4x4_rotate(_v3(0.0f, 0.0f, angle)) *
                mat4x4_translate(_v3(centerForRotation, 0.0f));
  SetUniform(context.UIShader.ModelMatrix, &modelMatrix, 1);

  glBindTexture(GL_TEXTURE_2D, handle->RenderId);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static void RenderSlider(const UISystem::ui_slider& slider,
                         render_context context)
{
  UISystem::ui_context& uiContext = *slider.Context;

  renderable* handle =
    (renderable*)get(*context.RenderableStore, (uintptr_t)slider.Handle);
  renderable* background =
    (renderable*)get(*context.RenderableStore, (uintptr_t)slider.Background);

  if (handle->RenderId == HOKI_OGL_NO_ID) {
    handle->RenderId = BindUITexture(*slider.Handle);
    background->RenderId = BindUITexture(*slider.Background);
  }

  mat4x4 modelMatrix = IDENTITY_MATRIX *
                       mat4x4_translate(_v3(slider.Position, 0.0f)) *
                       mat4x4_scale(_v3(slider.BackgroundSize, 0.0f));
  SetUniform(context.UIShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.UIShader.ViewProjection, &uiContext.ViewProjection, 1);

  SetSamplerUniform(context.UIShader.TextureDiffuse1, 0);
  SetUniform(context.UIShader.TextureCoordOffset, &V2_ZERO, 1);

  glBindVertexArray(BindRect(context));
  glBindTexture(GL_TEXTURE_2D, background->RenderId);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  v2 offset = _v2(0.0f);
  if (uiContext.HotId == slider.Id) {
    offset.X = 0.5f;
  }
  SetUniform(context.UIShader.TextureCoordOffset, &offset, 1);

  float valPercent = (*slider.Value + slider.Min) / slider.Max;
  v3 position = _v3(slider.Position.X - (slider.HandleSize.X * 0.5f) +
                      valPercent * (slider.BackgroundSize.X),
                    slider.Position.Y,
                    0.0f);

  modelMatrix = IDENTITY_MATRIX * mat4x4_translate(position) *
                mat4x4_scale(_v3(slider.HandleSize, 0.0f));
  SetUniform(context.UIShader.ModelMatrix, &modelMatrix, 1);

  glBindVertexArray(BindUIRect(context));
  glBindTexture(GL_TEXTURE_2D, handle->RenderId);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
