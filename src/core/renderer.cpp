#include "renderer.h"
#include "shader_types.h"
#include "shaders/depth_shader.h"
#include "shaders/phong_shader.h"
#include "shaders/unlit_shader.h"

#include "../app_state.h"
#include "rasterizer.h"
#include "texture.h"

#include <algorithm>
#include <cmath>

namespace core {
Renderer::Renderer(Rasterizer *rasterizer, int viewport_width,
                   int viewport_height)
    : m_rasterizer(rasterizer), m_viewport_width(viewport_width),
      m_viewport_height(viewport_height) {}

void Renderer::draw_mesh(const Mesh &mesh, const mat4 &model, const mat4 &mvp,
                         const Vec3 &view_pos, const AppState &state,
                         const Texture *texture, const Texture *normal_map) {
  PhongShader phong_shader;
  UnlitShader unlit_shader;
  DepthShader depth_shader;

  const IShader *shader = &phong_shader;
  switch (state.shader_type) {
  case ShaderType::Phong:
    shader = &phong_shader;
    break;
  case ShaderType::Unlit:
    shader = &unlit_shader;
    break;
  case ShaderType::Depth:
    shader = &depth_shader;
    break;
  }

  ShaderUniforms uniforms;
  uniforms.model = model;
  uniforms.mvp = mvp;
  uniforms.view_pos = view_pos;
  uniforms.texture = texture;
  uniforms.texture_enabled = state.texture_enabled;
  uniforms.normal_map = normal_map;
  uniforms.normal_map_enabled = state.normal_map_enabled;
  uniforms.light_enabled = state.light_enabled;
  uniforms.diff_enabled = state.diff_enabled;
  uniforms.spec_enabled = state.spec_enabled;
  uniforms.persp_interp_enabled = state.persp_interp_enabled;
  uniforms.light_pos = state.light_position;
  uniforms.light_intensity = state.LIGHT_INTENSITY;

  std::vector<ScreenTriangle> solid_tris;
  if (state.render_mode == RenderMode::Solid) {
    solid_tris.reserve(mesh.triangle_count());
  }

  for (int i = 0; i < mesh.triangle_count(); i++) {
    int i0, i1, i2;
    mesh.get_triangle_indices(i, i0, i1, i2);

    const Vec3 &v0 = mesh.positions[i0];
    const Vec3 &v1 = mesh.positions[i1];
    const Vec3 &v2 = mesh.positions[i2];

    Vec3 n0(0, 0, 1), n1(0, 0, 1), n2(0, 0, 1);
    if (mesh.has_normals()) {
      n0 = mesh.normals[i0];
      n1 = mesh.normals[i1];
      n2 = mesh.normals[i2];
    }

    Vec2 uv0(0, 0), uv1(0, 0), uv2(0, 0);
    if (mesh.has_texcoords()) {
      uv0 = mesh.texcoords[i0];
      uv1 = mesh.texcoords[i1];
      uv2 = mesh.texcoords[i2];
    }

    Vec3 t0(1.f, 0.f, 0.f), t1(1.f, 0.f, 0.f), t2(1.f, 0.f, 0.f);
    Vec3 b0(0.f, 1.f, 0.f), b1(0.f, 1.f, 0.f), b2(0.f, 1.f, 0.f);
    if (mesh.has_tangent_space()) {
      t0 = mesh.tangents[i0];
      t1 = mesh.tangents[i1];
      t2 = mesh.tangents[i2];
      b0 = mesh.bitangents[i0];
      b1 = mesh.bitangents[i1];
      b2 = mesh.bitangents[i2];
    }

    const VertexOut out0 = shader->vertex({v0, n0, uv0, t0, b0}, uniforms);
    const VertexOut out1 = shader->vertex({v1, n1, uv1, t1, b1}, uniforms);
    const VertexOut out2 = shader->vertex({v2, n2, uv2, t2, b2}, uniforms);

    const Vec4 clip0 = out0.clip_pos;
    const Vec4 clip1 = out1.clip_pos;
    const Vec4 clip2 = out2.clip_pos;

    // +z ----- -z
    // point ----- camera
    if (clip0.w <= 0.f || clip1.w <= 0.f || clip2.w <= 0.f) {
      continue;
    }

    if (m_clipping_enabled && (!all_inside_plane(clip0, clip1, clip2))) {
      continue;
    }

    // clip => NDC => SCREEN
    Vec3 screen0 = viewport_transform(perspective_divide(clip0));
    Vec3 screen1 = viewport_transform(perspective_divide(clip1));
    Vec3 screen2 = viewport_transform(perspective_divide(clip2));

    // Color fallback_color = 0xFFFFFFFF;
    // if (mesh.use_triangle_colors()) {
    //   fallback_color = mesh.triangle_colors[i];
    // } else if (mesh.use_vertex_colors()) {
    //   fallback_color = mesh.vertex_colors[i0];
    // }
    if (state.render_mode == RenderMode::Solid) {
      ScreenTriangle tri;

      tri.v0 = {screen0,     out0.uv,      clip0.w,       out0.world_pos,
                out0.normal, out0.tangent, out0.bitangent};
      tri.v1 = {screen1,     out1.uv,      clip1.w,       out1.world_pos,
                out1.normal, out1.tangent, out1.bitangent};
      tri.v2 = {screen2,     out2.uv,      clip2.w,       out2.world_pos,
                out2.normal, out2.tangent, out2.bitangent};
      tri.area = m_rasterizer->signed_triangle_area(tri.v0.pos, tri.v1.pos,
                                                    tri.v2.pos);
      if (m_rasterizer->is_back_face_enabled() && tri.area > 0.f) {
        continue;
      }

      tri.min_x = std::max(0, (int)std::floor(std::min(
                                  {tri.v0.pos.x, tri.v1.pos.x, tri.v2.pos.x})));
      tri.min_y = std::max(0, (int)std::floor(std::min(
                                  {tri.v0.pos.y, tri.v1.pos.y, tri.v2.pos.y})));
      tri.max_x = std::min(m_viewport_width - 1,
                           (int)std::floor(std::max(
                               {tri.v0.pos.x, tri.v1.pos.x, tri.v2.pos.x})));
      tri.max_y = std::min(m_viewport_height - 1,
                           (int)std::floor(std::max(
                               {tri.v0.pos.y, tri.v1.pos.y, tri.v2.pos.y})));
      if (tri.min_x > tri.max_x || tri.min_y > tri.max_y) {
        continue;
      }

      solid_tris.push_back(tri);
    }

    switch (state.render_mode) {
    case RenderMode::Solid: {

      // m_rasterizer->draw_filled_triangle(vert0, vert1, vert2, texture,
      //                                    view_pos);
      break;
    }
    case RenderMode::Vertex: {
      m_rasterizer->put_pixel(screen0.x, screen0.y);
      m_rasterizer->put_pixel(screen1.x, screen1.y);
      m_rasterizer->put_pixel(screen2.x, screen2.y);
      break;
    }
    case RenderMode::WireFrame: {
      m_rasterizer->draw_line(screen0, screen1);
      m_rasterizer->draw_line(screen1, screen2);
      m_rasterizer->draw_line(screen2, screen0);
      break;
    }
    } // switch RenderMode
  }
  if (state.render_mode == RenderMode::Solid) {
    m_rasterizer->draw_filled_triangles_tiled(solid_tris, *shader, uniforms);
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

bool Renderer::inside_plane(const Vec4 &plane, const Vec4 &p) {
  return (plane.x * p.x + plane.y * p.y + plane.z * p.z + plane.w * p.w) >= 0;
}

bool Renderer::all_inside_plane(const Vec4 &v0, const Vec4 &v1,
                                const Vec4 &v2) {
  if (v0.x > v0.w || v0.x < -v0.w) {
    return false;
  }
  if (v0.y > v0.w || v0.y < -v0.w) {
    return false;
  }
  if (v0.z > v0.w || v0.z < -v0.w) {
    return false;
  }

  if (v1.x > v1.w || v1.x < -v1.w) {
    return false;
  }
  if (v1.y > v1.w || v1.y < -v1.w) {
    return false;
  }
  if (v1.z > v1.w || v1.z < -v1.w) {
    return false;
  }

  if (v2.x > v2.w || v2.x < -v2.w) {
    return false;
  }
  if (v2.y > v2.w || v2.y < -v2.w) {
    return false;
  }
  if (v2.z > v2.w || v2.z < -v2.w) {
    return false;
  }

  return true;
}
} // namespace core
