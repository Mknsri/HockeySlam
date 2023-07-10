#include "ui_system.h"

#include "debug/debug_strings.cpp"

// TODO: Refactor so this isn't needed
float get_aim_power(const game_state& state);

namespace UISystem {
ui_context create_context(size_t itemCount, Asset::game_assets* assets)
{
  ui_context result = { 0 };
  result.TopMargin = 0.08f;
  result.ItemCursor = 0;
  result.ItemsMaxBytes = 512 * sizeof(uint8_t*);
  result.Items = (uint8_t*)allocate_t(result.ItemsMaxBytes);
  result.StringCursor = 0;
  result.TouchPosition = _v2(0.5f);
  result.Assets = assets;

  return result;
}

void reset_context(ui_context* context, render_context& renderContext)
{
  context->ItemCursor = 0;
  // Resolve hot id before focused id so it keeps it after touchUp
  context->HotId = context->FocusedId != SIZE_MAX ? context->HotId : SIZE_MAX;
  context->FocusedId = context->IsDown ? context->FocusedId : SIZE_MAX;
  context->ActivatedId = SIZE_MAX;
  context->StringCursor = 0;
  context->RenderContext = &renderContext;

  context->WentDown = false;
  context->WentUp = false;
  push_setup_ui_context(renderContext, context);
}

uint8_t* get_ui_item(ui_context* context, const size_t itemSize)
{
  uint8_t* itemCursor = context->Items + context->ItemCursor;
  context->ItemCursor += itemSize;
  HOKI_ASSERT(context->ItemCursor < context->ItemsMaxBytes);

  return itemCursor;
}

bool hits(const v2 point, const v2 position, const v2 size)
{
  bool xHits = (point.X > position.X) && (point.X < position.X + size.X);
  bool yHits = (point.Y > position.Y) && (point.Y < position.Y + size.Y);
  return xHits && yHits;
}

v2 correct_texture_size_to_ui_scale(const Asset::texture* texture,
                                    const int frameCount,
                                    const v2& uiScale,
                                    const v2 uiAspectRatio)
{
  v2 scale =
    _v2(texture->Image->PixelSize.X / frameCount, texture->Image->PixelSize.Y);
  if (scale.X > scale.Y) {
    scale.Y = scale.Y / scale.X;
    scale.X = 1.0f;
  } else {
    scale.X = scale.X / scale.Y;
    scale.Y = 1.0f;
  }

  return hadamard_multiply(hadamard_multiply(scale, uiScale), uiAspectRatio);
}

bool update_activation(ui_context* context, const size_t id)
{
  bool touchDown = context->IsDown;
  bool touchUp = context->WentUp;
  bool nothingHot = context->HotId == SIZE_MAX;
  bool nothingActivated = context->ActivatedId == SIZE_MAX;
  bool nothingFocused = context->FocusedId == SIZE_MAX;

  if (context->WentDown && nothingFocused) {
    context->FocusedId = id;
  }
  bool isFocused = context->FocusedId == id;

  if (touchDown && isFocused) {
    context->HotId = id;
  }
  bool isHot = context->HotId == id;

  if (isHot && touchUp && nothingActivated) {
    context->ActivatedId = id;
    return true;
  }

  return false;
}

bool do_button(ui_context* context,
               v2 pos,
               v2 size,
               Asset::texture* texture = nullptr)
{
  ui_button* button = (ui_button*)get_ui_item(context, sizeof(ui_button));
  *button = {};
  button->Id = (uint8_t*)button - context->Items;
  button->Position = pos;
  button->Context = context;
  button->Image =
    texture == nullptr ? &context->Assets->ButtonTexture : texture;
  button->Size = correct_texture_size_to_ui_scale(
    button->Image, 2, size, context->AspectScale);

  if (context->StackMode) {
    button->Position.X = context->StackX;
    context->StackX += button->Size.X;
  }

  bool activated = false;
  if (hits(context->TouchPosition, button->Position, button->Size)) {
    activated = update_activation(context, button->Id);
  }

  add_rendercommand(*context->RenderContext, button);

  return activated;
}

bool do_toggle(ui_context* context,
               v2 pos,
               v2 size,
               bool& value,
               Asset::texture* texture = nullptr)
{
  ui_toggle* toggle = (ui_toggle*)get_ui_item(context, sizeof(ui_toggle));
  *toggle = {};
  toggle->Id = (uint8_t*)toggle - context->Items;
  toggle->Position = pos;
  toggle->Context = context;
  toggle->Image =
    texture != nullptr ? texture : &context->Assets->ToggleTexture;
  toggle->On = &value;
  toggle->Size = correct_texture_size_to_ui_scale(
    toggle->Image, 2, size, context->AspectScale);

  if (context->StackMode) {
    toggle->Position.X = context->StackX;
    context->StackX += toggle->Size.X;
  }

  bool activated = false;
  if (hits(context->TouchPosition, toggle->Position, toggle->Size)) {
    activated = update_activation(context, toggle->Id);
    if (activated) {
      value = !value;
    }
  }

  add_rendercommand(*context->RenderContext, toggle);

  return activated;
}

void do_sprite(ui_context* context,
               const texture* texture,
               const v2 position,
               const v2 size,
               const float angle = 0.0f)
{
  ui_icon* icon = (ui_icon*)get_ui_item(context, sizeof(ui_icon));
  *icon = {};
  icon->Id = (uint8_t*)icon - context->Items;
  icon->Position = position;
  icon->Image = texture;
  icon->Context = context;
  icon->Angle = angle;
  icon->Size = correct_texture_size_to_ui_scale(
    icon->Image, 1, size, context->AspectScale);
  if (context->StackMode) {
    icon->Position.X = context->StackX;
    context->StackX += icon->Size.X;
  }

  push_render_icon(*context->RenderContext, icon);
}

void do_animated_sprite(ui_context* context,
                        const texture* texture,
                        const v2 size,
                        const v2 startPosition,
                        const v2 endPosition,
                        const double startTime,
                        const float duration,
                        const double currentTime,
                        ui_easing_function& easing = Linear)
{
  if (currentTime < startTime) {
    return;
  }

  float durationDone = (float)(currentTime - startTime) / duration; // Linear
  if (durationDone < 0.0f || durationDone > 1.0f) {
    return;
  }
  durationDone = easing(durationDone);

  ui_icon* icon = (ui_icon*)get_ui_item(context, sizeof(ui_icon));
  *icon = {};
  icon->Position = startPosition + (endPosition - startPosition) * durationDone;
  icon->Image = texture;
  icon->Origin = _v2(0.5f);
  icon->Context = context;
  icon->Size = correct_texture_size_to_ui_scale(
    icon->Image, 1, size, context->AspectScale);

  push_render_icon(*context->RenderContext, icon);
}

void do_scale_animated_sprite(ui_context* context,
                              const texture* texture,
                              const v2 position,
                              const v2 startSize,
                              const v2 endSize,
                              const double startTime,
                              const float duration,
                              const double currentTime,
                              ui_easing_function& easing = Linear)
{
  if (currentTime < startTime) {
    return;
  }

  float durationDone = (float)(currentTime - startTime) / duration; // Linear
  if (durationDone < 0.0f || durationDone > 1.0f) {
    return;
  }

  durationDone = easing(durationDone);

  ui_icon* icon = (ui_icon*)get_ui_item(context, sizeof(ui_icon));
  *icon = {};
  icon->Position = position;
  icon->Image = texture;
  icon->Context = context;
  icon->Origin = _v2(0.5f);
  v2 size = startSize + (endSize - startSize) * durationDone;
  icon->Size = correct_texture_size_to_ui_scale(
    icon->Image, 1, size, context->AspectScale);

  push_render_icon(*context->RenderContext, icon);
}

void do_text(const v2 pos,
             const float pixelHeight,
             ui_context* context,
             const Asset::font& font,
             const char* message,
             ...)
{
  ui_text* text = (ui_text*)get_ui_item(context, sizeof(ui_text));
  *text = {};
  text->Font = &font;
  text->PixelHeight = pixelHeight;
  text->Position = pos;
  text->Context = context;
  text->OriginOffset = Asset::get_origin_offset_pixels(pixelHeight, font);

  va_list messageArgs;
  va_start(messageArgs, message);
  size_t maxLen = (context->Strings + MAX_TEXT_LENGTH) -
                  (context->Strings + context->StringCursor);
  size_t strBytes = vsnprintf((char*)&context->Strings[context->StringCursor],
                              maxLen,
                              message,
                              messageArgs);
  HOKI_ASSERT(strBytes > 0);
  va_end(messageArgs);

  text->GlyphTable = Asset::get_glyph_table(font, text->PixelHeight);
  text->CodepointData =
    Asset::create_codepoint_data(font,
                                 text->PixelHeight,
                                 context->Strings + context->StringCursor,
                                 strBytes);
  context->StringCursor +=
    text->CodepointData.CodepointCount * sizeof(uint32_t);

  HOKI_ASSERT(context->StringCursor < MAX_TEXT_LENGTH);
  rendercommand_text(*context->RenderContext, text);
}

bool do_joystick(ui_context* context,
                 const v2 pos,
                 const v2 backgroundSize,
                 const v2 handleSize,
                 v2* joyPos)
{
  ui_joystick* joystick =
    (ui_joystick*)get_ui_item(context, sizeof(ui_joystick));
  *joystick = {};
  joystick->Id = (uint8_t*)joystick - context->Items;
  joystick->Position = pos;
  joystick->Context = context;
  joystick->Background = &context->Assets->JoystickBgTexture;
  joystick->BackgroundSize = correct_texture_size_to_ui_scale(
    joystick->Background, 2, backgroundSize, context->AspectScale);
  joystick->Handle = &context->Assets->JoystickHandleTexture;
  joystick->HandleSize = correct_texture_size_to_ui_scale(
    joystick->Handle, 2, handleSize, context->AspectScale);
  joystick->ControlPosition = joyPos;

  bool activated = false;
  if (hits(context->TouchPosition, joystick->Position, joystick->HandleSize)) {
    bool nothingFocused = context->FocusedId == SIZE_MAX;
    if (context->WentDown && nothingFocused) {
      context->FocusedId = joystick->Id;
    }
    bool isFocused = context->FocusedId == joystick->Id;

    if (context->WentDown && isFocused) {
      context->HotId = joystick->Id;
      activated = true;
    }
  }

  if (activated || context->HotId == joystick->Id) {
    v2 joyCenter = joystick->Position + joystick->BackgroundSize * 0.5f;
    *joyPos = hadamard_division((context->TouchPosition - joyCenter),
                                joystick->BackgroundSize);
    joyPos->X = clamp(joyPos->X, -0.5f, 0.5f);
    joyPos->Y = clamp(joyPos->Y, -0.5f, 0.5f);
  }

  add_rendercommand(*context->RenderContext, joystick);

  return activated;
}

bool do_slider(ui_context* context,
               const v2 pos,
               const v2 handleSize,
               const v2 backgroundSize,
               float& value,
               const float min,
               const float max)
{
  ui_slider* slider = (ui_slider*)get_ui_item(context, sizeof(ui_slider));
  *slider = {};
  slider->Id = (uint8_t*)slider - context->Items;
  slider->Position = pos;
  slider->Context = context;
  slider->Handle = &context->Assets->SliderHandleTexture;
  slider->Background = &context->Assets->SliderBgTexture;
  slider->Value = &value;
  slider->Min = min;
  slider->Max = max;
  slider->BackgroundSize = correct_texture_size_to_ui_scale(
    slider->Background, 1, backgroundSize, context->AspectScale);
  slider->HandleSize = correct_texture_size_to_ui_scale(
    slider->Handle, 2, handleSize, context->AspectScale);

  bool activated = false;

  float start = slider->Position.X;
  float end = slider->BackgroundSize.X;
  float valPercent = value / (max - min);
  v2 handlePosition = _v2(start + (valPercent * end), slider->Position.Y);
  // Due to value <-> position conversion we have to -half-size the hitcheck on
  // X-axis
  v2 half = _v2(slider->HandleSize.X * 0.5f, 0.0f);
  if (context->HotId == slider->Id ||
      hits(context->TouchPosition, handlePosition - half, slider->HandleSize)) {
    activated = update_activation(context, slider->Id);
    if (context->HotId == slider->Id) {
      float screenPos = context->TouchPosition.X;
      valPercent = (screenPos - start) / (end);
      value = clamp(min + (valPercent * (max - min)), min, max);
    }
  }

  add_rendercommand(*context->RenderContext, slider);

  return activated;
}

bool do_draggable(ui_context* context,
                  const texture* texture,
                  const v2 position,
                  const v2 size)
{
  ui_icon* draggable = (ui_icon*)get_ui_item(context, sizeof(ui_icon));
  *draggable = {};
  draggable->Id = (uint8_t*)draggable - context->Items;
  draggable->Position = position;
  draggable->Image = texture;
  draggable->Context = context;
  draggable->Size = correct_texture_size_to_ui_scale(
    draggable->Image, 1, size, context->AspectScale);

  bool activated = false;
  if (context->HotId == draggable->Id ||
      hits(context->TouchPosition, position, draggable->Size)) {
    bool nothingFocused = context->FocusedId == SIZE_MAX;
    if (context->WentDown && nothingFocused) {
      context->FocusedId = draggable->Id;
    }
    bool isFocused = context->FocusedId == draggable->Id;

    if (context->IsDown && isFocused) {
      context->HotId = draggable->Id;
      activated = true;
    }
  }

  if (activated) {
    draggable->Position = context->TouchPosition - draggable->Position;
  }

  push_render_icon(*context->RenderContext, draggable);

  return activated;
}

void do_debug_ui(game_state& state, ui_context* context)
{
  v2 mPos = state.AimPosition;
  do_text(
    _v2(0.0f, -context->TopMargin),
    22.0f,
    context,
    state.Assets->TestFont,
    "Time:%f (sim %f) (fps: %f) \tmem: %X\tmem_t: %X\n"
    "StringCursor: %X\t\n"
    "goalie state:%s\t nextstate:%s\t slide: %f\n"
    "aimpower: %f \n"
    "game phase:%s\n"
#if HOKI_DEV
    "debug camera(%i) pos:%f %f %f\n"
#endif
    "mPos: %f:%f\n"
    "touch: %f:%f\n - %d:%d\n"
    "ai movementAmoun %f",
    state.RealTime,
    state.SimTime,
    1.0f / state.FrameDelta,
    (permanentMemoryPool->Begin + (size_t)permanentMemoryPool->CursorOffset),
    (transientMemoryPool->Begin + (size_t)transientMemoryPool->CursorOffset),
    (&context->Strings - context->StringCursor),
    debug_to_string(state.AIGoalie.State),
    debug_to_string(state.AIGoalie.NextState),
    state.AIGoalie.MovementAmount,
    get_aim_power(state),
    debug_to_string(state.Phase),
#if HOKI_DEV
    state.DebugCameraActive,
    state.DebugCamera.Position.X,
    state.DebugCamera.Position.Y,
    state.DebugCamera.Position.Z,
#endif
    mPos.X,
    mPos.Y,
    context->TouchPosition.X,
    context->TouchPosition.Y,
    context->WentDown,
    context->WentUp,
    state.AIGoalie.MovementAmount);

  do_sprite(
    context, &state.Assets->CursorTexture, context->TouchPosition, _v2(0.1f));

#if 0
  v2 normalized = get_aim_power_dir(state);
  float angle = rad_to_deg(std::atan2(normalized.Y, normalized.X)) - 90.0f;
  do_sprite(context,
            &state.Assets->DebugDirectionTexture,
            _v2(0.5f, 0.45f),
            _v2(0.05f),
            angle);

  for (size_t i = 0; i < ARRAY_SIZE(state.AimPositionBuffer); i++) {
    do_sprite(context,
              &state.Assets->CursorTexture,
              state.AimPositionBuffer[i],
              _v2(0.05f));
  }
#endif
  if (do_toggle(context,
                _v2(0.85f, 0.45f),
                _v2(0.1f),
                context->RenderContext->HighQuality)) {
  }
}

void do_game_ui(game_state& state, ui_context* context)
{
  if (state.Phase == game_phase::PERF_TEST) {
    return;
  }

  int fails = state.GamesPlayed - state.Goals;

  texture* goalsTexture = &state.Assets->GoalCounterZeroGoal;
  texture* failsTexture = &state.Assets->GoalCounterZeroGoal;
  switch (state.Goals) {
    case 3:
      goalsTexture = &state.Assets->GoalCounterThreeGoal;
      break;
    case 2:
      goalsTexture = &state.Assets->GoalCounterTwoGoal;
      break;
    case 1:
      goalsTexture = &state.Assets->GoalCounterOneGoal;
      break;

    case 0:
    default:
      break;
  }
  switch (fails) {
    case 3:
      failsTexture = &state.Assets->GoalCounterThreeGoal;
      break;
    case 2:
      failsTexture = &state.Assets->GoalCounterTwoGoal;
      break;
    case 1:
      failsTexture = &state.Assets->GoalCounterOneGoal;
      break;

    case 0:
    default:
      break;
  }

  v2 bbpos = _v2(0.65f, 0.0f);
  v2 bbsize =
    correct_texture_size_to_ui_scale(&state.Assets->GoalBackboardTexture,
                                     1,
                                     _v2(0.45f),
                                     state.UIContext.AspectScale);
  if ((bbpos + bbsize).X > 1.0f) {
    bbpos.X -= bbpos.X + bbsize.X - 1.0f;
  }
  do_sprite(context, &state.Assets->GoalBackboardTexture, bbpos, _v2(0.45f));

  v2 pos = bbpos;
  pos += hadamard_multiply(_v2(0.149f, 0.28f), bbsize);
  do_sprite(context, goalsTexture, pos, _v2(0.042f));

  pos = bbpos;
  pos += hadamard_multiply(_v2(0.822f, 0.28f), bbsize);
  do_sprite(context, failsTexture, pos, _v2(0.042f));

  context->StackMode = true;
  const float goalCounterSize = 0.15f;
  context->StackX = 0.05f;
  if (do_button(context,
                _v2(0.0f, 0.02f),
                _v2(goalCounterSize),
                &state.Assets->GoalCounterResetButtonTexture)) {
    push_state_command(state.StateCommands, COMMAND_RESET_GAME);
  }
  do_toggle(context,
            _v2(0.05f, 0.02f),
            _v2(goalCounterSize * 1.8f),
            state.SoundEnabled,
            &state.Assets->SoundToggleTexture);
  context->StackMode = false;

#if 1
  if (do_joystick(
        context, _v2(0.4f, 0.75f), _v2(0.2f), _v2(0.18f), &state.AimPosition)) {
    push_state_command(state.StateCommands, COMMAND_ACTIVATE_JOYSTICK);
  }
#else
  if (do_button(context, _v2(0.4f, 0.75f), _v2(0.2f))) {
    push_state_command(state.StateCommands, COMMAND_ACTIVATE_JOYSTICK);
  }
#endif
}

void update_input_pos_delta(ui_context* context, v2 deltaPos)
{
  context->TouchPosition += deltaPos;
  context->TouchPosition.X = clamp(context->TouchPosition.X, 0.0f, 1.0f);
  context->TouchPosition.Y = clamp(context->TouchPosition.Y, 0.0f, 1.0f);
}

void update_input_pos_absolute(ui_context* context, v2 pos)
{
  pos.Y -= context->TopMargin;
  context->TouchPosition.X = clamp(pos.X, 0.0f, 1.0f);
  context->TouchPosition.Y = clamp(pos.Y, 0.0f, 1.0f);
}

void update_input_went_down(ui_context* context, bool wentDown)
{
  context->WentDown = wentDown;
  context->WentUp = !wentDown;
  if (context->WentDown) {
    context->IsDown = true;
  }
  if (context->WentUp) {
    context->IsDown = false;
  }
}

}
