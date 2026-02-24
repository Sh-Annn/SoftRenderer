#include "app.h"
#include "platform/sdl_include.h"

int main(int argc, char* argv[]) {
  App app;

  if (!app.init("Soft Renderer", 1300, 600)) {
    return 1;
  }

  while (app.handle_events()) {
    app.update();
    app.sync_state();
    app.render();
  }

  app.shutdown();
  return 0;
}
