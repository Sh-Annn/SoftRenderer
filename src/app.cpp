#include "app.h"

#include "color.h"

#include "core/mesh_loader.h"
#include "core/texture_loader.h"
#include "ui/ui_layer.h"

#include <SDL2/SDL.h>
#include <iostream>

bool App::init(const char *title, int window_width, int window_height) {
  if (!m_sdl_app.init(title, window_width, window_height)) {
    return false;
  }

  ui::init(m_sdl_app.window(), m_sdl_app.renderer());

  m_rasterizer.Init(RD_WIDTH, RD_HEIGHT);
  m_renderer = new core::Renderer(&m_rasterizer, RD_WIDTH, RD_HEIGHT);

  if (!m_fb_texture.create(m_sdl_app.renderer(), RD_WIDTH, RD_HEIGHT)) {
    return false;
  }

  m_camera = core::Camera(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
  m_camera.set_perspective(45.f, (float)RD_WIDTH / RD_HEIGHT, 0.1f, 100.f);
  m_camera.sync_orthographic_to_perspective(4.f);

  m_mesh = core::MeshLoader::load_obj("../obj/african_head/african_head.obj",
                                      0xFF80FF);

  if (!core::TextureLoader::load("../obj/floor_diffuse.tga", m_texture)) {
    std::cerr << "Failed to load diffuse texture!\n";
    return false;
  }

  m_running = true;
  return true;
}

void App::shutdown() {
  delete m_renderer;
  m_renderer = nullptr;

  m_fb_texture.destroy();
  ui::shutdown();
  m_sdl_app.shutdown();
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
  }

  return m_running;
}

void App::handle_input() {
  if (!ui::is_mouse_in_render_area()) {
    return;
  }
  // forward - backward
  if (m_input.is_key_down(SDL_SCANCODE_W)) {
    std::cout << "W pressed\n";
    m_camera.move_forward(MOVE_SPEED);
  }
  if (m_input.is_key_down(SDL_SCANCODE_S)) {
    std::cout << "S pressed\n";
    m_camera.move_forward(-MOVE_SPEED);
  }

  // left - right
  if (m_input.is_key_down(SDL_SCANCODE_A)) {
    std::cout << "A pressed\n";
    m_camera.move_right(-MOVE_SPEED);
  }
  if (m_input.is_key_down(SDL_SCANCODE_D)) {
    std::cout << "D pressed\n";
    m_camera.move_right(MOVE_SPEED);
  }

  // up - down
  if (m_input.is_key_down(SDL_SCANCODE_SPACE)) {
    std::cout << "SPACE pressed\n";
    m_camera.move_up(MOVE_SPEED);
  }
  if (m_input.is_key_down(SDL_SCANCODE_LSHIFT)) {
    std::cout << "LSHIFT pressed\n";
    m_camera.move_up(-MOVE_SPEED);
  }

  if (m_input.is_mouse_button_down(SDL_BUTTON_RIGHT)) {
    std::cout << "Right mouse: dx=" << m_input.mouse_delta_x()
              << " dy=" << m_input.mouse_delta_y() << '\n';
    float pitch_delta = -m_input.mouse_delta_y() * ROTATE_SPEED;
    float yaw_delta = m_input.mouse_delta_x() * ROTATE_SPEED;
    m_camera.rotate(pitch_delta, yaw_delta);
  }
}

void App::update() { handle_input(); }

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

  // depth test
  m_rasterizer.set_depth_test_enabled(m_app_state.depth_test_enabled);

  if (m_app_state.request_camera_reset) {
    m_camera.reset();
    m_app_state.request_camera_reset = false;
  }

  // m_camera && ui
  m_app_state.camera_position = m_camera.postion();
  m_app_state.camera_pitch = m_camera.pitch();
  m_app_state.camera_yaw = m_camera.yaw();
  m_app_state.fov = m_camera.fov();
}

void App::render() {
  m_rasterizer.clear();

  // fill background - gray
  Color bg_color = colors::Gray;
  for (auto &pixel : m_rasterizer.frame_buffer()) {
    pixel = bg_color;
  }

  // render
  mat4 model(1.f);
  mat4 mvp = m_camera.mvp_matrix(model);

  m_renderer->draw_mesh(m_mesh, mvp, &m_texture);
  // m_renderer->draw_mesh(m_mesh, mvp);

  m_fb_texture.update(m_rasterizer.frame_buffer());

  // app
  m_sdl_app.begin_frame();

  // ui
  ui::begin_frame();
  ui::draw(m_fb_texture.texture(), m_app_state);
  ui::end_frame(m_sdl_app.renderer());

  m_sdl_app.end_frame();
}
