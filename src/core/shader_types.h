#pragma once

#include "../types.h"

namespace core {
class Texture;

struct VertexIn {
  Vec3 position = {0.f, 0.f, 0.f};
  Vec3 normal = {0.f, 0.f, 0.f};
  Vec2 uv = {0.f, 0.f};
  Vec3 tangent = {1.f, 0.f, 0.f};
  Vec3 bitangent = {0.f, 1.f, 0.f};
};

struct VertexOut {
  Vec4 clip_pos = {0.f, 0.f, 0.f, 1.f};
  Vec3 world_pos = {0.f, 0.f, 0.f};
  Vec3 normal = {0.f, 0.f, 1.f};
  Vec3 tangent = {1.f, 0.f, 0.f};
  Vec3 bitangent = {0.f, 1.f, 0.f};
  Vec2 uv = {0.f, 0.f};
  float inv_w = 0.f;
};

struct FragmentIn {
  Vec3 world_pos = {0.f, 0.f, 0.f};
  Vec3 normal = {0.f, 0.f, 1.f};
  Vec3 tangent = {1.f, 0.f, 0.f};
  Vec3 bitangent = {0.f, 1.f, 0.f};
  Vec2 uv = {0.f, 0.f};
  float depth = 1.f;
};

struct FragmentOut {
  Color color = 0;
  bool discard = false;
};

struct ShaderUniforms {
  mat4 model = mat4(1.f);
  mat4 mvp = mat4(1.f);

  Vec3 view_pos = {0.f, 0.f, 0.f};

  Vec3 light_pos = {-2.f, 2.f, 2.f};
  Vec3 light_color = {1.f, 1.f, 1.f};
  float light_intensity = 1.f;

  float ambient_intensity = 0.5f;
  Vec3 kd = {0.5f, 0.5f, 0.5f};
  Vec3 ks = {0.5f, 0.5f, 0.5f};
  float shininess = 150.f;

  bool texture_enabled = false;
  bool normal_map_enabled = false;
  bool light_enabled = true;
  bool diff_enabled = true;
  bool spec_enabled = true;
  bool persp_interp_enabled = true;

  const Texture *texture = nullptr;
  const Texture *normal_map = nullptr;
};
} // namespace core
