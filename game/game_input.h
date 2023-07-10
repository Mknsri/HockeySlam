#ifndef GAME_INPUT_H
#define GAME_INPUT_H

enum game_input_code
{
  INPUT_CODE_NO_OP,
  INPUT_CODE_CURSOR_CLICK,
  INPUT_CODE_CURSOR_MOVE,

  INPUT_CODE_UP,
  INPUT_CODE_DOWN,
  INPUT_CODE_LEFT,
  INPUT_CODE_RIGHT,

  // DEBUG
  INPUT_CODE_NUM_1,
  INPUT_CODE_NUM_2,
  INPUT_CODE_NUM_3,
  INPUT_CODE_NUM_4,
  INPUT_CODE_NUM_5,

  INPUT_CODE_R // Record
};

enum game_input_state_flags
{
  INPUT_STATE_NO_STATE = 0x0,
  INPUT_STATE_IS_DOWN = 0x1,
  INPUT_STATE_IS_UP = 0x2,
  INPUT_STATE_CHANGED = 0x4,
  INPUT_IS_ABSOLUTE = 0x8
};

struct game_input_state
{
  game_input_code Code;
  uint32_t Flags;
};

struct game_input
{
  game_input_state State;
  v2 Position;
};

struct game_input_buffer
{
  game_input* Inputs;
  uint32_t InputCount;
  uint32_t InputIndex;
};

#endif // GAME_INPUT_H