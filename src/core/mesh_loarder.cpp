#include "mesh_loarder.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace core {
Mesh MeshLoader::load_obj(const std::string &filename, Color default_color) {
  Mesh mesh;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Faild to open OBJ file: " << filename << '\n';
    return mesh;
  }

  std::vector<Vec3> temp_positions;
  std::vector<Vec3> temp_normals;

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    if (prefix == "v") {
      Vec3 pos;
      iss >> pos.x >> pos.y >> pos.z;
      temp_positions.push_back(pos);
    } else if (prefix == "vn") {
      Vec3 normal;
      iss >> normal.x >> normal.y >> normal.z;
      temp_normals.push_back(normal);
    } else if (prefix == "f") {
      std::string v1_str, v2_str, v3_str;
      iss >> v1_str >> v2_str >> v3_str;

      int v1, vt1, vn1;
      int v2, vt2, vn2;
      int v3, vt3, vn3;

      parse_face_index(v1_str, v1, vt1, vn1);
      parse_face_index(v2_str, v2, vt2, vn2);
      parse_face_index(v3_str, v3, vt3, vn3);

      v1--;
      v2--;
      v3--;

      int base_idx = mesh.positions.size();

      mesh.positions.push_back(temp_positions[v1]);
      mesh.positions.push_back(temp_positions[v2]);
      mesh.positions.push_back(temp_positions[v3]);

      mesh.indices.push_back(base_idx + 0);
      mesh.indices.push_back(base_idx + 1);
      mesh.indices.push_back(base_idx + 2);

      if (vn1 > 0 && vn2 > 0 && vn3 > 0) {
        vn1--;
        vn2--;
        vn3--;
        mesh.normals.push_back(temp_normals[vn1]);
        mesh.normals.push_back(temp_normals[vn2]);
        mesh.normals.push_back(temp_normals[vn3]);
      }
    }
  }
  file.close();

  mesh.set_uniform_color(default_color);

  std::cout << "Loaded OBJ: " << filename << '\n';
  std::cout << "  Vertices: " << mesh.vertex_count() << '\n';
  std::cout << "  Triangles: " << mesh.triangle_count() << '\n';
  std::cout << "  Has normals: " << (mesh.has_normals() ? "yes" : "no") << '\n';

  return mesh;
}

void MeshLoader::parse_face_index(const std::string &token, int &v_idx,
                                  int &vt_idx, int &vn_idx) {
  v_idx = vt_idx = vn_idx = 0;

  std::istringstream iss(token);
  std::string part;

  if (std::getline(iss, part, '/')) {
    v_idx = std::stoi(part);
  }
  if (std::getline(iss, part, '/')) {
    if (!part.empty()) {
      vt_idx = std::stoi(part);
    }
  }
  if (std::getline(iss, part, '/')) {
    if (!part.empty()) {
      vn_idx = std::stoi(part);
    }
  }
}
} // namespace core
