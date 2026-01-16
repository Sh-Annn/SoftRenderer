#include "core/rasterizer.h"
#include "platform/sdl_app.h"
#include "ui/ui_layer.h"

#include <SDL2/SDL.h>

int main() {
  platform::SdlApp app;
  if (!app.init("Soft Renderer (minimal)", 900, 600)) {
    return 1;
  }

  ui::UiState ui_state;
  ui::init(app.window(), app.renderer());

  core::Rasterizer rasterizer;
  rasterizer.Init(500, 600);

  bool running = true;
  while (running) {
    // event
    // running = app.poll_events();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
      }
      if (e.type == SDL_WINDOWEVENT &&
          e.window.event == SDL_WINDOWEVENT_CLOSE) {
        running = false;
      }
    }

    // app
    app.begin_frame();

    // ui
    ui::begin_frame();
    ui::draw(ui_state);
    ui::end_frame(app.renderer());

    app.end_frame();
  }

  ui::shutdown();
  app.shutdown();
  return 0;
}
