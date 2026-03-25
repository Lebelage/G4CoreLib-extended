export module GeantCore.Core.EventManager;
import GeantCore.Utils.Event;
export namespace GeantCore::Core {
using namespace GeantCore::Utils;
class EventManager {
public:
  static Event<> &GetGeometryUpdatedEvent() { return GeometryUpdated; };

private:
  inline static Event<> GeometryUpdated;
};
} // namespace GeantCore::Core