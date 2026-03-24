#include <memory>
import GeantCore.Core.Interfaces.IGeantCoreManager;
import GeantCore.Core.GeantCoreManager;

int main(int argc, char **argv)
{
    std::unique_ptr<GeantCore::Core::GeantCoreManager> GeantManager = std::make_unique<GeantCore::Core::GeantCoreManager>();
    GeantManager->Initialize(argc, argv);

    return 0;
}