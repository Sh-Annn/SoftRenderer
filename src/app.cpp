#include "app.h"

#include "core/framebuffer_export.h"
#include "core/mesh_loader.h"
#include "core/texture_loader.h"
#include "ui/ui_layer.h"

#include <SDL2/SDL.h>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <nfd.h>
#include <sstream>
#include <utility>

namespace {
bool open_file_dialog(const nfdu8filteritem_t *fileters, nfdfiltersize_t count,
                      const std::string &current_path,
                      std::string &out_selected_path, std::string &out_status) {
  std::string default_dir;
  if (!current_path.empty()) {
    std::filesystem::path path(current_path);
    default_dir = path.has_parent_path() ? path.parent_path().string() : "";
  }

  nfdu8char_t *picked_path = nullptr;
  const char *default_path =
      default_dir.empty() ? nullptr : default_dir.c_str();
  const nfdresult_t result =
      NFD_OpenDialogU8(&picked_path, fileters, count, default_path);

  if (result == NFD_OKAY) {
    out_selected_path = picked_path;
    NFD_FreePathU8(picked_path);
    return true;
  }
  if (result == NFD_CANCEL) {
    out_status = "Canceled";
    return false;
  }

  const char *error = NFD_GetError();
  out_status = std::string("Dialog error: ") + (error ? error : "unknown");
  return false;
}
} // namespace

bool App::init(const char *title, int window_width, int window_height) {
  if (!m_sdl_app.init(title, window_width, window_height)) {
    return false;
  }

  if (NFD_Init() != NFD_OKAY) {
    const char *error = NFD_GetError();
    std::cerr << "Failed to initialize NFDe: " << (error ? error : "unknown")
              << '\n';
    return false;
  }
  m_nfd_ready = true;

  ui::init(m_sdl_app.window(), m_sdl_app.renderer());

  m_rasterizer.Init(RD_WIDTH, RD_HEIGHT);
  m_renderer = new core::Renderer(&m_rasterizer, RD_WIDTH, RD_HEIGHT);
  m_latest_frame.assign(RD_WIDTH * RD_HEIGHT, 0xFF000000);
  m_last_presented_frame = m_latest_frame;

  if (!m_fb_texture.create(m_sdl_app.renderer(), RD_WIDTH, RD_HEIGHT)) {
    return false;
  }

  m_camera = core::Camera(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
  m_camera.set_perspective(45.f, (float)RD_WIDTH / RD_HEIGHT, 2.f, 100.f);
  m_camera.sync_orthographic_to_perspective(4.f);

  m_mesh = core::MeshLoader::load_obj("../obj/boggie/body.obj", 0xFF80FF);

  if (!core::TextureLoader::load("../obj/boggie/body_diffuse.tga", m_texture)) {
    std::cerr << "Failed to load diffuse texture!\n";
    return false;
  }
  if (!core::TextureLoader::load("../obj/boggie/body_nm_tangent.tga",
                                 m_normal_map)) {
    std::cerr << "Failed to load normal map texture!\n";
    return false;
  }
  m_app_state.io_status = "Default assets loaded";
  start_render_worker();

  m_running = true;
  return true;
}

void App::shutdown() {
  stop_render_worker();
  delete m_renderer;
  m_renderer = nullptr;

  m_fb_texture.destroy();
  ui::shutdown();
  m_sdl_app.shutdown();

  if (m_nfd_ready) {
    NFD_Quit();
    m_nfd_ready = false;
  }
}

bool App::handle_events() {
  m_input.begin_frame();

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    ui::process_event(e);
    m_input.process_event(e);

    if (e.type == SDL_QUIT) {
      m_running = false;
    }
    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
      m_running = false;
    }

    if (e.type == SDL_KEYDOWN && e.key.repeat == 0 &&
        e.key.keysym.scancode == SDL_SCANCODE_F12) {
      m_screenshot_requested = true;
    }
  }

  return m_running;
}

void App::handle_input() {
  if (!ui::is_mouse_in_render_area()) {
    return;
  }
  // forward - backward
  if (m_input.is_key_down(SDL_SCANCODE_W)) {
    // std::cout << "W pressed\n";
    m_camera.move_forward(MOVE_SPEED);
  }
  if (m_input.is_key_down(SDL_SCANCODE_S)) {
    // std::cout << "S pressed\n";
    m_camera.move_forward(-MOVE_SPEED);
  }

  // left - right
  if (m_input.is_key_down(SDL_SCANCODE_A)) {
    // std::cout << "A pressed\n";
    m_camera.move_right(-MOVE_SPEED);
  }
  if (m_input.is_key_down(SDL_SCANCODE_D)) {
    // std::cout << "D pressed\n";
    m_camera.move_right(MOVE_SPEED);
  }

  // up - down
  if (m_input.is_key_down(SDL_SCANCODE_SPACE)) {
    // std::cout << "SPACE pressed\n";
    m_camera.move_up(MOVE_SPEED);
  }
  if (m_input.is_key_down(SDL_SCANCODE_LSHIFT)) {
    // std::cout << "LSHIFT pressed\n";
    m_camera.move_up(-MOVE_SPEED);
  }

  if (m_input.is_mouse_button_down(SDL_BUTTON_RIGHT)) {
    // std::cout << "Right mouse: dx=" << m_input.mouse_delta_x()
    //           << " dy=" << m_input.mouse_delta_y() << '\n';
    float pitch_delta = -m_input.mouse_delta_y() * ROTATE_SPEED;
    float yaw_delta = m_input.mouse_delta_x() * ROTATE_SPEED;
    m_camera.rotate(pitch_delta, yaw_delta);
  }
}

void App::update() {
  handle_input();
  process_file_dialog_requests();
}

void App::process_file_dialog_requests() {
  if (!m_nfd_ready) {
    m_app_state.request_load_model = false;
    m_app_state.request_load_diffuse = false;
    m_app_state.request_load_normal = false;
    m_app_state.io_status = "NFDe is not initialized";
    return;
  }

  if (m_app_state.request_load_model) {
    m_app_state.request_load_model = false;
    static constexpr nfdu8filteritem_t kModelFilters[] = {
        {"Wavefront OBJ:", "obj"}};

    std::string selected_path;
    if (!open_file_dialog(kModelFilters, 1, m_app_state.load_path,
                          selected_path, m_app_state.io_status)) {
      return;
    }

    core::Mesh mesh = core::MeshLoader::load_obj(selected_path, 0xFF80FF);
    if (mesh.vertex_count() == 0 || mesh.triangle_count() == 0) {
      m_app_state.io_status = "Failed to load model";
      return;
    }

    m_mesh = std::move(mesh);
    m_app_state.load_path = selected_path;
    m_app_state.io_status = "Model loaded";
    return;
  }

  if (m_app_state.request_load_diffuse) {
    m_app_state.request_load_diffuse = false;
    static constexpr nfdu8filteritem_t kTextureFilters[] = {
        {"Image files:", "png,jpg,jpeg,tga,bmp"}};

    std::string selected_path;
    if (!open_file_dialog(kTextureFilters, 1, m_app_state.load_path,
                          selected_path, m_app_state.io_status)) {
      return;
    }

    core::Texture texture;
    if (!core::TextureLoader::load(selected_path, texture)) {
      m_app_state.io_status = "Failed to load diffuse texture";
      return;
    }

    m_texture = std::move(texture);
    m_app_state.load_path = selected_path;
    m_app_state.io_status = "Diffuse loaded";
    return;
  }

  if (m_app_state.request_load_normal) {
    m_app_state.request_load_normal = false;
    static constexpr nfdu8filteritem_t kTextureFilters[] = {
        {"Image files:", "png,jpg,jpeg,tga,bmp"}};

    std::string selected_path;
    if (!open_file_dialog(kTextureFilters, 1, m_app_state.load_path,
                          selected_path, m_app_state.io_status)) {
      return;
    }

    core::Texture texture;
    if (!core::TextureLoader::load(selected_path, texture)) {
      m_app_state.io_status = "Failed to load normal map";
      return;
    }

    m_normal_map = std::move(texture);
    m_app_state.load_path = selected_path;
    m_app_state.io_status = "Normal map loaded";
  }
}

void App::sync_state() {
  // FOV
  if (m_camera.fov() != m_app_state.fov) {
    m_camera.set_fov(m_app_state.fov);
  }

  // projection
  core::ProjectionType core_proj =
      (m_app_state.projection_type == ProjectionType::Perspective)
          ? core::ProjectionType::Perspective
          : core::ProjectionType::Orthographic;
  if (m_camera.projection_type() != core_proj) {
    m_camera.set_projection_type(core_proj);
  }

  if (m_app_state.request_camera_reset) {
    m_camera.reset();
    m_app_state.request_camera_reset = false;
  }

  // m_camera && ui
  m_app_state.camera_position = m_camera.postion();
  m_app_state.camera_pitch = m_camera.pitch();
  m_app_state.camera_yaw = m_camera.yaw();
  m_app_state.fov = m_camera.fov();

  RenderJob job;
  job.model = mat4(1.f);
  job.mvp = m_camera.mvp_matrix(job.model);
  job.camera_position = m_camera.postion();
  job.state = m_app_state;
  job.frame_id = m_next_frame_id++;
  submit_render_job(job);
}

void App::render() {
  std::vector<Color> frame;
  if (consume_latest_frame(frame)) {
    m_last_presented_frame = frame;
    m_fb_texture.update(frame);
  }
  save_screenshot_if_requested(m_last_presented_frame);

  // app
  m_sdl_app.begin_frame();

  // ui
  ui::begin_frame();
  ui::draw(m_fb_texture.texture(), m_app_state);
  ui::end_frame(m_sdl_app.renderer());

  m_sdl_app.end_frame();
}

void App::start_render_worker() {
  m_stop_render_worker = false;
  m_render_thread = std::thread(&App::render_worker_loop, this);
}

void App::stop_render_worker() {
  {
    std::lock_guard<std::mutex> lock(m_job_mutex);
    m_stop_render_worker = true;
    m_has_pending_job = false;
  }
  m_job_cv.notify_all();

  if (m_render_thread.joinable()) {
    m_render_thread.join();
  }
}

void App::submit_render_job(const RenderJob &job) {
  {
    std::lock_guard<std::mutex> lock(m_job_mutex);
    m_pending_job = job;
    m_has_pending_job = true;
  }
  m_job_cv.notify_one();
}

bool App::consume_latest_frame(std::vector<Color> &out_frame) {
  std::lock_guard<std::mutex> lock(m_frame_mutex);
  if (!m_has_new_frame) {
    return false;
  }
  out_frame = m_latest_frame;
  m_has_new_frame = false;
  return true;
}

void App::render_worker_loop() {
  while (true) {
    RenderJob job;
    {
      std::unique_lock<std::mutex> lock(m_job_mutex);
      m_job_cv.wait(
          lock, [this] { return m_stop_render_worker || m_has_pending_job; });
      if (m_stop_render_worker) {
        break;
      }
      job = m_pending_job;
      m_has_pending_job = false;
    }

    m_renderer->set_clipping_enabled(job.state.clipping_enabled);
    m_rasterizer.set_depth_test_enabled(job.state.depth_test_enabled);
    m_rasterizer.set_persp_interp_enabled(job.state.persp_interp_enabled);
    m_rasterizer.set_back_face_enabled(job.state.back_face_enabled);
    // m_rasterizer.set_tiled_render_enabled(job.state.tiled_render_enabled);

    m_rasterizer.clear();
    {
      std::shared_lock lock(m_asset_mutex);
      m_renderer->draw_mesh(m_mesh, job.model, job.mvp, job.camera_position,
                            job.state, &m_texture, &m_normal_map);
    }

    {
      std::lock_guard<std::mutex> frame_lock(m_frame_mutex);
      m_latest_frame = m_rasterizer.frame_buffer();
      m_has_new_frame = true;
    }
  }
}

std::string App::next_screenshot_path() {
  std::filesystem::create_directories("captures");

  std::ostringstream oss;
  oss << "captures/frame_" << std::setw(6) << std::setfill('0')
      << m_screenshot_index++ << ".png";
  return oss.str();
}

void App::save_screenshot_if_requested(const std::vector<Color> &frame) {
  if (!m_screenshot_requested) {
    return;
  }
  m_screenshot_requested = false;

  const std::string path = next_screenshot_path();
  const bool ok = core::save_framebuffer_png(frame, RD_WIDTH, RD_HEIGHT, path);

  if (ok) {
    std::cout << "Screenshot saved: " << path << '\n';
  } else {
    std::cerr << "Failed to save screenshot: " << path << '\n';
  }
}
