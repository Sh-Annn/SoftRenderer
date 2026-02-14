#pragma once

#include "../types.h"

namespace core {
struct Vertex {
  Vec3 pos;
  Vec2 uv;
  float w;

  Vec3 world_pos;
  Vec3 normal;
  Vec3 tangent;
  Vec3 bitangent;
};
} // namespace core
