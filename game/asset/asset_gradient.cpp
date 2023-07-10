#include "asset_gradient.h"

namespace Asset {

void CreateGradient(game_window_info* windowInfo,
                    render_command* renderCommand,
                    const int xOffset,
                    const int yOffset)
{
  entity_gradient* gradient = (entity_gradient*)renderCommand->Memory;
  gradient->Width = windowInfo->Width;
  gradient->Height = windowInfo->Height;
  gradient->Pitch = windowInfo->Pitch;

  uint8_t* row = (uint8_t*)gradient->Memory;
  for (uint32_t y = 0; y < gradient->Height; y++) {
    uint32_t* pixel = (uint32_t*)row;
    for (uint32_t x = 0; x < gradient->Width; x++) {
      *pixel = ((uint8_t)(x + xOffset)) << 16 | (uint8_t)(y + yOffset) | 0x00;
      pixel++;
    }
    row += gradient->Pitch;
  }

  // renderCommand->Size = state->Asset.Size;
}
}