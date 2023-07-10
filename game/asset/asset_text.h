#ifndef ASSET_TEXT_H
#define ASSET_TEXT_H

#include "../game_collections.h"

struct stbtt_fontinfo;

namespace Asset {

struct font
{
  const stbtt_fontinfo* FontInfo;
  hash_table GlyphTableStore;
};

struct glyph
{
#if HOKI_DEV
  uint32_t DEBUGCodepoint;
#endif
  uint8_t ByteCount;
  v2 PixelSize;
  v2 PixelBearing;
  float PixelAdvance;
  void* Memory;
};

struct glyph_table
{
  hash_table Glyphs;
};

struct text_codepoint_data
{
  uint32_t* Codepoints;
  size_t CodepointCount;
};

}

#endif // ASSET_TEXT_H