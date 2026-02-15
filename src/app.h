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

#include <string>

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

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
  struct RenderJob {
    mat4 model = mat4(1.f);
    mat4 mvp = mat4(1.f);
    Vec3 camera_position = Vec3(0.f);
    AppState state;
    uint64_t frame_id = 0;
  };

  void handle_input();
  void process_file_dialog_requests();
  void start_render_worker();
  void stop_render_worker();
  void render_worker_loop();
  void submit_render_job(const RenderJob &job);
  bool consume_latest_frame(std::vector<Color> &out_frame);
  std::string next_screenshot_path();
  void save_screenshot_if_requested(const std::vector<Color> &frame);

  static constexpr int RD_WIDTH = 900;
  static constexpr int RD_HEIGHT = 600;
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
  core::Texture m_normal_map;

  // file manager
  bool m_screenshot_requested = false;
  int m_screenshot_index = 0;
  bool m_nfd_ready = false;

  std::shared_mutex m_asset_mutex;

  std::thread m_render_thread;
  std::mutex m_job_mutex;
  std::condition_variable m_job_cv;
  RenderJob m_pending_job;
  bool m_has_pending_job = false;
  bool m_stop_render_worker = false;
  uint64_t m_next_frame_id = 1;

  std::mutex m_frame_mutex;
  std::vector<Color> m_latest_frame;
  std::vector<Color> m_last_presented_frame;
  bool m_has_new_frame = false;
};
