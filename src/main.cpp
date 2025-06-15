#include "core.h"
#include "liverun.h"

int main(int argc, char *argv[]) {
  livrn::Core app;

  int result = 1;
  try {
    result = app.run(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << "[fatal] Unhandled exception in main: " << e.what()
              << std::endl;
  } catch (...) {
    std::cerr << "[fatal] Unknown exception in main" << std::endl;
  }

  return result;
}
