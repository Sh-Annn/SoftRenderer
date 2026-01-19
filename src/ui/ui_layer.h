#pragma once
#include <SDL2/SDL.h>

namespace core {
class Camera;
class Rasterizer;
} // namespace core

namespace ui {

struct UiState {
  bool depth_test_enabled = true;
};

bool init(SDL_Window *window, SDL_Renderer *renderer);
void shutdown();

void begin_frame();
void draw(SDL_Texture *framebuffer_tex, UiState &state, core::Camera *camera,
          core::Rasterizer *rasterizer);
void end_frame(SDL_Renderer *renderer);

void process_event(const SDL_Event &e);

bool is_mouse_in_render_area();
} // namespace ui
