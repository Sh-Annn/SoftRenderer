#pragma once

#include "../types.h"
#include "mesh.h"
#include "rasterizer.h"
#include "texture.h"

struct AppState;

namespace core {
class Renderer {
public:
  Renderer(Rasterizer *rasterizer, int viewport_width, int viewport_height);

  void draw_mesh(const Mesh &mesh, const mat4 &model, const mat4 &mvp,
                 const Vec3 &view_pos, const AppState &state,
                 const Texture *texture = nullptr);

  void set_viewport(int width, int height);

private:
  Vec3 perspective_divide(const Vec4 &clip_pos) const;
  Vec3 viewport_transform(const Vec3 &ndc) const;

  bool inside_plane(const Vec4 &plane, const Vec4 &p);
  bool all_inside_plane(const Vec4 &v0, const Vec4 &v1, const Vec4 &v2);

  Rasterizer *m_rasterizer;
  int m_viewport_width;
  int m_viewport_height;
};
} // namespace core
