#include "ui_layer.h"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "imgui.h"

namespace ui {
struct Fonts {
  ImFont *normal;
  ImFont *title;
};
Fonts g_fonts;

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

void draw(SDL_Texture *framebuffer_tex) {
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

  // 2) 右侧调试固定宽度
  const float debug_w = 360.0f; // 你要的固定宽度
  const float spacing = ImGui::GetStyle().ItemSpacing.x;

  // 左：渲染区域（宽度 = 剩余）
  ImGui::PushFont(g_fonts.title);
  ImGui::BeginChild("RenderPanel", ImVec2(-debug_w - spacing, 0), true,
                    ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoScrollWithMouse);

  // ——这里放你的“渲染画面显示区域”——
  // 如果你目前是 SDL 直接把 framebuffer present 到窗口：
  //   你仍然可以在这里放交互逻辑/提示文字/未来换成 ImGui::Image 显示 fb texture
  ImGui::Text("Render Viewport");
  ImGui::PopFont();
  ImGui::Separator();

  // 若你准备把 framebuffer 转成 SDL_Texture* 显示到 ImGui（推荐未来做）：
  // ImVec2 avail = ImGui::GetContentRegionAvail();
  // ImGui::Image((ImTextureID)fbTex, avail);
  if (framebuffer_tex) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)framebuffer_tex, avail);
  }

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

  float text = 1.0f;
  ImGui::SliderFloat("text111111111", &text, 1.f, 30.f);

  ImGui::PopFont();
  ImGui::EndChild();

  ImGui::End();

  ImGui::PopStyleVar(3);
}

void end_frame(SDL_Renderer *renderer) {
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}
} // namespace ui
