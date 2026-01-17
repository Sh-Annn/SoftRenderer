#pragma once
#include <SDL2/SDL.h>

namespace ui {

bool init(SDL_Window *window, SDL_Renderer *renderer);
void shutdown();

void begin_frame();
void draw(SDL_Texture *framebuffer_tex);
void end_frame(SDL_Renderer *renderer);

void process_event(const SDL_Event &e);

bool is_mouse_in_render_area();
} // namespace ui
