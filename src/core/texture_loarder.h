#pragma once

#include "texture.h"
#include <string>

namespace core {
class TextureLoader {
public:
  static bool load(const std::string &filename, Texture &out_texture);
};
} // namespace core
