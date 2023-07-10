#include "game_state.h"

STATE_ACTION(debug_move_camera)
{
#if HOKI_DEV
  if (state.DebugCameraActive) {
    game_input_state inputForCamera = cmd.InputState;
    update_camera_velocity(
      inputForCamera.Flags, inputForCamera.Code, state.DebugCamera);
  }
#endif
}

STATE_ACTION(set_wireframe)
{
#if 0
    render_command &wireframeRenderCommand = renderContext.Commands.Commands[renderContext.Commands.Count++];
    wireframeRenderCommand.Type = DEBUG_RENDER_COMMAND_TYPE_SET_WIREFRAME;
#endif
}

STATE_ACTION(build_shaders)
{
#if 0
  *state.Assets = Asset::load_assets(gameMemory);
  Asset::compile_shaders(renderContext, *state.Assets);
#endif
}

STATE_ACTION(toggle_debug_camera)
{
#if HOKI_DEV
  state.DebugCameraActive = !state.DebugCameraActive;
#endif
}

STATE_ACTION(toggle_mousepicker)
{
#if HOKI_DEV
  state.MousePickerActive = !state.MousePickerActive;
#endif
}

STATE_ACTION(debug_toggle_debug_ui)
{
#if HOKI_DEV
  state.ShowDebugUI = !state.ShowDebugUI;
#endif
}
