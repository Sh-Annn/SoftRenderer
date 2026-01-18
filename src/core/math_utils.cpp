#include "math_utils.h"

namespace core {
namespace math {
mat4 translate(const Vec3 &offset) {
  mat4 result(1.0f);

  // glm column-major
  // glm[col][row]
  // ** col row x y z
  // ** x
  // ** y
  // ** z
  result[3][0] = offset.x;
  result[3][1] = offset.y;
  result[3][2] = offset.z;

  return result;
}
mat4 scale(const Vec3 &scale_factors) {
  mat4 result(1.0f);

  result[0][0] = scale_factors.x;
  result[1][1] = scale_factors.y;
  result[2][2] = scale_factors.z;

  return result;
}
mat4 rotate_x(float angle) {
  mat4 result(1.0f);

  float c = cos(angle);
  float s = sin(angle);

  result[1][1] = c;
  result[2][1] = -s;
  result[1][2] = s;
  result[2][2] = c;

  return result;
}
mat4 rotate_y(float angle) {
  mat4 result(1.0f);

  float c = cos(angle);
  float s = sin(angle);

  result[0][0] = c;
  result[2][0] = s;
  result[0][2] = -s;
  result[2][2] = c;

  return result;
}
mat4 rotate_z(float angle) {
  mat4 result(1.0f);

  float c = cos(angle);
  float s = sin(angle);

  result[0][0] = c;
  result[1][0] = -s;
  result[0][1] = s;
  result[1][1] = c;

  return result;
}

// allways
// eye position (0, 0, 0)
// eye forward -z
mat4 look_at(const Vec3 &eye, const Vec3 &target, const Vec3 &up) {
  Vec3 forward = glm::normalize(eye - target);
  Vec3 right = glm::normalize(glm::cross(up, forward));
  Vec3 camera_up = glm::cross(forward, right);

  // z => forward
  // y => camera_up
  // x => y x z (right)
  // x   y   z
  //
  mat4 rotation(1.0f);
  rotation[0][0] = right.x;
  rotation[1][0] = right.y;
  rotation[2][0] = right.z;

  rotation[0][1] = camera_up.x;
  rotation[1][1] = camera_up.y;
  rotation[2][1] = camera_up.z;

  rotation[0][2] = forward.x;
  rotation[1][2] = forward.y;
  rotation[2][2] = forward.z;

  // move eye to (0, 0, 0)
  mat4 translation(1.0f);
  translation[3][0] = -eye.x;
  translation[3][1] = -eye.y;
  translation[3][2] = -eye.z;

  return rotation * translation;
}

// aspect => rate of width and height
mat4 perspective(float fov, float aspect, float near, float far) {
  // top
  float tan_half_fov = tan(fov / 2.f);

  mat4 result(0.0f);

  result[0][0] = 1.f / (aspect * tan_half_fov);
  result[1][1] = 1.f / tan_half_fov;
  result[2][2] = -(far + near) / (far - near);
  result[2][3] = -1.f;
  result[3][2] = -(2.f * far * near) / (far - near);

  return result;
}
mat4 orthographic(float left, float right, float bottom, float top, float near,
                  float far) {
  mat4 result(1.f);

  result[0][0] = 2.f / (right - left);
  result[1][1] = 2.f / (top - bottom);
  result[2][2] = -2.f / (far - near);

  // -(right + left) * (2 / (right - left))
  result[3][0] = -(right + left) / (right - left);
  result[3][1] = -(top + bottom) / (top - bottom);
  result[3][2] = -(far + near) / (far - near);

  return result;
}

Vec3 barycentric(const Vec2 &p, const Vec2 &a, const Vec2 &b, const Vec2 &c) {
  float beta = ((c.y - a.y) * (p.x - a.x) + (a.x - c.x) * (p.y - a.y)) /
               ((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y));
  float gama = ((a.y - b.y) * (p.x - a.x) + (b.x - a.x) * (p.y - a.y)) /
               ((b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y));
  float alpha = 1 - beta - gama;

  return {alpha, beta, gama};
}

float lerp(float a, float b, float t) { return a + (b - a) * t; }
Vec3 lerp(const Vec3 &a, const Vec3 &b, float t) { return a + (b - a) * t; }

float clamp(float value, float min_val, float max_val) {
  if (value < min_val) {
    return min_val;
  }
  if (value > max_val) {
    return max_val;
  }

  return value;
}
int clamp(int value, int min_val, int max_val) {
  if (value < min_val) {
    return min_val;
  }
  if (value > max_val) {
    return max_val;
  }

  return value;
}
} // namespace math
} // namespace core
