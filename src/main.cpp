#include "core/rasterizer.h"
#include "platform/sdl_app.h"
#include "platform/sdl_texture.h"
#include "sdl_texture.h"
#include "ui/ui_layer.h"

#include <SDL2/SDL.h>

constexpr Color make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
  return (a << 24) | (r << 16) | (g << 8) | b;
}

int main() {
  platform::SdlApp app;
  if (!app.init("Soft Renderer (minimal)", 900, 600)) {
    return 1;
  }

  ui::init(app.window(), app.renderer());

  const int fb_width = 500;
  const int fb_height = 500;

  core::Rasterizer rasterizer;
  rasterizer.Init(fb_width, fb_height);

  platform::Sdl_Texture fb_texture;
  if (!fb_texture.create(app.renderer(), fb_width, fb_height)) {
    return 1;
  }

  bool running = true;
  while (running) {
    // event
    // running = app.poll_events();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      ui::process_event(e);
      if (e.type == SDL_QUIT) {
        running = false;
      }
      if (e.type == SDL_WINDOWEVENT &&
          e.window.event == SDL_WINDOWEVENT_CLOSE) {
        running = false;
      }
    }

    rasterizer.clear();

    // fill background - gray
    Color bg_color = make_color(30, 30, 30);
    for (auto &pixel : rasterizer.frame_buffer()) {
      pixel = bg_color;
    }

    Vec3 v0(250, 100, 0);
    Vec3 v1(100, 400, 0);
    Vec3 v2(400, 400, 0);
    rasterizer.draw_filled_triangle(v0, v1, v2, make_color(255, 0, 0));

    fb_texture.update(rasterizer.frame_buffer());

    // app
    app.begin_frame();

    // ui
    ui::begin_frame();
    ui::draw(fb_texture.texture());
    ui::end_frame(app.renderer());

    app.end_frame();
  }

  fb_texture.destroy();
  ui::shutdown();
  app.shutdown();
  return 0;
}
