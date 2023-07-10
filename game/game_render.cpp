#include "game_render.h"

void add_renderable(const hash_table& renderableCache,
                    const void* renderableData)
{
  HOKI_ASSERT(renderableData != nullptr);
  uintptr_t keyFromData = (uintptr_t)renderableData;
  if (get(renderableCache, keyFromData) == nullptr) {
    renderable* newRenderable = (renderable*)allocate_t(sizeof(renderable));
    *newRenderable = {};
    insert(renderableCache, keyFromData, newRenderable);
  }
}

void add_ui_renderable(const hash_table& renderableCache,
                       const void* renderableData)
{
  HOKI_ASSERT(renderableData != nullptr);
  uintptr_t keyFromData = (uintptr_t)renderableData;
  if (get(renderableCache, keyFromData) == nullptr) {
    renderable* newRenderable = (renderable*)allocate_t(sizeof(renderable));
    *newRenderable = {};
    insert(renderableCache, keyFromData, newRenderable);
  }
}

render_command& create_render_command(render_context& context)
{
  return context.Commands.Commands[context.Commands.Count++];
}

void push_render_initialize(render_context& context)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_INITIALIZE;
}

#if HOKI_DEV
void add_debug_rendercommand(render_context& context, const game_entity* entity)
{
  render_command& command = create_render_command(context);
  command.Type = DEBUG_RENDER_COMMAND_TYPE_BONES;
  command.Entity = entity;
  for (size_t i = 0; i < entity->Model->BoneCount; i++) {
    add_renderable(*context.RenderableStore, entity->Model->Bones + i);
  }
}

void add_debug_rendercommand(render_context& context,
                             const debug_render_line* line)
{
  render_command& command = create_render_command(context);
  command.Type = DEBUG_RENDER_COMMAND_TYPE_LINE;
  command.Line = line;
}

void add_debug_rendercommand(render_context& context,
                             const PhysicsSystem::body* body)
{
  render_command& command = create_render_command(context);
  command.Type = DEBUG_RENDER_COMMAND_TYPE_PHYSICS_BODY;
  command.Body = body;
}

void add_debug_rendercommand(render_context& context,
                             const PhysicsSystem::space* space)
{
  for (size_t i = 0; i < space->BodyCount; i++) {
    add_debug_rendercommand(context, &space->Bodies[i]);
  }
}

void debug_push_render_light(render_context& context, const game_light* light)
{
  render_command& command = create_render_command(context);
  command.Type = DEBUG_RENDER_COMMAND_TYPE_LIGHT;
  command.Light = light;
}

void debug_push_render_cube(render_context& context, const v3* position)
{
  render_command& command = create_render_command(context);
  command.Type = DEBUG_RENDER_COMMAND_TYPE_CUBE;
  command.CubePos = position;
}
#else
void add_debug_rendercommand(render_context& context, const game_entity& entity)
{}
void add_debug_rendercommand(render_context& context,
                             const PhysicsSystem::body& body)
{}
void add_debug_rendercommand(render_context& context,
                             const PhysicsSystem::space& space)
{}
void add_debug_rendercommand(render_context& context,
                             const debug_render_line& line)
{}
void debug_push_render_light(render_context& context, const game_light* light)
{}

void debug_push_render_cuber(render_context& context, const v3 position) {}
#endif

void push_render_entity(render_context& context, const game_entity* entity)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_ENTITY;
  command.Entity = entity;

  add_renderable(*context.RenderableStore, entity->Model);
  for (size_t i = 0; i < entity->Model->TextureCount; i++) {
    add_renderable(*context.RenderableStore, entity->Model->Textures + i);
  }
  for (size_t i = 0; i < entity->Model->MaterialCount; i++) {
    if (entity->Model->Materials[i].Translucent) {
      render_command& translucentCommand = create_render_command(context);
      translucentCommand.Entity = entity;
      translucentCommand.Type = RENDER_COMMAND_TYPE_TRANSLUCENT;
      break;
    }
  }
}

void push_render_instanced(render_context& context,
                           const instanced_entity* entity)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_INSTANCED;
  command.InstancedEntity = entity;

  add_renderable(*context.RenderableStore, entity->Model);
  for (size_t i = 0; i < entity->Model->TextureCount; i++) {
    add_renderable(*context.RenderableStore, entity->Model->Textures + i);
  }
  for (size_t i = 0; i < entity->Model->MaterialCount; i++) {
    if (entity->Model->Materials[i].Translucent) {
      render_command& translucentCommand = create_render_command(context);
      translucentCommand.Entity = entity;
      translucentCommand.Type = RENDER_COMMAND_TYPE_TRANSLUCENT;
      break;
    }
  }
}

void push_render_update_camera(render_context& context,
                               const game_camera* camera)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_CAMERA;
  command.Camera = camera;
}

void add_rendercommand(render_context& context,
                       const ogl_shader_program& shaderProgram)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_CREATE_SHADER;
  command.ShaderProgram = &shaderProgram;
  add_renderable(*context.RenderableStore, shaderProgram.VertexShader);
  add_renderable(*context.RenderableStore, shaderProgram.FragmentShader);
}

void push_render_light(render_context& context, const game_light* light)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_LIGHT;
  command.Light = light;
}

void rendercommand_text(render_context& context, const UISystem::ui_text* text)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_TEXT;
  command.Text = text;
  for (uint32_t i = 0; i < text->CodepointData.CodepointCount; i++) {
    uint32_t codepoint = text->CodepointData.Codepoints[i];
    uint32_t key = codepoint;
    Asset::glyph* characterInfo =
      (Asset::glyph*)get(text->GlyphTable->Glyphs, key);
#if HOKI_DEV
    HOKI_ASSERT(characterInfo->DEBUGCodepoint == codepoint);
#endif
    add_renderable(*context.RenderableStore, characterInfo);
  }
}

void push_render_map(render_context& context, const MapSystem::map* map)
{
  render_command& shadowCommand = create_render_command(context);
  shadowCommand.Map = map;
  shadowCommand.Type = RENDER_COMMAND_TYPE_SHADOW;

  for (size_t i = 0; i < ARRAY_SIZE(map->Lights); i++) {
    push_render_light(context, map->Lights + i);
#if 0
    debug_push_render_light(context, map->Lights + i);
#endif
  }

  for (size_t i = 0; i < ARRAY_SIZE(map->EntitiesList); i++) {
    push_render_entity(context, &map->EntitiesList[i]);
  }
}

void push_setup_ui_context(render_context& renderContext,
                           UISystem::ui_context* uiContext)
{
  render_command& command = create_render_command(renderContext);
  command.Type = RENDER_COMMAND_TYPE_UI_CONTEXT;
  command.UIContext = uiContext;
}

void push_render_icon(render_context& context, const UISystem::ui_icon* icon)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_UI_ICON;
  command.Icon = icon;
  add_renderable(*context.RenderableStore, icon->Image);
}

void add_rendercommand(render_context& context,
                       const UISystem::ui_button* button)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_UI_BUTTON;
  command.Button = button;
  add_renderable(*context.RenderableStore, button->Image);
}

void add_rendercommand(render_context& context,
                       const UISystem::ui_toggle* toggle)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_UI_TOGGLE;
  command.Toggle = toggle;
  add_renderable(*context.RenderableStore, toggle->Image);
}

void add_rendercommand(render_context& context,
                       const UISystem::ui_joystick* joystick)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_UI_JOYSTICK;
  command.Joystick = joystick;
  add_renderable(*context.RenderableStore, joystick->Handle);
  add_renderable(*context.RenderableStore, joystick->Background);
}

void add_rendercommand(render_context& context,
                       const UISystem::ui_slider* slider)
{
  render_command& command = create_render_command(context);
  command.Type = RENDER_COMMAND_TYPE_UI_SLIDER;
  command.Slider = slider;
  add_renderable(*context.RenderableStore, slider->Handle);
  add_renderable(*context.RenderableStore, slider->Background);
}
