#pragma once

#include "../types.h"

#include "texture.h"
#include "vertex.h"

namespace core {
class Rasterizer {
public:
  void Init(int width, int height);

  void clear();

  void put_pixel(int x, int y, Color color);
  void draw_line(Vec3 &a, Vec3 &b, Color color);
  // void draw_filled_triangle(const Vec3 &a, const Vec3 &b, const Vec3 &c,
  //                           Color color);
  // void draw_filled_triangle(const Vec3 &sa, const Vec3 &sb, const Vec3 &sc,
  //                           const Vec2 &uv0, const Vec2 &uv1, const Vec2
  //                           &uv2, float w0, float w1, float w2, const Texture
  //                           *texture, Color fallback_color);
  // void draw_filled_triangle(const Vec3 &sa, const Vec3 &sb, const Vec3 &sc,
  //                           const Vec2 &uv0, const Vec2 &uv1, const Vec2
  //                           &uv2, float w0, float w1, float w2, Color
  //                           fallback_color);
  void draw_filled_triangle(const Vertex &v0, const Vertex &v1,
                            const Vertex &v2, const Texture *texture,
                            Color fallback_color);
  float signed_triangle_area(const Vec3 &a, const Vec3 &b, const Vec3 &c);

  std::vector<Color> &frame_buffer() { return frame_buf; }
  std::vector<float> &depth_buffer() { return depth_buf; }

  void set_depth_test_enabled(bool enabled) { m_depth_test_enabled = enabled; }
  bool is_depth_test_enabled() const { return m_depth_test_enabled; }

private:
  std::vector<Color> frame_buf;
  std::vector<float> depth_buf;
  int w_ = 0;
  int h_ = 0;
  bool m_depth_test_enabled = true;

  bool valid() const { return w_ > 0 && h_ > 0; }
};

} // namespace core
