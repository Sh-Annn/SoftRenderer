#pragma once

#include "app_state.h"

#include "core/camera.h"
#include "core/mesh.h"
#include "core/rasterizer.h"
#include "core/renderer.h"
#include "core/texture.h"

#include "platform/input.h"
#include "platform/sdl_app.h"
#include "platform/sdl_texture.h"

class App {
public:
  App() = default;
  ~App() = default;

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  bool init(const char *title, int window_width, int window_height);
  void shutdown();

  bool handle_events();
  void update();
  void sync_state();
  void render();

  bool is_running() const { return m_running; }

private:
  void handle_input();

  static constexpr int RD_WIDTH = 500;
  static constexpr int RD_HEIGHT = 500;
  static constexpr float MOVE_SPEED = 0.05f;
  static constexpr float ROTATE_SPEED = 0.2f;

  bool m_running = false;
  AppState m_app_state;

  platform::SdlApp m_sdl_app;
  platform::Sdl_Texture m_fb_texture;
  platform::Input m_input;

  core::Rasterizer m_rasterizer;
  core::Renderer *m_renderer = nullptr;
  core::Camera m_camera;

  core::Mesh m_mesh;
  core::Texture m_texture;
};
