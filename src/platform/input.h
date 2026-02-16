#pragma once

#include "sdl_include.h"
#include <unordered_set>

namespace platform {
class Input {
public:
  void begin_frame();
  void process_event(const SDL_Event &e);
  bool is_key_down(SDL_Scancode key) const;
  bool is_mouse_button_down(int button) const;

  int mouse_x() const { return m_mouse_x; }
  int mouse_y() const { return m_mouse_y; }

  int mouse_delta_x() const { return m_mouse_dx; }
  int mouse_delta_y() const { return m_mouse_dy; }

private:
  std::unordered_set<SDL_Scancode> m_keys_down;

  int m_mouse_x = 0;
  int m_mouse_y = 0;
  int m_mouse_dx = 0; // per fps
  int m_mouse_dy = 0;
  uint32_t m_mouse_buttons = 0; // SDL which button
};
} // namespace platform
