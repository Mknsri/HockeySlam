#include "physics_system.h"

namespace PhysicsSystem {

body& add_body_to_space(space& physicsSpace,
                        const v3 position,
                        const body_type type,
                        const v3 size)
{
  body& newBody = physicsSpace.Bodies[physicsSpace.BodyCount++];
  HOKI_ASSERT(physicsSpace.BodyCount < PHYSICS_ARENA_MAX_BODIES);

  newBody = {};
  newBody.State.Position = position;
  newBody.State.Velocity = V3_ZERO;
  newBody.Size = size;

  newBody.Type = type;
  newBody.PreviousState = newBody.State;
  newBody.Bounce = 0.5f;
  newBody.Friction = DEFAULT_FRICTION;
  newBody.Sleeping = false;

  return newBody;
}

void add_bodies_to_bones(space& physicsSpace, game_entity& entity)
{
  const Asset::model& model = *entity.Model;
  HOKI_ASSERT(physicsSpace.BodyCount + model.BoneCount <
              PHYSICS_ARENA_MAX_BODIES);
  entity.Body = &add_body_to_space(
    physicsSpace, entity.Position, body_type::PHYSICS_BODY_TYPE_RAY, _v3(1.0f));
  entity.Body->Flags = body_flags::PHYSICS_BODY_FLAG_TRIGGER |
                       body_flags::PHYSICS_BODY_FLAG_STATIC;

  for (uint32_t i = 0; i < model.BoneCount; i++) {
    Asset::model_bone& bone = model.Bones[i];

    if (std::strcmp(bone.Name.Value, "Bone") == 0) {
      continue;
    }

    body& newBody = physicsSpace.Bodies[physicsSpace.BodyCount++];
    newBody = {};
    newBody.State.Position = entity.Position;
    newBody.State.Velocity = V3_ZERO;
    newBody.Size = _v3(0.5f, 0.5f, 0.15f);
    // Ugly
    if (std::strcmp(bone.Name.Value, "hand.L") == 0) {
      newBody.Size = newBody.Size * 2.0f;
    }

    newBody.Type = PHYSICS_BODY_TYPE_AABB;
    newBody.PreviousState = newBody.State;
    newBody.Flags =
      PHYSICS_BODY_FLAG_STATIC | PHYSICS_BODY_FLAG_ENTITY_CONTROLLED;
    newBody.ParentPosition = (v3*)&entity.BoneWorldPositions[i].S[12];

#if HOKI_DEV
    newBody.Name = entity.Model->Name;
#endif
  }
}

body& add_body_to_entity(space& physicsSpace,
                         game_entity& entity,
                         const v3 offset,
                         const v3 size,
                         float friction = DEFAULT_FRICTION)
{
  body& newBody = physicsSpace.Bodies[physicsSpace.BodyCount++];
  HOKI_ASSERT(physicsSpace.BodyCount < PHYSICS_ARENA_MAX_BODIES);

  newBody = {};
  // Since a lot of the models dont have origin at center mass, the positions
  // need to be also offset
  newBody.State.Position = entity.Position + offset;
  newBody.State.Velocity = V3_ZERO;
  newBody.Type = PhysicsSystem::PHYSICS_BODY_TYPE_AABB;
  newBody.PreviousState = newBody.State;
  newBody.Size = size;
  newBody.Friction = friction;
  newBody.Bounce = 0.01f;
  newBody.Sleeping = false;

  entity.BodyOffset = -offset;

  entity.Body = &newBody;

  return newBody;
}

body& add_static_body_to_entity(space& physicsSpace,
                                game_entity& entity,
                                const v3 offset,
                                const v3 size)
{
  body& createdBody = add_body_to_entity(physicsSpace, entity, offset, size);
  createdBody.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;

  return createdBody;
}

body& add_controlled_body_to_entity(space& physicsSpace,
                                    game_entity& entity,
                                    const v3 offset,
                                    const v3 size)
{
  body& createdBody = add_body_to_entity(physicsSpace, entity, offset, size);
  createdBody.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC |
                      PhysicsSystem::PHYSICS_BODY_FLAG_ENTITY_CONTROLLED;

  return createdBody;
}
}