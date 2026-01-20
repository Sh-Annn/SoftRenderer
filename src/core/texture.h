#pragma once

#include "../types.h"
#include <vector>

namespace core {
enum class FilterMode { Nearest, Bilinear };
enum class WrapMode { Repeat, Clamp };

class Texture {
public:
  Texture() = default;
  ~Texture() = default;

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
  Texture(Texture &&) = default;
  Texture &operator=(Texture &&) = default;

  void create(int width, int height, int channels, const unsigned char *data);
  void clear();

  Color sample(float u, float v, FilterMode filter = FilterMode::Nearest,
               WrapMode wrap = WrapMode::Repeat) const;

  int width() const { return m_width; }
  int height() const { return m_height; }
  int channels() const { return m_channels; }
  bool valid() const { return !m_data.empty(); }

private:
  Color get_pixel(int x, int y) const;
  float wrap_coord(float coord, WrapMode mode) const;

  std::vector<unsigned char> m_data;
  int m_width = 0;
  int m_height = 0;
  int m_channels = 0;
};
} // namespace core
