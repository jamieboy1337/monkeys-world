#include <engine/Context.hpp>

namespace monkeysworld {
namespace engine {

using file::CachedFileLoader;
using input::WindowEventManager;
using audio::AudioManager;

Context::Context(GLFWwindow* window) {
  file_loader_ = std::make_shared<CachedFileLoader>("context_cache");
  event_mgr_ = std::make_shared<WindowEventManager>(window);
  audio_mgr_ = std::make_shared<AudioManager>();
  window_ = window;
}

void Context::GetFramebufferSize(int* width, int* height) {
  glfwGetFramebufferSize(window_, width, height);
}

const std::shared_ptr<CachedFileLoader> Context::GetCachedFileLoader() {
  return file_loader_;
}

const std::shared_ptr<WindowEventManager> Context::GetEventManager() {
  return event_mgr_;
}

const std::shared_ptr<AudioManager> Context::GetAudioManager() {
  return audio_mgr_;
}

std::shared_ptr<engine::Scene> Context::GetNextScene() {
  // nothing yet
  return std::shared_ptr<engine::Scene>();
}

double Context::GetDeltaTime() {
  return frame_delta_;
}

void Context::FrameUpdate() {
  event_mgr_->ProcessWaitingEvents();
}


} // namespace critter
} // namespace monkeysworld