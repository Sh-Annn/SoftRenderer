#pragma once

#include "types.h"

enum class ProjectionType { Perspective, Orthographic };

enum class RenderMode { Solid, WireFrame, Vertex };

struct AppState {
  RenderMode render_mode = RenderMode::Solid;
  bool depth_test_enabled = true;

  ProjectionType projection_type = ProjectionType::Perspective;
  float fov = 45.f;

  Vec3 camera_position = Vec3(0, 0, 5);
  float camera_pitch = 0.f;
  float camera_yaw = -90.f;

  bool request_camera_reset = false;

  bool persp_interp = true;

  bool texture_enabled = false;

  bool light_enabled = true;
  bool diff_enabled = true;
  bool spec_enabled = true;
  Vec3 light_position = {-2.f, 2.f, 2.f};
  float LIGHT_INTENSITY = 1.f;

  bool clipping_enabled = true;
  bool back_face_enabled = true;
};
