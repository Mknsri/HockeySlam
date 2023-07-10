#include "../game/ui_system.h"

using Asset::glyph;
using UISystem::ui_text;

static uint32_t BindCharacterTexture(glyph& character, int characterCode)
{
  GLuint textureId;
  // Generate texture
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
#if HOKI_DEV && !GL_ES_VERSION_3_0
  // apply the name, -1 means NULL terminated
  char buffer[50];
  sprintf_s(buffer, sizeof(buffer), "char_%c", characterCode);
  glObjectLabel(GL_TEXTURE, textureId, -1, buffer);
#endif
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_R8,
               (uint32_t)character.PixelSize.X,
               (uint32_t)character.PixelSize.Y,
               0,
               GL_RED,
               GL_UNSIGNED_BYTE,
               character.Memory);

  // Set texture options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return textureId;
}

static bool HandleSpecial(const int character,
                          const v2 scaledBearing,
                          const float originOffset,
                          const v2 textOrigin,
                          const float advance,
                          v2* offset)
{
  switch (character) {
    case '\n':
      offset->X = textOrigin.X;
      offset->Y += originOffset;

      return true;

    case ' ':
      offset->X += advance;
      return true;

    case '\t': {
      float tabSize = (advance * 5.0f);
      float nextTabStop = fmod(offset->X + tabSize, tabSize);
      offset->X += tabSize - nextTabStop;
    }
      return true;

    default:
      return false;
  }
}

static void RenderText(const UISystem::ui_text* text,
                       const game_window_info windowInfo,
                       render_context& context)
{
  const UISystem::ui_context* uiContext = text->Context;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

  v3 color = _v3(1.0f, 1.0f, 0.0f);
  SetUniform(context.TextShader.ViewProjection, &uiContext->ViewProjection, 1);

  float fontSize = ceilf(text->PixelHeight / 10.0f) * 10.0f;
  float fontScale = text->PixelHeight / fontSize;
  v2 screenScale =
    _v2(1.0f / (float)windowInfo.Width, 1.0f / (float)windowInfo.Height);
  v2 textPos = text->Position;
  float scaledOffset = text->OriginOffset * screenScale.Y;

  uint32_t* currentCharacterEntry = text->CodepointData.Codepoints;
  while (*currentCharacterEntry) {
    uint32_t codepoint = *currentCharacterEntry;
    currentCharacterEntry++;

    glyph* info = (glyph*)get(text->GlyphTable->Glyphs, (uintptr_t)codepoint);
#if HOKI_DEV
    HOKI_ASSERT(info->DEBUGCodepoint == codepoint);
#endif
    v2 scaledBearing =
      hadamard_multiply(info->PixelBearing * fontScale, screenScale);

    // Special characters
    v2 specialOffset = _v2(0.0f);
    if (HandleSpecial(codepoint,
                      scaledBearing,
                      scaledOffset,
                      text->Position,
                      info->PixelAdvance * screenScale.X,
                      &textPos)) {
      continue;
    }

    GLuint rectVAO = BindRect(context);

    renderable* glyphTexture =
      (renderable*)get(*context.RenderableStore, (uintptr_t)info);
    if (glyphTexture->RenderId == HOKI_OGL_NO_ID) {
      glyphTexture->RenderId = BindCharacterTexture(*info, codepoint);
    }
    glBindVertexArray(rectVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glyphTexture->RenderId);
    SetSamplerUniform(context.TextShader.CharacterTexture, 0);

    v2 charPos = textPos + scaledBearing;
    charPos.Y += scaledOffset;

    // Shadow
    SetUniform(context.TextShader.TextColor, &COLOR_BLACK, 1);
    mat4x4 modelMatrix =
      IDENTITY_MATRIX * mat4x4_translate(_v3(charPos + screenScale, 0.0f)) *
      mat4x4_scale(
        _v3(hadamard_multiply(info->PixelSize * fontScale, screenScale), 0.0f));
    SetUniform(context.TextShader.ModelMatrix, &modelMatrix, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Text
    SetUniform(context.TextShader.TextColor, &color, 1);
    // Optimization: Move the shadow model matrix to draw the actual text
    modelMatrix.M[3][0] = modelMatrix.M[3][0] - screenScale.X;
    modelMatrix.M[3][1] = modelMatrix.M[3][1] - screenScale.Y;
    SetUniform(context.TextShader.ModelMatrix, &modelMatrix, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    textPos.X += info->PixelAdvance * fontScale * screenScale.X;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}
