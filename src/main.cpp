#include "app.h"

int main() {
  App app;

  if (!app.init("Soft Renderer", 900, 600)) {
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
