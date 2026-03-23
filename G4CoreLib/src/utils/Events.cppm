module;
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

export module GeantCore.Utils.Event;
export namespace GeantCore::Utils {
template <typename... Args> class Event {
public:
  using Handler = std::function<void(Args...)>;
  using HandlerId = std::uint64_t;

  struct Entry {
    std::optional<std::weak_ptr<void>> owner;
    Handler handler;
  };

  template <typename T>
  HandlerId Add(std::shared_ptr<T> owner, Handler handler) {
    std::cout << "OWNER ADD\n";
    HandlerId id = nextId++;
    handlers[id] = Entry{owner, std::move(handler)};
    return id;
  }

  HandlerId Add(Handler handler) {
    std::cout << "NORMAL ADD\n";
    HandlerId id = nextId++;
    handlers[id] = Entry{std::nullopt, std::move(handler)};
    return id;
  }

  void Remove(HandlerId id) { handlers.erase(id); }

  void operator-=(HandlerId id) { Remove(id); }

  void Invoke(Args... args) {
    std::vector<HandlerId> dead;

    for (auto &[id, entry] : handlers) {
      if (!entry.owner.has_value()) {
        std::invoke(entry.handler, args...);
      } else if (!entry.owner->expired()) {
        std::invoke(entry.handler, args...);
      } else {
        dead.push_back(id);
      }
    }

    for (auto id : dead)
      handlers.erase(id);
  }

private:
  std::unordered_map<HandlerId, Entry> handlers;
  HandlerId nextId = 1;
};
} // namespace GeantCore::Utils
