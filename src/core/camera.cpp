#include "camera.h"
#include "math_utils.h"
#include <glm/geometric.hpp>

namespace core {
Camera::Camera(const Vec3 &position, const Vec3 &target, const Vec3 &up)
    : m_position(position), m_initial_position(position),
      m_initial_target(target), m_initial_up(up) {
  m_forward = glm::normalize(target - position);
  m_right = glm::normalize(glm::cross(m_forward, up));
  m_up = glm::normalize(glm::cross(m_right, m_forward));

  m_pitch = math::degrees(asin(m_forward.y));
  m_yaw = math::degrees(atan2(m_forward.z, m_forward.x));
}

void Camera::move_forward(float distance) {
  m_position += m_forward * distance;
}
void Camera::move_right(float distance) { m_position += m_right * distance; }
void Camera::move_up(float distance) { m_position += Vec3(0, 1, 0) * distance; }

/*
 * pitch_delta => left - right
 * yaw_delta => up - down
 */
void Camera::rotate(float pitch_delta, float yaw_delta) {
  m_pitch += pitch_delta;
  m_yaw += yaw_delta;

  if (m_pitch > 89.f) {
    m_pitch = 89.f;
  }
  if (m_pitch < -89.f) {
    m_pitch = -89.f;
  }

  if (m_yaw > 359.f) {
    m_yaw = 0.f;
  }
  if (m_yaw < -359.f) {
    m_yaw = 0.f;
  }

  update_vectors();
}

void Camera::update_vectors() {
  Vec3 front;
  front.x = cos(math::radians(m_yaw)) * cos(math::radians(m_pitch));
  front.y = sin(math::radians(m_pitch));
  front.z = sin(math::radians(m_yaw)) * cos(math::radians(m_pitch));
  m_forward = glm::normalize(front);

  m_right = glm::normalize(glm::cross(m_forward, Vec3(0, 1, 0)));
  m_up = glm::normalize(glm::cross(m_right, m_forward));
}

void Camera::reset() {
  m_position = m_initial_position;
  m_forward = glm::normalize(m_initial_target - m_initial_position);
  m_right = glm::normalize(glm::cross(m_forward, m_initial_up));
  m_up = glm::normalize(glm::cross(m_right, m_forward));

  m_pitch = math::degrees(asin(m_forward.y));
  m_yaw = math::degrees(atan2(m_forward.z, m_forward.x));
}

mat4 Camera::view_matrix() const {
  return math::look_at(m_position, m_position + m_forward, m_up);
}
mat4 Camera::projection_matrix() const {
  if (m_projection_type == ProjectionType::Perspective) {
    return math::perspective(m_fov, m_aspect, m_near, m_far);
  } else {
    return math::orthographic(m_ortho_left, m_ortho_right, m_ortho_bottom,
                              m_ortho_top, m_ortho_near, m_ortho_far);
  }
}
mat4 Camera::mvp_matrix(const mat4 &model) const {
  return projection_matrix() * view_matrix() * model;
}

void Camera::set_perspective(float fov, float aspect, float near, float far) {
  m_fov = fov;
  m_aspect = aspect;
  m_near = near;
  m_far = far;
  m_projection_type = ProjectionType::Perspective;
}
void Camera::set_orthographic(float left, float right, float bottom, float top,
                              float near, float far) {
  m_ortho_left = left;
  m_ortho_right = right;
  m_ortho_bottom = bottom;
  m_ortho_top = top;
  m_ortho_near = near;
  m_ortho_far = far;
  m_projection_type = ProjectionType::Orthographic;
}
void Camera::sync_orthographic_to_perspective(float distance) {
  float half_height = distance * tan(math::radians(m_fov / 2.f));
  float half_width = half_height * m_aspect;

  m_ortho_left = -half_width;
  m_ortho_right = half_width;
  m_ortho_bottom = -half_height;
  m_ortho_top = half_height;
  m_ortho_near = m_near;
  m_ortho_far = m_far;
}
} // namespace core
