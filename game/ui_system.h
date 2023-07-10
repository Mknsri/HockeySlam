#ifndef UI_SYSTEM_H
#define UI_SYSTEM_H

struct render_context;

namespace UISystem {

const size_t MAX_TEXT_LENGTH = sizeof(uint32_t) * 1024;

struct ui_context
{
  Asset::game_assets* Assets;

  size_t FocusedId;
  size_t HotId;
  size_t ActivatedId;

  bool WentDown;
  bool IsDown;
  bool WentUp;
  v2 TouchPosition;

  bool StackMode;
  float StackX;

  mat4x4 ViewProjection;
  v2 AspectScale;

  render_context* RenderContext;
  uint8_t* Items;
  size_t ItemCursor;
  size_t ItemsMaxBytes;

  size_t StringCursor;
  uint32_t Strings[MAX_TEXT_LENGTH];
  float TopMargin; // To Android cutout
};

struct ui_button
{
  size_t Id;
  v2 Position;
  v2 Size;
  Asset::texture* Image;

  ui_context* Context;
};

struct ui_toggle
{
  size_t Id;
  v2 Position;
  v2 Size;
  Asset::texture* Image;

  bool* On;
  ui_context* Context;
};

struct ui_joystick
{
  size_t Id;
  v2 Position;
  v2 HandleSize;
  v2 BackgroundSize;
  Asset::texture* Handle;
  Asset::texture* Background;

  v2* ControlPosition;
  ui_context* Context;
};

struct ui_icon
{
  size_t Id;
  v2 Origin;
  v2 Position;
  v2 Size;
  float Angle;
  const Asset::texture* Image;

  ui_context* Context;
};

struct ui_text
{
  v2 Position;
  float PixelHeight;
  float OriginOffset;
  Asset::text_codepoint_data CodepointData;
  Asset::glyph_table* GlyphTable;
  const Asset::font* Font;
  const ui_context* Context;
};

struct ui_slider
{
  size_t Id;
  v2 Position;
  v2 HandleSize;
  v2 BackgroundSize;
  Asset::texture* Background;
  Asset::texture* Handle;

  float* Value;
  float Min;
  float Max;
  ui_context* Context;
};

#define UI_EASING_FUNCTION(name) float name(float t)
typedef UI_EASING_FUNCTION(ui_easing_function);
UI_EASING_FUNCTION(Linear)
{
  return t;
}

} // namespace UI

#endif // UI_SYSTEM_H