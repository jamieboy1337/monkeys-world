#include <input/WindowEventManager.hpp>

#include <boost/log/trivial.hpp>

#include <engine/Context.hpp>
#include <engine/Scene.hpp>

#include <shared_mutex>

namespace monkeysworld {
namespace input {

utils::IDGenerator WindowEventManager::event_desc_generator_;

WindowEventManager::WindowEventManager(GLFWwindow* window, engine::Context* ctx) {
  glfwSetWindowUserPointer(window, this);
  // lambdas are equiv. to fptrs if there are no captures
  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    WindowEventManager* that =
      reinterpret_cast<WindowEventManager*>(glfwGetWindowUserPointer(window));
    that->GenerateKeyEvent(window, key, scancode, action, mods);
  });

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
    WindowEventManager* that =
      reinterpret_cast<WindowEventManager*>(glfwGetWindowUserPointer(window));
    that->GenerateClickEvent(window, button, action, mods);
  });

  ctx_ = ctx;
  window_ = window;

  cursor_ = std::make_shared<Cursor>(window);
}

std::shared_ptr<Cursor> WindowEventManager::GetCursor() {
  return cursor_;
}

void WindowEventManager::GenerateKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods) {
  std::unique_lock<std::shared_timed_mutex>(event_mutex_);
  event_queue_.push_back({window, key, scancode, action, mods});
}

void WindowEventManager::GenerateClickEvent(GLFWwindow* window, int button, int action, int mods) {
  std::unique_lock<std::shared_timed_mutex>(event_mutex_);
  event_queue_.push_back({window, button, -1, action, mods});
}

void WindowEventManager::ProcessWaitingEvents() {
  cursor_->UpdateCursorPosition();
  std::shared_lock<std::shared_timed_mutex>(callback_mutex_);
  std::vector<event_info> events;
  {
    std::unique_lock<std::shared_timed_mutex>(event_mutex_);
    events = event_queue_;
    event_queue_.clear();
  }

  for (auto event : events) {
    if (event.scancode < 0) {
      // mouse event
      double x_pos;
      double y_pos;
      MouseEvent e;
      // prepare mouse event
      glfwGetCursorPos(window_, &x_pos, &y_pos);
      e.absolute_pos.x = x_pos;
      e.absolute_pos.y = y_pos;
      e.button = event.key;
      e.action = event.action;
      e.mods = event.mods;

      for (auto callback : mouse_callbacks_) {
        callback.second(e);
      }

      e.local_pos = e.absolute_pos;
      auto ui_root = ctx_->GetScene()->GetUIObjectRoot();
      e.local_pos -= ui_root->GetPosition();
      auto ui_dims = ui_root->GetDimensions();
      if (e.local_pos.x >= 0 && e.local_pos.y >= 0
       && e.local_pos.x < ui_dims.x && e.local_pos.y < ui_dims.y
       && !cursor_->IsCursorLocked()) {
        ui_root->HandleClickEvent(e);
      }
    } else {
      // key event
      auto callbacks = callbacks_.find(event.key);
      if (callbacks != callbacks_.end()) {
        std::shared_lock<std::shared_timed_mutex>(callbacks->second->set_lock);
        for (auto callback : callbacks->second->callbacks) {
          callback.second(event.key, event.action, event.mods);
        }
      }
    }
  }
}

uint64_t WindowEventManager::RegisterKeyListener(int key, std::function<void(int, int, int)> callback) {
  std::unique_lock<std::shared_timed_mutex>(callback_mutex_);
  auto callbacks = callbacks_.find(key);
  std::shared_ptr<key_callback_info> info_ptr;
  if (callbacks == callbacks_.end()) {
    info_ptr = std::make_shared<key_callback_info>();
    callbacks_.insert(std::make_pair(key, info_ptr));
  } else {
    info_ptr = callbacks->second;
  }

  uint64_t listener_id = event_desc_generator_.GetUniqueId();
  callback_to_key_.insert(std::make_pair(listener_id, key));
  std::unique_lock<std::shared_timed_mutex>(info_ptr->set_lock);
  info_ptr->callbacks.insert(std::make_pair(listener_id, callback));
  return listener_id;
}

uint64_t WindowEventManager::RegisterClickListener(std::function<void(MouseEvent)> callback) {
  // use callback mutex to add to mouse_callbacks.
  std::unique_lock<std::shared_timed_mutex>(callback_mutex_);
  uint64_t listener_id = event_desc_generator_.GetUniqueId();
  mouse_callbacks_.insert(std::make_pair(listener_id, callback));
  return listener_id;
}

bool WindowEventManager::RemoveKeyListener(uint64_t event_id) {
  std::unique_lock<std::shared_timed_mutex>(callback_mutex_);
  auto entry = callback_to_key_.find(event_id);
  if (entry == callback_to_key_.end()) {
    return false;
  }

  int key = entry->second;
  auto info_entry = callbacks_.find(key);
  if (info_entry == callbacks_.end()) {
    // assert?
    BOOST_LOG_TRIVIAL(error) << "Invalid state reached in WindowEventManager: info not found for callback";
    return false;
  }

  std::unique_lock<std::shared_timed_mutex>(info_entry->second->set_lock);
  info_entry->second->callbacks.erase(event_id);
  if (info_entry->second->callbacks.size() == 0) {
    BOOST_LOG_TRIVIAL(info) << "No more callbacks associated with key " << key << ". Erasing...";
    callbacks_.erase(key);
  }

  return true;
}

bool WindowEventManager::RemoveClickListener(uint64_t event_id) {
  // remove from mouse_callbacks
  std::unique_lock<std::shared_timed_mutex>(callback_mutex_);
  auto entry = mouse_callbacks_.find(event_id);
  if (entry != mouse_callbacks_.end()) {
    mouse_callbacks_.erase(entry);
    return true;
  }

  return false;
}

} // namespace input
} // namespace monkeysworld