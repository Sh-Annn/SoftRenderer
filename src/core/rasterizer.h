#pragma once

#include "../types.h"

#include "texture.h"
#include "vertex.h"

namespace core {
class Rasterizer {
public:
  void Init(int width, int height);

  void clear();

  void put_pixel(int x, int y);
  void draw_line(Vec3 a, Vec3 b);
  void draw_filled_triangle(const Vertex &v0, const Vertex &v1,
                            const Vertex &v2, const Texture *texture,
                            const Vec3 &view_pos);
  float signed_triangle_area(const Vec3 &a, const Vec3 &b, const Vec3 &c);

  std::vector<Color> &frame_buffer() { return frame_buf; }
  std::vector<float> &depth_buffer() { return depth_buf; }

  void set_depth_test_enabled(bool enabled) { m_depth_test_enabled = enabled; }
  void set_persp_interp_enabled(bool enabled) {
    m_persp_interp_enabled = enabled;
  }
  void set_texture_enabled(bool enabled) { m_texture_enabled = enabled; }
  void set_light_enabled(bool enabled) { m_light_enabled = enabled; }
  void set_diff_enabled(bool enabled) { m_diff_enabled = enabled; }
  void set_spec_enabled(bool enabled) { m_spec_enabled = enabled; }
  void set_light_pos(Vec3 position) { m_light_pos = position; }
  void set_light_intensity(float intensity) { m_light_intensity = intensity; }

  bool is_depth_test_enabled() const { return m_depth_test_enabled; }
  bool is_persp_interp_enabled() const { return m_persp_interp_enabled; }
  bool is_texture_enabled() const { return m_texture_enabled; }
  bool is_light_enabled() const { return m_light_enabled; }
  bool is_diff_enabled() const { return m_diff_enabled; }
  bool is_spec_enabled() const { return m_spec_enabled; }

private:
  std::vector<Color> frame_buf;
  std::vector<float> depth_buf;
  int w_ = 0;
  int h_ = 0;
  bool m_depth_test_enabled = true;
  bool m_persp_interp_enabled = true;
  bool m_texture_enabled = true;
  bool m_light_enabled = true;
  bool m_diff_enabled = true;
  bool m_spec_enabled = true;
  Vec3 m_light_pos = {-2.f, 2.f, 2.f};
  float m_light_intensity = 1.f;

  bool valid() const { return w_ > 0 && h_ > 0; }
};

} // namespace core
