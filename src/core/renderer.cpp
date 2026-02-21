#include "renderer.h"
#include "shader_types.h"
#include "shaders/depth_shader.h"
#include "shaders/phong_shader.h"
#include "shaders/unlit_shader.h"

#include "../app_state.h"
#include "rasterizer.h"
#include "texture.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace core {
namespace {
constexpr float kClipEpsilon = 1e-5f;

struct ClipPlane {
  Vec4 plane;
  float bias = 0.f;
};

using ClipTriangle = std::array<VertexOut, 3>;

float plane_distance(const ClipPlane &clip_plane, const Vec4 &p) {
  return clip_plane.plane.x * p.x + clip_plane.plane.y * p.y +
         clip_plane.plane.z * p.z + clip_plane.plane.w * p.w + clip_plane.bias;
}

bool inside(const ClipPlane &clip_plane, const Vec4 &p) {
  return plane_distance(clip_plane, p) >= 0.f;
}

VertexOut lerp_vertex_out(const VertexOut &a, const VertexOut &b, float t) {
  VertexOut out;

  out.clip_pos = a.clip_pos + (b.clip_pos - a.clip_pos) * t;
  out.world_pos = a.world_pos + (b.world_pos - a.world_pos) * t;
  out.normal = a.normal + (b.normal - a.normal) * t;
  out.tangent = a.tangent + (b.tangent - a.tangent) * t;
  out.bitangent = a.bitangent + (b.bitangent - a.bitangent) * t;
  out.uv = a.uv + (b.uv - a.uv) * t;
  out.inv_w = (out.clip_pos.w != 0.f) ? (1.f / out.clip_pos.w) : 0.f;

  return out;
}

void clip_polygon_against_plane(const std::vector<VertexOut> &input,
                                const ClipPlane &clip_plane,
                                std::vector<VertexOut> &output) {
  output.clear();
  if (input.empty()) {
    return;
  }

  const VertexOut *start = &input.back();
  bool start_inside = inside(clip_plane, start->clip_pos);

  for (const VertexOut &end : input) {
    const bool end_inside = inside(clip_plane, end.clip_pos);

    if (start_inside && end_inside) {
      output.push_back(end);
    } else if (start_inside && !end_inside) {
      const float ds = plane_distance(clip_plane, start->clip_pos);
      const float de = plane_distance(clip_plane, end.clip_pos);
      const float denom = ds - de;
      const float t = (std::abs(denom) > 1e-8f) ? (ds / denom) : 0.f;
      output.push_back(lerp_vertex_out(*start, end, t));
    } else if (!start_inside && end_inside) {
      const float ds = plane_distance(clip_plane, start->clip_pos);
      const float de = plane_distance(clip_plane, end.clip_pos);
      const float denom = ds - de;
      const float t = (std::abs(denom) > 1e-8f) ? (ds / denom) : 0.f;
      output.push_back(lerp_vertex_out(*start, end, t));
      output.push_back(end);
    }

    start = &end;
    start_inside = end_inside;
  }
}

void clip_triangle_to_frustum(const VertexOut &a, const VertexOut &b,
                              const VertexOut &c,
                              std::vector<ClipTriangle> &out_tris) {
  static const std::array<ClipPlane, 7> kClipPlanes = {{
      {{0.f, 0.f, 0.f, 1.f}, -kClipEpsilon}, // w >= epsilon
      {{1.f, 0.f, 0.f, 1.f}, 0.f},           // x >= -w
      {{-1.f, 0.f, 0.f, 1.f}, 0.f},          // x <= w
      {{0.f, 1.f, 0.f, 1.f}, 0.f},           // y >= -w
      {{0.f, -1.f, 0.f, 1.f}, 0.f},          // y <= w
      {{0.f, 0.f, 1.f, 1.f}, 0.f},           // z >= -w
      {{0.f, 0.f, -1.f, 1.f}, 0.f},          // z <= w
  }};

  std::vector<VertexOut> poly = {a, b, c};
  std::vector<VertexOut> buffer;

  for (const ClipPlane &clip_plane : kClipPlanes) {
    clip_polygon_against_plane(poly, clip_plane, buffer);
    if (buffer.size() < 3) {
      out_tris.clear();
      return;
    }
    poly.swap(buffer);
  }

  out_tris.clear();
  out_tris.reserve(poly.size() - 2);
  for (size_t i = 1; i + 1 < poly.size(); ++i) {
    out_tris.push_back({poly[0], poly[i], poly[i + 1]});
  }
}

} // namespace

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

  std::vector<ClipTriangle> clipped_tris;
  clipped_tris.reserve(6);

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

    if (m_clipping_enabled) {
      clip_triangle_to_frustum(out0, out1, out2, clipped_tris);
      for (const ClipTriangle &tri : clipped_tris) {
        emit_triangle(tri[0], tri[1], tri[2], state, solid_tris);
      }
      continue;
    }

    emit_triangle(out0, out1, out2, state, solid_tris);
  }

  if (state.render_mode == RenderMode::Solid && !solid_tris.empty()) {
    m_rasterizer->draw_filled_triangles_tiled(solid_tris, *shader, uniforms);
  }
}
void Renderer::emit_triangle(const VertexOut &out0, const VertexOut &out1,
                             const VertexOut &out2, const AppState &state,
                             std::vector<ScreenTriangle> &solid_tris) {
  const Vec4 clip0 = out0.clip_pos;
  const Vec4 clip1 = out1.clip_pos;
  const Vec4 clip2 = out2.clip_pos;

  if (clip0.w <= kClipEpsilon || clip1.w <= kClipEpsilon ||
      clip2.w <= kClipEpsilon) {
    return;
  }

  // clip => NDC => SCREEN
  Vec3 screen0 = viewport_transform(perspective_divide(clip0));
  Vec3 screen1 = viewport_transform(perspective_divide(clip1));
  Vec3 screen2 = viewport_transform(perspective_divide(clip2));

  if (state.render_mode == RenderMode::Solid) {
    ScreenTriangle tri;
    tri.v0 = {screen0,     out0.uv,      clip0.w,       out0.world_pos,
              out0.normal, out0.tangent, out0.bitangent};
    tri.v1 = {screen1,     out1.uv,      clip1.w,       out1.world_pos,
              out1.normal, out1.tangent, out1.bitangent};
    tri.v2 = {screen2,     out2.uv,      clip2.w,       out2.world_pos,
              out2.normal, out2.tangent, out2.bitangent};
    tri.area =
        m_rasterizer->signed_triangle_area(tri.v0.pos, tri.v1.pos, tri.v2.pos);
    if (m_rasterizer->is_back_face_enabled() && tri.area > 0.f) {
      return;
    }

    tri.min_x = std::max(0, (int)std::floor(std::min(
                                {tri.v0.pos.x, tri.v1.pos.x, tri.v2.pos.x})));
    tri.min_y = std::max(0, (int)std::floor(std::min(
                                {tri.v0.pos.y, tri.v1.pos.y, tri.v2.pos.y})));
    tri.max_x = std::min(
        m_viewport_width - 1,
        (int)std::floor(std::max({tri.v0.pos.x, tri.v1.pos.x, tri.v2.pos.x})));
    tri.max_y = std::min(
        m_viewport_height - 1,
        (int)std::floor(std::max({tri.v0.pos.y, tri.v1.pos.y, tri.v2.pos.y})));

    if (tri.min_x > tri.max_x || tri.min_y > tri.max_y) {
      return;
    }
    solid_tris.push_back(tri);
    return;
  }
  switch (state.render_mode) {
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
  case RenderMode::Solid:
    break;
  } // switch RenderMode
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
