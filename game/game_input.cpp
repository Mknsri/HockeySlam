#include "game_input.h"

void handle_input(game_input_buffer& inputBuffer,
                  game_state_command_buffer& commandBuffer)
{
  while (inputBuffer.InputCount > 0) {
    game_input* input = &inputBuffer.Inputs[--inputBuffer.InputCount];
    game_input_code inputCode = input->State.Code;
    uint32_t inputState = input->State.Flags;

    switch (inputCode) {
      case INPUT_CODE_CURSOR_CLICK:
        if (inputState & INPUT_STATE_CHANGED) {
          if (inputState & INPUT_STATE_IS_DOWN) {
            push_state_command(
              commandBuffer, COMMAND_PRIMARY_INPUT_START, input->Position);
          } else {
            push_state_command(commandBuffer, COMMAND_PRIMARY_INPUT_END);
          }
        }
        break;

      case INPUT_CODE_CURSOR_MOVE:
        push_state_command(
          commandBuffer, COMMAND_PRIMARY_INPUT, input->Position);
        break;

      case INPUT_CODE_UP:
      case INPUT_CODE_DOWN:
      case INPUT_CODE_LEFT:
      case INPUT_CODE_RIGHT:
        push_state_command(commandBuffer, COMMAND_MOVE_CAMERA, input->State);
        break;

      case INPUT_CODE_NUM_1:
        push_state_command(commandBuffer, COMMAND_BUILD_SHADERS);
        break;

      case INPUT_CODE_NUM_2:
        push_state_command(commandBuffer, COMMAND_RESET_GAME);
        break;

      case INPUT_CODE_NUM_3:
#if HOKI_DEV
        if (inputState == (INPUT_STATE_IS_DOWN | INPUT_STATE_CHANGED)) {
          push_state_command(commandBuffer, DEBUG_COMMAND_TOGGLE_DEBUG_CAMERA);
        }
#endif
        break;

      case INPUT_CODE_NUM_4:
#if HOKI_DEV
        if (inputState == (INPUT_STATE_IS_UP | INPUT_STATE_CHANGED)) {
          push_state_command(commandBuffer, DEBUG_COMMAND_TOGGLE_MOUSEPICKER);
        }
#endif
        break;

      case INPUT_CODE_NUM_5:
#if HOKI_DEV
        if (inputState == (INPUT_STATE_IS_UP | INPUT_STATE_CHANGED)) {
          push_state_command(commandBuffer, DEBUG_COMMAND_TOGGLE_DEBUG_UI);
        }
#endif
        break;

      case INPUT_CODE_NO_OP:
        break;

      default:
        HOKI_ASSERT(false);
        break;
    }
  }
}