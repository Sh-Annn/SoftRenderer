#pragma once

#include "sdl_include.h"

namespace platform {
class SdlApp {
public:
  bool init(const char *title, int w, int h);
  void shutdown();

  void begin_frame(); // clear
  void end_frame();   // present

  SDL_Window *window() const { return m_window; }
  SDL_Renderer *renderer() const { return m_renderer; }

  void window_size(int w, int h) const;

private:
  SDL_Window *m_window = nullptr;
  SDL_Renderer *m_renderer = nullptr;
};
} // namespace platform
