#pragma once

#include "../types.h"

namespace core {
enum class ProjectionType { Perspective, Orthographic };

class Camera {
public:
  Camera(const Vec3 &position = Vec3(0, 0, 3),
         const Vec3 &target = Vec3(0, 0, 0), const Vec3 &up = Vec3(0, 1, 0));

  void move_forward(float distance);
  void move_right(float distance);
  void move_up(float distance);

  /*
   * pitch_delta => left - right
   * yaw_delta => up - down
   */
  void rotate(float pitch_delta, float yaw_delta);

  mat4 view_matrix() const;
  mat4 projection_matrix() const;
  mat4 mvp_matrix(const mat4 &model) const;

  void set_perspective(float fov, float aspect, float near, float far);
  void set_orthographic(float left, float right, float bottom, float top,
                        float near, float far);
  void sync_orthographic_to_perspective(float distance);

  void set_projection_type(ProjectionType type) { m_projection_type = type; }
  ProjectionType projection_type() const { return m_projection_type; }

  void reset();
  void set_fov(float fov) { m_fov = fov; }

  const Vec3 &postion() const { return m_position; }
  const Vec3 &forward() const { return m_forward; }
  const Vec3 &right() const { return m_right; }
  const Vec3 &up() const { return m_up; }

  float pitch() const { return m_pitch; }
  float yaw() const { return m_yaw; }

  float fov() const { return m_fov; }
  float aspect() const { return m_aspect; }

private:
  void update_vectors();

  Vec3 m_position;
  Vec3 m_forward;
  Vec3 m_right;
  Vec3 m_up;

  float m_pitch = 0.0f;
  float m_yaw = -90.f; // init forward to -z

  ProjectionType m_projection_type = ProjectionType::Perspective;

  float m_fov = 45.f;
  float m_aspect = 1.f;
  float m_near = 0.1f;
  float m_far = 100.f;

  // orthographic scale
  float m_ortho_left = -2.f;
  float m_ortho_right = 2.f;
  float m_ortho_bottom = -2.f;
  float m_ortho_top = 2.f;
  float m_ortho_near = 0.1f;
  float m_ortho_far = 100.f;

  Vec3 m_initial_position;
  Vec3 m_initial_target;
  Vec3 m_initial_up;
};
} // namespace core
