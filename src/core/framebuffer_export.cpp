#include "framebuffer_export.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace core {
bool save_framebuffer_png(const std::vector<Color> &framebuffer, int width,
                          int height, const std::string &path) {
  if (width < 0 || height < 0) {
    return false;
  }

  const size_t expected_size = static_cast<size_t>(width) * height;
  if (framebuffer.size() < expected_size) {
    return false;
  }

  std::vector<unsigned char> rgba(expected_size * 4);
  for (size_t i = 0; i < expected_size; i++) {
    const Color c = framebuffer[i];
    const unsigned char a = static_cast<unsigned char>((c >> 24) & 0xFF);
    const unsigned char r = static_cast<unsigned char>((c >> 16) & 0xFF);
    const unsigned char g = static_cast<unsigned char>((c >> 8) & 0xFF);
    const unsigned char b = static_cast<unsigned char>(c & 0xFF);

    rgba[i * 4 + 0] = r;
    rgba[i * 4 + 1] = g;
    rgba[i * 4 + 2] = b;
    rgba[i * 4 + 3] = a;
  }

  return stbi_write_png(path.c_str(), width, height, 4, rgba.data(),
                        width * 4) != 0;
}
} // namespace core
