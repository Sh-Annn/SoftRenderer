#pragma once

#include "../types.h"

namespace core {
namespace math {
mat4 translate(const Vec3 &offset);
mat4 scale(const Vec3 &scale_factors);
mat4 rotate_x(float angle);
mat4 rotate_y(float angle);
mat4 rotate_z(float angle);

mat4 look_at(const Vec3 &eye, const Vec3 &target, const Vec3 &up);
mat4 perspective(float fov, float aspect, float near, float far);
mat4 orthographic(float left, float right, float bottom, float top, float near,
                  float far);

Vec3 barycentric(const Vec2 &p, const Vec2 &a, const Vec2 &b, const Vec2 &c);

float lerp(float a, float b, float t);
Vec3 lerp(const Vec3 &a, const Vec3 &b, float t);

float clamp(float value, float min_val, float max_val);
int clamp(int value, int min_val, int max_val);

inline float radians(float degrees) { return degrees * 3.1415926535f / 180.f; }
inline float degrees(float radians_val) {
  return radians_val * 180.f / 3.1415926535f;
}
} // namespace math
} // namespace core
