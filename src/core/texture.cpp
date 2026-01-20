#include "texture.h"

#include <algorithm>
#include <cmath>

namespace core {
void Texture::create(int width, int height, int channels,
                     const unsigned char *data) {
  m_width = width;
  m_height = height;
  m_channels = channels;

  size_t size = width * height * channels;
  m_data.assign(data, data + size);
}

void Texture::clear() {
  m_data.clear();
  m_width = m_height = m_channels = 0;
}

float Texture::wrap_coord(float coord, WrapMode mode) const {
  if (mode == WrapMode::Repeat) {
    return coord - std::floor(coord);
  } else {
    return std::clamp(coord, 0.f, 1.f);
  }
}

Color Texture::get_pixel(int x, int y) const {
  x = std::clamp(x, 0, m_width - 1);
  y = std::clamp(y, 0, m_height - 1);

  int idx = (y * m_width + x) * m_channels;

  unsigned char r = m_data[idx];
  unsigned char g = m_data[idx];
  unsigned char b = m_data[idx];
  unsigned char a = (m_channels == 4) ? m_data[idx + 3] : 255;

  return (a << 24) | (r << 16) | (g << 8) | b;
}
Color Texture::sample(float u, float v, FilterMode filter,
                      WrapMode wrap) const {
  if (!valid()) {
    return 0xFFFFFFFF;
  }

  u = wrap_coord(u, wrap);
  v = wrap_coord(v, wrap);

  v = 1.f - v;

  if (filter == FilterMode::Nearest) {
    int x = static_cast<int>(u * m_width);
    int y = static_cast<int>(v * m_height);
    return get_pixel(x, y);
  }

  return get_pixel(static_cast<int>(u * m_width),
                   static_cast<int>(v * m_height));
}
} // namespace core
