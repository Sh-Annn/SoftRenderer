#pragma once

#if __has_include(<SDL.h>)
#include <SDL.h>
#elif __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#error "SDL header not found. Please add SDL include directory to your build."
#endif

#if __has_include(<SDL_keycode.h>)
#include <SDL_keycode.h>
#elif __has_include(<SDL2/SDL_keycode.h>)
#include <SDL2/SDL_keycode.h>
#endif
