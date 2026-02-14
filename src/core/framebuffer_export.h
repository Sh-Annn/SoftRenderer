#pragma once

#include "../types.h"

#include <string>
#include <vector>

namespace core {
bool save_framebuffer_png(const std::vector<Color> &framebuffer, int width,
                          int height, const std::string &path);
}
