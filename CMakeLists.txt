cmake_minimum_required(VERSION 3.21)
project(soft_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# sdl2 -------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# ImGui ------------------------------------------------------
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
	${IMGUI_DIR}
	${IMGUI_DIR}/backends
	${SDL2_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
)

target_compile_options(imgui PRIVATE ${SDL2_CFLAGS_OTHER})

# core library -----------------------------------------------
add_library(core STATIC
	${CMAKE_SOURCE_DIR}/src/core/rasterizer.cpp
)
target_include_directories(core PUBLIC
	${CMAKE_SOURCE_DIR}/src/core
	${CMAKE_SOURCE_DIR}/src
)

# platform layer ---------------------------------------------
add_library(platform STATIC
	${CMAKE_SOURCE_DIR}/src/platform/sdl_app.cpp
	${CMAKE_SOURCE_DIR}/src/platform/sdl_texture.cpp
)
target_include_directories(platform PUBLIC
	${CMAKE_SOURCE_DIR}/src/platform
	${SDL2_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(platform PUBLIC ${SDL2_LIBRARIES})
target_compile_options(platform PUBLIC ${SDL2_CFLAGS_OTHER})

# ui ---------------------------------------------------------
add_library(ui STATIC
	src/ui/ui_layer.cpp
)
target_include_directories(ui PUBLIC
	${CMAKE_SOURCE_DIR}/src/ui
	${SDL2_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(ui PUBLIC
	imgui
	${SDL2_LIBRARIES}
	# core
)
target_compile_options(ui PRIVATE ${SDL2_CFLAGS_OTHER})

# main ------------------------------------------------------
add_executable(soft_renderer
	src/main.cpp
	# src/app/app.cpp
)
target_include_directories(soft_renderer PRIVATE
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(soft_renderer PRIVATE
	platform
	core
	ui
)

