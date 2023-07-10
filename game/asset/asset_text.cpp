#include "asset_text.h"

#include <map>
#include <cstdio>
#include <cstdarg>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x, u) allocate_t(x)
#define STBTT_free(x, u) unallocate_t(x)
#include "../../3rdparty/stb_truetype.h"

namespace Asset {

font initializeFont(file& fontFile)
{
  font result = {};
  stbtt_fontinfo* fontInfo =
    (stbtt_fontinfo*)allocate_t(sizeof(stbtt_fontinfo));

  stbtt_InitFont(fontInfo,
                 (uint8_t*)fontFile.Memory,
                 stbtt_GetFontOffsetForIndex((uint8_t*)fontFile.Memory, 0));
  result.GlyphTableStore = create_hash_table(16);

  result.FontInfo = fontInfo;
  return result;
}

glyph_table* create_glyph_table()
{
  glyph_table* result = (glyph_table*)allocate_t(sizeof(glyph_table));
  *result = {};
  result->Glyphs = create_hash_table(1024);

  return result;
}

glyph_table* get_glyph_table(const font& font, const float pixelHeight)
{
  // Ceil to closest 10
  float ceiledHeight = ceilf(pixelHeight / 10.0f) * 10.0f;
  size_t key = (size_t)ceiledHeight;

  glyph_table* table = (glyph_table*)get(font.GlyphTableStore, key);
  if (table == nullptr) {
    table = create_glyph_table();
    insert(font.GlyphTableStore, key, table);
  }

  return table;
}

inline uint8_t get_char_byte_count(const char character)
{
  if ((character & 0x80) == 0) {
    return 1;
  }

  if ((character & 0xF0) == 0xF0) {
    return 4;
  }

  if ((character & 0xE0) == 0xE0) {
    return 3;
  }

  return 2; // 0xC0
}

inline uint8_t get_mask_for_char(const char character)
{
  if ((character & 0x80) == 0) { // 0xxx
    return 0x7F;
  }

  if ((character & 0xC0) == 0x80) { // 10xx
    return 0x3F;
  }

  if ((character & 0xF8) == 0xF0) { // 1111 0xxx
    return 0x07;
  }

  if ((character & 0xF0) == 0xE0) { // 1110 xxxx
    return 0x0F;
  }

  if ((character & 0xE0) == 0xC0) { // 110x xxxx
    return 0x1F;
  }

  HOKI_ASSERT(false);
  return 0;
}

inline uint8_t get_content_bits_count(const char character)
{
  switch (get_mask_for_char(character)) {
    case 0x7F:
      return 7;
    case 0x3F:
      return 6;
    case 0x07:
      return 3;
    case 0x0F:
      return 4;
    case 0x1F:
      return 5;
    case 0x00:
      return 0;
  }

  HOKI_ASSERT(false);
  return 0;
}

size_t text_length(const char* start)
{
  size_t result = 0;
  char* cursor = (char*)start;
  while (*cursor) {
    cursor += get_char_byte_count(cursor[0]);
    result++;
  }

  return result;
}

size_t text_size(const char* start)
{
  size_t result = 0;
  char* cursor = (char*)start;
  while (*cursor) {
    uint8_t bytes = get_char_byte_count(cursor[0]);
    cursor += bytes;
    result += bytes;
  }

  return result;
}

inline uint32_t bytes_to_codepoint(const uint8_t* characterStart)
{
  uint32_t result = 0;

  uint8_t byteCount = get_char_byte_count(*characterStart);
  uint8_t* characterCursor = (uint8_t*)characterStart;
  for (uint8_t i = 1; i <= byteCount; i++) {
    char nextCharacter = *characterCursor & get_mask_for_char(*characterCursor);
    result += nextCharacter;

    if (i < byteCount) {
      characterCursor++;
      result <<= get_content_bits_count(*characterCursor);
    }
  }

  return result;
}

glyph* createCharacter(const uint8_t* character,
                       const float fontSizePx,
                       const font& font)
{
  uint8_t byteCount = get_char_byte_count(*character);
  int wideCharacter = bytes_to_codepoint(character);

  int width, height, yOffset, xOffset, leftSideBearing, advance;
  stbtt_GetCodepointHMetrics(
    font.FontInfo, wideCharacter, &advance, &leftSideBearing);
  float scale = stbtt_ScaleForPixelHeight(font.FontInfo,
                                          (ceilf(fontSizePx / 10.0f) * 10.0f));

  uint8_t* bitmap = stbtt_GetCodepointBitmapSubpixel(font.FontInfo,
                                                     0,
                                                     scale,
                                                     0.5f,
                                                     0.0f,
                                                     wideCharacter,
                                                     &width,
                                                     &height,
                                                     &xOffset,
                                                     &yOffset);

  glyph* newCharacter = (glyph*)allocate_t(sizeof(glyph));
  newCharacter->ByteCount = byteCount;
  newCharacter->PixelSize = _v2((float)width, (float)height);
  newCharacter->PixelBearing = _v2((float)xOffset, (float)yOffset);
  newCharacter->PixelAdvance = (float)advance * scale;
  newCharacter->Memory = bitmap;

  return newCharacter;
}

float get_origin_offset_pixels(const float fontSizePx, const font& font)
{
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(font.FontInfo, &ascent, &descent, &lineGap);
  float scale = stbtt_ScaleForPixelHeight(font.FontInfo, fontSizePx);

  return (float)(ascent - descent + lineGap) * scale;
}

uintptr_t message_to_hash_key(const hash_table& table,
                              const uint8_t* buffer,
                              const size_t length)
{
  uintptr_t result = 0;
  for (size_t i = 0; i < length; i++) {
    result += buffer[i] * 31 ^ (length - i);
  }

  return result;
}

text_codepoint_data create_codepoint_data(const font& font,
                                          const float fontHeightPixels,
                                          uint32_t* stringCursor,
                                          const size_t strLenBytes)
{
  text_codepoint_data result = {};
  uint8_t* messageCursor = (uint8_t*)stringCursor;
  const size_t stringLen = text_length((char*)stringCursor);

  result.Codepoints = stringCursor + stringLen;
  result.CodepointCount = stringLen;
  size_t textLengthBytes = strLenBytes;

  glyph_table* glyphTable = get_glyph_table(font, fontHeightPixels);
  for (size_t i = 0; i < stringLen; i++) {
    uint32_t codepoint = bytes_to_codepoint(messageCursor);

    glyph* cachedCharacter = (glyph*)get(glyphTable->Glyphs, codepoint);
    if (cachedCharacter == NULL) {
      cachedCharacter = createCharacter(messageCursor, fontHeightPixels, font);
#if HOKI_DEV
      cachedCharacter->DEBUGCodepoint = codepoint;
#endif
      insert(glyphTable->Glyphs, (uintptr_t)codepoint, cachedCharacter);
    }

    messageCursor += cachedCharacter->ByteCount;
    result.Codepoints[i] = codepoint;
  }
  size_t readBuffer = messageCursor - (uint8_t*)stringCursor;
  size_t wroteBuffer = textLengthBytes;
  HOKI_ASSERT(readBuffer == wroteBuffer);

  result.Codepoints[stringLen] = 0;

  return result;
}
}
