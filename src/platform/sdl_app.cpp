#include "sdl_app.h"

#include <iostream>

namespace platform {
bool SdlApp::init(const char *title, int w, int h) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
    return false;
  }

  // create Window and Renderer
  m_window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w,
                       h, SDL_WINDOW_RESIZABLE);
  if (!m_window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
    return false;
  }

  m_renderer = SDL_CreateRenderer(
      m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!m_renderer) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
    return false;
  }

  m_running = true;
  return true;
}
void SdlApp::shutdown() {
  if (m_renderer) {
    SDL_DestroyRenderer(m_renderer);
  }
  if (m_window) {
    SDL_DestroyWindow(m_window);
  }
  m_renderer = nullptr;
  m_window = nullptr;

  SDL_Quit();
}

// bool SdlApp::poll_events() {
//   SDL_Event e;
//   while (SDL_PollEvent(&e)) {
//     if (e.type == SDL_QUIT) {
//       m_running = false;
//     }
//     if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
//     {
//       m_running = false;
//     }
//   }
//
//   return m_running;
// }

void SdlApp::begin_frame() {
  SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
  SDL_RenderClear(m_renderer);
}
void SdlApp::end_frame() { SDL_RenderPresent(m_renderer); }

void SdlApp::window_size(int w, int h) const {}
} // namespace platform
