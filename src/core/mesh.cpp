#include "mesh.h"

namespace core {
Mesh Mesh::create_cube() {
  Mesh mesh;

  mesh.positions = {
      // -z
      Vec3(-1, 1, -1), Vec3(1, 1, -1), Vec3(1, -1, -1), Vec3(-1, -1, -1),
      // +z
      Vec3(-1, 1, 1), Vec3(1, 1, 1), Vec3(1, -1, 1), Vec3(-1, -1, 1)};

  mesh.indices = {

      0, 1, 2, 0, 2,
      3, // (-z) red

      4, 6, 5, 4, 7,
      6, // (+z) green

      0, 3, 7, 0, 7,
      4, // (-x) blue

      1, 5, 6, 1, 6,
      2, // (+x) yellow

      0, 4, 5, 0, 5,
      1, // (-y) qing

      3, 2, 6, 3, 6,
      7, // (+y) yang_hong
  };

  return mesh;
}

void Mesh::set_uniform_color(Color color) {
  triangle_colors.clear();
  triangle_colors.resize(triangle_count(), color);
}
} // namespace core
