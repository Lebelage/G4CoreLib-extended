#include <memory>
import GeantCore.Core.GeantCoreManager;

using namespace GeantCore::Core;

int main(int argc, char **argv) {
  auto &GeantManager = GeantCoreManager::GetInstance();
  GeantManager.Initialize(argc, argv);
  GeantManager.InitializeUI(argc, argv);

  return 0;
}
