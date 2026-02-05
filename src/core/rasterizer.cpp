#include "rasterizer.h"
#include "../color.h"
#include "math_utils.h"

#include <algorithm>
#include <glm/geometric.hpp>

namespace core {
// Vec3 fragment(Vec3 n, Vec3 pos) {
//   Vec3 kd = {0.79, 0.79, 0.79};
//   Vec3 light = {-20, 20, 0};
//
//   Vec3 l = glm::normalize(light - pos);
//
//   float r_2 = glm::dot((pos - light), (pos - light));
//
//   Vec3 ls = kd * (glm::normalize(light) / r_2) * std::max(0.f, glm::dot(n,
//   l));
//
//   Vec3 result = ls;
//
//   return result;
// }
void Rasterizer::Init(int width, int height) {
  w_ = width;
  h_ = height;
  frame_buf.resize(width * height);
  depth_buf.resize(width * height);
}
void Rasterizer::clear() {
  if (!valid()) {
    return;
  }

  std::fill(frame_buf.begin(), frame_buf.end(), 0);
  std::fill(depth_buf.begin(), depth_buf.end(), 1.f);
}

void Rasterizer::put_pixel(int x, int y, Color color) {
  if (x >= 0 && x < w_ && y >= 0 && y < h_) {
    frame_buf[y * w_ + x] = color;
  }
}

void Rasterizer::draw_line(Vec3 a, Vec3 b, Color color) {
  bool steep = std::abs(b.y - a.y) > std::abs(b.x - a.x);
  if (steep) {
    std::swap(a.x, a.y);
    std::swap(b.x, b.y);
  }
  if (a.x > b.x) {
    std::swap(a.x, b.x);
    std::swap(a.y, b.y);
  }

  int y = a.y;
  int ierror = 0;
  for (int x = a.x; x < b.x; ++x) {
    float t = (float)(x - a.x) / (b.x - a.x);
    float curr_z = math::lerp(a.z, b.z, t);
    int idx = y * w_ + x;
    if (steep) {
      put_pixel(y, x, color);
    } else {
      put_pixel(x, y, color);
    }

    ierror += 2 * std::abs(b.y - a.y);
    y += (b.y > a.y ? 1 : -1) * (ierror > b.x - a.x);
    ierror -= 2 * (b.x - a.x) * (ierror > b.x - a.x);
  }
}

float Rasterizer::signed_triangle_area(const Vec3 &a, const Vec3 &b,
                                       const Vec3 &c) {
  return 0.5 * ((b.y - a.y) * (b.x + a.x) + (c.y - b.y) * (c.x + b.x) +
                (a.y - c.y) * (a.x + c.x));
}

void Rasterizer::draw_filled_triangle(const Vertex &v0, const Vertex &v1,
                                      const Vertex &v2, const Texture *texture,
                                      Color fallback_color,
                                      const Vec3 &view_pos) {
  if (!valid()) {
    return;
  }

  const Vec3 &sa = v0.pos;
  const Vec3 &sb = v1.pos;
  const Vec3 &sc = v2.pos;

  int min_x = (int)std::floor(std::min({sa.x, sb.x, sc.x}));
  int min_y = (int)std::floor(std::min({sa.y, sb.y, sc.y}));
  int max_x = (int)std::floor(std::max({sa.x, sb.x, sc.x}));
  int max_y = (int)std::floor(std::max({sa.y, sb.y, sc.y}));

  min_x = std::max(min_x, 0);
  min_y = std::max(min_y, 0);
  max_x = std::min(max_x, w_ - 1);
  max_y = std::min(max_y, h_ - 1);

  float area = signed_triangle_area(sa, sb, sc);
  if (std::abs(area) < 1e-6f) {
    return;
  }

  float inv_w0 = 1.f / v0.w;
  float inv_w1 = 1.f / v1.w;
  float inv_w2 = 1.f / v2.w;

  // Phong Lighting parameters (Constant for the frame/triangle in this simple
  // implementation)
  const Vec3 light_pos = {2.0f, 2.0f, 2.0f};
  const Vec3 light_color = {1.0f, 1.0f, 1.0f};
  const float Ia = 0.5f;
  const float Is = 3.f;
  const Vec3 ambient = Ia * light_color;

  for (int y = min_y; y <= max_y; ++y) {
    for (int x = min_x; x <= max_x; ++x) {
      float alpha = signed_triangle_area({x, y, 0}, sb, sc) / area;
      float beta = signed_triangle_area({x, y, 0}, sc, sa) / area;
      float gama = signed_triangle_area({x, y, 0}, sa, sb) / area;

      if (alpha >= 0 && beta >= 0 && gama >= 0) {
        float depth = alpha * sa.z + beta * sb.z + gama * sc.z;

        int idx = y * w_ + x;
        bool pass_depth = !m_depth_test_enabled || (depth < depth_buf[idx]);

        if (pass_depth) {
          Color color = fallback_color;

          float inv_w = alpha * inv_w0 + beta * inv_w1 + gama * inv_w2;

          if (m_texture_enabled && texture && texture->valid()) {
            float u, v;

            if (m_persp_interp_enabled) {

              u = (alpha * v0.uv.x * inv_w0 + beta * v1.uv.x * inv_w1 +
                   gama * v2.uv.x * inv_w2) /
                  inv_w;
              v = (alpha * v0.uv.y * inv_w0 + beta * v1.uv.y * inv_w1 +
                   gama * v2.uv.y * inv_w2) /
                  inv_w;

            } else {
              u = alpha * v0.uv.x + beta * v1.uv.x + gama * v2.uv.x;
              v = alpha * v0.uv.y + beta * v1.uv.y + gama * v2.uv.y;
            }

            color = texture->sample(u, v);

          } else {
            color = fallback_color;
          }

          if (m_light_enabled) {
            Vec3 p_world_pos =
                (alpha * v0.world_pos * inv_w0 + beta * v1.world_pos * inv_w1 +
                 gama * v2.world_pos * inv_w2) /
                inv_w;
            Vec3 p_normal =
                (alpha * v0.normal * inv_w0 + beta * v1.normal * inv_w1 +
                 gama * v2.normal * inv_w2) /
                inv_w;
            p_normal = glm::normalize(p_normal);

            Vec3 object_color;
            object_color.r = ((color >> 16) & 0xFF) / 255.f;
            object_color.g = ((color >> 8) & 0xFF) / 255.f;
            object_color.b = (color & 0xFF) / 255.f;

            // Diffuse
            Vec3 light_dir = glm::normalize(light_pos - p_world_pos);
            float diff = std::max(glm::dot(p_normal, light_dir), 0.f);
            Vec3 diffuse = diff * light_color;

            // Result
            Vec3 result = diffuse * object_color;
            result = glm::clamp(result, 0.f, 1.f);

            color = make_color((u8)(result.x * 255), (u8)(result.y * 255),
                               (u8)(result.z * 255));
          }

          frame_buf[idx] = color;

          if (m_depth_test_enabled) {
            depth_buf[idx] = depth;
          }
        } // depth test
      } // barycentric coordinates
    }
  }
}
} // namespace core
