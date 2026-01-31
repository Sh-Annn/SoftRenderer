#include "renderer.h"
#include "texture.h"

namespace core {
Renderer::Renderer(Rasterizer *rasterizer, int viewport_width,
                   int viewport_height)
    : m_rasterizer(rasterizer), m_viewport_width(viewport_width),
      m_viewport_height(viewport_height) {}

// void Renderer::draw_mesh(const Mesh &mesh, const mat4 &mvp,
//                          const Texture *texture) {
void Renderer::draw_mesh(const Mesh &mesh, const mat4 &mvp,
                         const Texture *texture) {
  for (int i = 0; i < mesh.triangle_count(); i++) {
    int i0, i1, i2;
    mesh.get_triangle_indices(i, i0, i1, i2);

    const Vec3 &v0 = mesh.positions[i0];
    const Vec3 &v1 = mesh.positions[i1];
    const Vec3 &v2 = mesh.positions[i2];

    Vec2 uv0(0, 0), uv1(0, 0), uv2(0, 0);
    if (mesh.has_texcoords()) {
      uv0 = mesh.texcoords[i0];
      uv1 = mesh.texcoords[i1];
      uv2 = mesh.texcoords[i2];
    }

    Vec4 clip0 = mvp * Vec4(v0, 1.f);
    Vec4 clip1 = mvp * Vec4(v1, 1.f);
    Vec4 clip2 = mvp * Vec4(v2, 1.f);

    if (clip0.w <= 0.f || clip1.w <= 0.f || clip2.w <= 0.f) {
      continue;
    }

    // clip => NDC
    Vec3 ndc0 = perspective_divide(clip0);
    Vec3 ndc1 = perspective_divide(clip1);
    Vec3 ndc2 = perspective_divide(clip2);

    // NDC => SCREEN
    Vec3 screen0 = viewport_transform(ndc0);
    Vec3 screen1 = viewport_transform(ndc1);
    Vec3 screen2 = viewport_transform(ndc2);

    Color fallback_color = 0xFFFFFFFF;
    if (mesh.use_triangle_colors()) {
      fallback_color = mesh.triangle_colors[i];
    } else if (mesh.use_vertex_colors()) {
      fallback_color = mesh.vertex_colors[i0];
    }

    // m_rasterizer->draw_filled_triangle(screen0, screen1, screen2, uv0, uv1,
    // uv2,
    //                                    clip0.w, clip1.w, clip2.w, texture,
    //                                    fallback_color);
    Vertex vert0 = {screen0, uv0, clip0.w};
    Vertex vert1 = {screen1, uv1, clip1.w};
    Vertex vert2 = {screen2, uv2, clip2.w};
    m_rasterizer->draw_filled_triangle(vert0, vert1, vert2, texture,
                                       fallback_color);
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
