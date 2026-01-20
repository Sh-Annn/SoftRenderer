#pragma once

#include "../types.h"
#include <vector>

namespace core {
class Mesh {
public:
  Mesh() = default;

  std::vector<Vec3> positions;
  std::vector<int> indices;

  std::vector<Vec3> normals;
  std::vector<Color> vertex_colors;
  std::vector<Color> triangle_colors;

  int triangle_count() const { return static_cast<int>(indices.size()) / 3; }
  int vertex_count() const { return static_cast<int>(positions.size()); }

  bool has_normals() const { return !normals.empty(); }
  bool use_vertex_colors() const { return !vertex_colors.empty(); }
  bool use_triangle_colors() const { return !triangle_colors.empty(); }

  void get_triangle_indices(int idx, int &i0, int &i1, int &i2) const {
    i0 = indices[idx * 3 + 0];
    i1 = indices[idx * 3 + 1];
    i2 = indices[idx * 3 + 2];
  }

  static Mesh create_cube();
  void set_uniform_color(Color color);
};
} // namespace core
