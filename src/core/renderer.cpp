#include "renderer.h"

namespace core {
Renderer::Renderer(Rasterizer *rasterizer, int viewport_width,
                   int viewport_height)
    : m_rasterizer(rasterizer), m_viewport_width(viewport_width),
      m_viewport_height(viewport_height) {}

void Renderer::draw_triangle(const Vec3 &va, const Vec3 &vb, const Vec3 &vc,
                             const mat4 &mvp, Color color) {
  // world => clip (apply mvp)
  Vec4 clip0 = mvp * Vec4(va, 1.f);
  Vec4 clip1 = mvp * Vec4(vb, 1.f);
  Vec4 clip2 = mvp * Vec4(vc, 1.f);

  if (clip0.w <= 0.f || clip1.w <= 0.f || clip2.w <= 0.f) {
    return;
  }

  // clip => NDC
  Vec3 ndc0 = perspective_divide(clip0);
  Vec3 ndc1 = perspective_divide(clip1);
  Vec3 ndc2 = perspective_divide(clip2);

  // NDC => SCREEN
  Vec3 screen0 = viewport_transform(ndc0);
  Vec3 screen1 = viewport_transform(ndc1);
  Vec3 screen2 = viewport_transform(ndc2);

  // Rasterizer
  m_rasterizer->draw_filled_triangle(screen0, screen1, screen2, color);
}

void Renderer::draw_mesh(const Mesh &mesh, const mat4 &mvp) {
  for (int i = 0; i < mesh.triangle_count(); i++) {
    int i0, i1, i2;
    mesh.get_triangle_indices(i, i0, i1, i2);

    const Vec3 &v0 = mesh.positions[i0];
    const Vec3 &v1 = mesh.positions[i1];
    const Vec3 &v2 = mesh.positions[i2];

    Color color;
    if (mesh.use_triangle_colors()) {
      color = mesh.triangle_colors[i];
    } else if (mesh.use_vertex_colors()) {
      color = mesh.vertex_colors[i0];
    } else {
      color = 0xFFFFFFFF;
    }

    draw_triangle(v0, v1, v2, mvp, color);
  }
}

void Renderer::set_viewport(int width, int height) {
  m_viewport_width = width;
  m_viewport_height = height;
}

Vec3 Renderer::perspective_divide(const Vec4 &clip_pos) const {
  if (clip_pos.w == 0.0f) {
    return Vec3(0, 0, 0);
  }
  return Vec3(clip_pos.x / clip_pos.w, clip_pos.y / clip_pos.w,
              clip_pos.z / clip_pos.w);
}
Vec3 Renderer::viewport_transform(const Vec3 &ndc) const {
  float x = (ndc.x + 1.f) * 0.5f * m_viewport_width;
  float y = (1 - ndc.y) * 0.5f * m_viewport_height;
  float z = (ndc.z + 1.f) * 0.5f;

  return Vec3(x, y, z);
}
} // namespace core
