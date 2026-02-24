#include "texture_loader.h"

#include <iostream>

#ifdef _WIN32
#define STBI_WINDOWS_UTF8
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace core {
bool TextureLoader::load(const std::string &filename, Texture &out_texture) {
  int width, height, channels;

  unsigned char *data =
      stbi_load(filename.c_str(), &width, &height, &channels, 0);

  if (!data) {
    std::cerr << "Failed to load texture: " << filename << '\n';
    std::cerr << "  Reason: " << stbi_failure_reason() << '\n';
    return false;
  }

  out_texture.create(width, height, channels, data);

  stbi_image_free(data);

  std::cout << "Loaded texture: " << filename << '\n';
  std::cout << "  Size: " << width << 'x' << height << '\n';
  std::cout << " Channels: " << channels << '\n';

  return true;
}
} // namespace core
