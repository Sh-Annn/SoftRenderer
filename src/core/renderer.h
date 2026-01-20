#pragma once

#include "../types.h"
#include "mesh.h"
#include "rasterizer.h"

namespace core {
class Renderer {
public:
  Renderer(Rasterizer *rasterizer, int viewport_width, int viewport_height);

  void draw_triangle(const Vec3 &va, const Vec3 &vb, const Vec3 &vc,
                     const mat4 &mvp, Color color);
  void draw_mesh(const Mesh &mesh, const mat4 &mvp);

  void set_viewport(int width, int height);

private:
  Vec3 perspective_divide(const Vec4 &clip_pos) const;
  Vec3 viewport_transform(const Vec3 &ndc) const;

  Rasterizer *m_rasterizer;
  int m_viewport_width;
  int m_viewport_height;
};
} // namespace core
