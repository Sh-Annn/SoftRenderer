#pragma once

#include <SDL2/SDL.h>

namespace platform {
class SdlApp {
public:
  bool init(const char *title, int w, int h);
  void shutdown();

  // bool poll_events(); // is quit? false->quit
  void begin_frame(); // clear
  void end_frame();   // present

  SDL_Window *window() const { return m_window; }
  SDL_Renderer *renderer() const { return m_renderer; }

  void window_size(int w, int h) const;

private:
  SDL_Window *m_window = nullptr;
  SDL_Renderer *m_renderer = nullptr;
  bool m_running = true;
};
} // namespace platform
