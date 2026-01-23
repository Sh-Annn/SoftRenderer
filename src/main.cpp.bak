#include "SDL_scancode.h"
#include "core/camera.h"
#include "core/mesh.h"
#include "core/mesh_loader.h"
#include "core/rasterizer.h"
#include "core/renderer.h"
#include "core/texture.h"
#include "core/texture_loader.h"
#include "platform/input.h"
#include "platform/sdl_app.h"
#include "platform/sdl_texture.h"
#include "ui/ui_layer.h"

#include <SDL2/SDL.h>
#include <iostream>

#include "app_state.h"
#include "color.h"

int main() {
  platform::SdlApp app;
  if (!app.init("Soft Renderer (minimal)", 900, 600)) {
    return 1;
  }

  AppState app_state;

  ui::init(app.window(), app.renderer());

  const int fb_width = 500;
  const int fb_height = 500;

  core::Rasterizer rasterizer;
  rasterizer.Init(fb_width, fb_height);

  core::Renderer renderer(&rasterizer, fb_width, fb_height);

  core::Mesh cube_mesh = core::MeshLoader::load_obj(
      "../obj/diablo3_pose/diablo3_pose.obj", 0xFF80FF);
  core::Texture diffuse_texture;
  if (!core::TextureLoader::load("../obj/diablo3_pose/diablo3_pose_diffuse.tga",
                                 diffuse_texture)) {
    std::cerr << "Faild to load diffuse texture!\n";
    return 1;
  }

  platform::Sdl_Texture fb_texture;
  if (!fb_texture.create(app.renderer(), fb_width, fb_height)) {
    return 1;
  }

  platform::Input input;

  core::Camera camera(Vec3(0, 0, 5), Vec3(0, 0, 0), Vec3(0, 1, 0));
  // camera.set_perspective(45.f, (float)fb_width / fb_height, 0.1f, 100.f);
  camera.set_perspective(45.f, (float)fb_width / fb_height, 0.1f, 100.f);
  camera.sync_orthographic_to_perspective(4.f);

  bool running = true;
  while (running) {
    input.begin_frame();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      ui::process_event(e);
      input.process_event(e);

      if (e.type == SDL_QUIT) {
        running = false;
      }
      if (e.type == SDL_WINDOWEVENT &&
          e.window.event == SDL_WINDOWEVENT_CLOSE) {
        running = false;
      }
    }

    if (ui::is_mouse_in_render_area()) {
      const float move_speed = 0.05f;
      const float rotate_speed = 0.2f;

      // forward - backward
      if (input.is_key_down(SDL_SCANCODE_W)) {
        std::cout << "W pressed\n";
        camera.move_forward(move_speed);
      }
      if (input.is_key_down(SDL_SCANCODE_S)) {
        std::cout << "S pressed\n";
        camera.move_forward(-move_speed);
      }

      // left - right
      if (input.is_key_down(SDL_SCANCODE_A)) {
        std::cout << "A pressed\n";
        camera.move_right(-move_speed);
      }
      if (input.is_key_down(SDL_SCANCODE_D)) {
        std::cout << "D pressed\n";
        camera.move_right(move_speed);
      }

      // up - down
      if (input.is_key_down(SDL_SCANCODE_SPACE)) {
        std::cout << "SPACE pressed\n";
        camera.move_up(move_speed);
      }
      if (input.is_key_down(SDL_SCANCODE_LSHIFT)) {
        std::cout << "LSHIFT pressed\n";
        camera.move_up(-move_speed);
      }
      if (input.is_mouse_button_down(SDL_BUTTON_RIGHT)) {
        std::cout << "Right mouse: dx=" << input.mouse_delta_x()
                  << " dy=" << input.mouse_delta_y() << '\n';
        float pitch_delta = -input.mouse_delta_y() * rotate_speed;
        float yaw_delta = input.mouse_delta_x() * rotate_speed;
        camera.rotate(pitch_delta, yaw_delta);
      }
    }

    // FOV
    if (camera.fov() != app_state.fov) {
      camera.set_fov(app_state.fov);
    }

    // projection
    core::ProjectionType core_proj =
        (app_state.projection_type == ProjectionType::Perspective)
            ? core::ProjectionType::Perspective
            : core::ProjectionType::Orthographic;
    if (camera.projection_type() != core_proj) {
      camera.set_projection_type(core_proj);
    }

    // depth test
    rasterizer.set_depth_test_enabled(app_state.depth_test_enabled);

    if (app_state.request_camera_reset) {
      camera.reset();
      app_state.request_camera_reset = false;
    }

    // camera && ui
    app_state.camera_position = camera.postion();
    app_state.camera_pitch = camera.pitch();
    app_state.camera_yaw = camera.yaw();
    app_state.fov = camera.fov();

    rasterizer.clear();

    // fill background - gray
    Color bg_color = colors::Gray;
    for (auto &pixel : rasterizer.frame_buffer()) {
      pixel = bg_color;
    }

    // render
    mat4 model(1.f);
    mat4 mvp = camera.mvp_matrix(model);

    renderer.draw_mesh(cube_mesh, mvp, &diffuse_texture);

    fb_texture.update(rasterizer.frame_buffer());

    // app
    app.begin_frame();

    // ui
    ui::begin_frame();
    ui::draw(fb_texture.texture(), app_state);
    ui::end_frame(app.renderer());

    app.end_frame();
  }

  fb_texture.destroy();
  ui::shutdown();
  app.shutdown();
  return 0;
}
