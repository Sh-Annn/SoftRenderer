#pragma once

#include "mesh.h"
#include <string>

namespace core {
class MeshLoader {
public:
  static Mesh load_obj(const std::string &filename, Color default_color);

private:
  static void parse_face_index(const std::string &token, int &v_idx,
                               int &vt_idx, int &vn_idx);
};
} // namespace core
