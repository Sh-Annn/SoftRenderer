#include "mesh.h"
#include <iostream>

namespace core {

Mesh Mesh::create_cube() {
  Mesh mesh;

  // 8 个顶点位置
  mesh.positions = {
      Vec3(-1, 1, -1),  // 0: 左上后
      Vec3(1, 1, -1),   // 1: 右上后
      Vec3(1, -1, -1),  // 2: 右下后
      Vec3(-1, -1, -1), // 3: 左下后
      Vec3(-1, 1, 1),   // 4: 左上前
      Vec3(1, 1, 1),    // 5: 右上前
      Vec3(1, -1, 1),   // 6: 右下前
      Vec3(-1, -1, 1),  // 7: 左下前
  };

  // 12 个三角形索引（36 个元素）
  mesh.indices = {
      0, 1, 2, 0, 2, 3, // 后面（-Z）
      4, 6, 5, 4, 7, 6, // 前面（+Z）
      0, 3, 7, 0, 7, 4, // 左面（-X）
      1, 5, 6, 1, 6, 2, // 右面（+X）
      0, 4, 5, 0, 5, 1, // 底面（-Y）
      3, 2, 6, 3, 6, 7, // 顶面（+Y）
  };

  // 调试输出
  std::cout << "[Mesh:: create_cube] positions. size() = "
            << mesh.positions.size() << std::endl;
  std::cout << "[Mesh::create_cube] indices. size() = " << mesh.indices.size()
            << std::endl;
  std::cout << "[Mesh::create_cube] triangle_count() = "
            << mesh.triangle_count() << std::endl;

  return mesh;
}

void Mesh::set_uniform_color(Color color) {
  triangle_colors.clear();
  triangle_colors.resize(triangle_count(), color);
}

} // namespace core
