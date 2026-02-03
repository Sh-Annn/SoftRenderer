#include "ui_layer.h"

#include "../app_state.h"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "imgui.h"

namespace ui {
struct Fonts {
  ImFont *normal;
  ImFont *title;
};
Fonts g_fonts;

static bool g_render_panel_hovered = false;

void InitFonts(float dpi_scale) {
  ImGuiIO &io = ImGui::GetIO();

  ImFontConfig cfg;
  cfg.SizePixels = 8.0f * dpi_scale;
  g_fonts.normal = io.Fonts->AddFontDefault(&cfg);

  cfg.SizePixels = 12.0f * dpi_scale;
  g_fonts.title = io.Fonts->AddFontDefault(&cfg);
}

bool init(SDL_Window *window, SDL_Renderer *renderer) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  InitFonts(1.f);

  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  return true;
}

void shutdown() {
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void process_event(const SDL_Event &e) { ImGui_ImplSDL2_ProcessEvent(&e); }

void begin_frame() {
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void draw(SDL_Texture *framebuffer_tex, AppState &state) {
  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 2.f;

  // 1) 主布局窗口：全屏、不可移动/缩放/折叠
  const ImGuiWindowFlags layout_flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));

  ImGui::Begin("##MainLayout", nullptr, layout_flags);

  // 2 右侧调试固定宽度
  const float debug_w = 400.0f; // 固定宽度
  const float spacing = ImGui::GetStyle().ItemSpacing.x;

  // 左：渲染区域（宽度 = 剩余）
  ImGui::PushFont(g_fonts.title);
  ImGui::BeginChild("RenderPanel", ImVec2(-debug_w - spacing, 0), true,
                    ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoScrollWithMouse);

  ImGui::Text("Render Viewport");
  ImGui::PopFont();
  ImGui::Separator();

  // 把 framebuffer 转成 SDL_Texture* 显示到 ImGui：
  if (framebuffer_tex) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)framebuffer_tex, avail);
  }

  g_render_panel_hovered = ImGui::IsWindowHovered();

  ImGui::EndChild();

  ImGui::SameLine();

  // 右：调试面板（固定宽度）
  ImGui::PushFont(g_fonts.title);
  ImGui::BeginChild("DebugPanel", ImVec2(debug_w, 0), true,
                    ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_AlwaysVerticalScrollbar);

  ImGui::Text("Debug");
  ImGui::PopFont();

  ImGui::PushFont(g_fonts.normal);
  ImGui::Separator();

  // information panel
  if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.f / io.Framerate);
  }
  // camera
  if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Position:");
    ImGui::Indent();
    ImGui::Text("X: %.2f", state.camera_position.x);
    ImGui::Text("Y: %.2f", state.camera_position.y);
    ImGui::Text("Z: %.2f", state.camera_position.z);
    ImGui::Unindent();

    ImGui::Separator();

    ImGui::Text("Rotation:");
    ImGui::Indent();
    ImGui::Text("Pitch: %.1f°", state.camera_pitch);
    ImGui::Text("Yaw: %.1f°", state.camera_yaw);
    ImGui::Unindent();

    ImGui::Separator();

    // fov
    ImGui::SliderFloat("FOV", &state.fov, 10.f, 90.f, "%.1f°");

    ImGui::Separator();

    if (ImGui::Button("Reset Camera", ImVec2(-1, 0))) {
      state.request_camera_reset = true;
    }
  }
  // render settings
  if (ImGui::CollapsingHeader("Render Settings",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    const char *projection_types[] = {"Perspective", "Orthographic"};
    int current_projection =
        (state.projection_type == ProjectionType::Perspective) ? 0 : 1;
    if (ImGui::Combo("Projection", &current_projection, projection_types, 2)) {
      state.projection_type = (current_projection == 0)
                                  ? ProjectionType::Perspective
                                  : ProjectionType::Orthographic;
    }

    ImGui::Separator();

    const char *render_modes[] = {"Solid", "Wireframe", "Vertex"};
    int current_mode = static_cast<int>(state.render_mode);
    if (ImGui::Combo("Render Mode", &current_mode, render_modes, 3)) {
      state.render_mode = static_cast<RenderMode>(current_mode);
    }

    ImGui::Separator();

    ImGui::Checkbox("Depth Test", &state.depth_test_enabled);
    if (!state.depth_test_enabled) {
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "(!)");
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Depth test disabled - triangles may overlap incorrectly");
      }
    }

    ImGui::Separator();
    ImGui::Checkbox("perspctive interp", &state.persp_interp);
    // if (!state.depth_test_enabled) {
    //   ImGui::SameLine();
    //   ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "(!)");
    //   if (ImGui::IsItemHovered()) {
    //     ImGui::SetTooltip(
    //         "Depth test disabled - triangles may overlap incorrectly");
    //   }
    // }

    ImGui::Separator();
    ImGui::Checkbox("texture enabled", &state.texture_enabled);
  }

  ImGui::PopFont();
  ImGui::EndChild();

  ImGui::End();

  ImGui::PopStyleVar(3);
}

void end_frame(SDL_Renderer *renderer) {
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

bool is_mouse_in_render_area() { return g_render_panel_hovered; }
} // namespace ui
