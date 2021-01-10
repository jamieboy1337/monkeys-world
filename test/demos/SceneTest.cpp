// test for game engine methods. creates a basic scene

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <boost/log/trivial.hpp>

#include <engine/BaseEngine.hpp>
#include <engine/Scene.hpp>
#include <engine/RenderContext.hpp>

#include <engine/Context.hpp>
#include <critter/Empty.hpp>
#include <critter/GameCamera.hpp>
#include <critter/GameObject.hpp>
#include <critter/Model.hpp>

#include <font/TextObject.hpp>

#include <shader/light/SpotLight.hpp>

#include <shader/materials/MatteMaterial.hpp>

#include <glm/gtx/euler_angles.hpp>

// create everything inline -- its pretty lazy

using ::monkeysworld::engine::Scene;
using ::monkeysworld::engine::RenderContext;
using namespace ::monkeysworld::engine::baseengine;

using ::monkeysworld::engine::Context;
using ::monkeysworld::critter::Empty;
using ::monkeysworld::critter::GameCamera;
using ::monkeysworld::critter::GameObject;
using ::monkeysworld::critter::Object;
using ::monkeysworld::critter::Model;

using ::monkeysworld::critter::camera_info;

using ::monkeysworld::shader::light::SpotLight;

using ::monkeysworld::shader::light::spotlight_info;

using ::monkeysworld::shader::materials::MatteMaterial;

using ::monkeysworld::audio::AudioFiletype;

using ::monkeysworld::font::TextObject;

class RatModel2 : public Model {
 public:
  RatModel2(Context* ctx) : Model(ctx), m(ctx) {
    SetMesh(ctx->GetCachedFileLoader()->LoadModel("resources/test/untitled4.obj"));
  }

  void RenderMaterial(const RenderContext& rc) override {
    glm::mat4 tf_matrix = GetTransformationMatrix();
    camera_info cam = rc.GetActiveCamera();
    // matte material doesn't accept spotlights!
    // TODO: modify material to accept different types of lights
    m.SetSpotlights(rc.GetSpotlights());
    spotlight_info i = rc.GetSpotlights()[0];
    m.SetModelTransforms(tf_matrix);
    m.SetCameraTransforms(cam.view_matrix);
    m.SetSurfaceColor(glm::vec4(0.0, 1.0, 0.0, 1.0));
    m.UseMaterial();
    Draw();
  }
 private:
  MatteMaterial m;
};

class RatModel : public Model {
 public:
  RatModel(Context* ctx) : Model(ctx), rot_(0), m(ctx) {
    SetMesh(ctx->GetCachedFileLoader()->LoadModel("resources/test/rat/Rat.obj"));
    ctx->GetAudioManager()->AddFileToBuffer("resources/chamberofreflection.ogg", AudioFiletype::OGG);
    // create a key listener which accomplishes rat motion
    // or just rotate consistently with time
  }

  void Update() override {
    rot_ += rot_inc_ * (GetContext()->GetDeltaTime());
    SetRotation(glm::vec3(0.0, rot_, 0.0));
    auto gc = std::dynamic_pointer_cast<GameCamera>(GetActiveCamera());
  }

  void RenderMaterial(const RenderContext& rc) override {
    glm::mat4 tf_matrix = GetTransformationMatrix();
    camera_info cam = rc.GetActiveCamera();
    // matte material doesn't accept spotlights!
    // TODO: modify material to accept different types of lights
    m.SetSpotlights(rc.GetSpotlights());
    spotlight_info i = rc.GetSpotlights()[0];
    m.SetModelTransforms(tf_matrix);
    m.SetCameraTransforms(cam.view_matrix);
    m.SetSurfaceColor(glm::vec4(1.0, 0.6, 0.0, 1.0));
    m.UseMaterial();
    Draw();
  }

  
 private:
  const float rot_inc_ = 1.0f;
  float rot_;
  MatteMaterial m;
};

class MovingCamera : public GameCamera {
 public:
  MovingCamera(Context* ctx) : GameCamera(ctx) {
    // create event which moves the camera

    motion_x = 0;
    motion_z = 0;

    rot_x = 0;
    rot_y = 0;

    // ideally: we always pass the context in instead of relying on *this*
    // note also: the creator is responsible for destroying this event handler or else shit will break
    auto event_lambda = [&, this](int key, int action, int mods) {
      int mod = 0;
      if (action == GLFW_PRESS) {
        mod = 5;
      } else if (action == GLFW_RELEASE) {
        mod = -5;
      }

      switch (key) {
        case GLFW_KEY_S:
          motion_z -= mod;
          break;
        case GLFW_KEY_W:
          motion_z += mod;
          break;
        case GLFW_KEY_A:
          motion_x += mod;
          break;
        case GLFW_KEY_D:
          motion_x -= mod;
      }

      BOOST_LOG_TRIVIAL(trace) << "button press!!!";
      BOOST_LOG_TRIVIAL(trace) << motion_x << ", " << motion_z;
    };

    auto rotation_lambda = [&, this](int key, int action, int mods) {
      int mod = 0;
      if (action == GLFW_PRESS) {
        mod = 2;
      } else if (action == GLFW_RELEASE) {
        mod = -2;
      }

      switch (key) {
        case GLFW_KEY_DOWN:
          rot_x -= mod;
          break;
        case GLFW_KEY_UP:
          rot_x += mod;
          break;
        case GLFW_KEY_LEFT:
          rot_y += mod;
          break;
        case GLFW_KEY_RIGHT:
          rot_y -= mod;
      }

      BOOST_LOG_TRIVIAL(trace) << "button press!!!";
      BOOST_LOG_TRIVIAL(trace) << rot_x << ", " << rot_y;
    };

    // need a way to register multiple events to a single id.
    w_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_W, event_lambda);
    s_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_S, event_lambda);
    a_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_A, event_lambda);
    d_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_D, event_lambda);

    u_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_UP, rotation_lambda);
    do_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_DOWN, rotation_lambda);
    l_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_LEFT, rotation_lambda);
    r_event = GetContext()->GetEventManager()->RegisterKeyListener(GLFW_KEY_RIGHT, rotation_lambda);
  }

  void Update() override {
    auto w = GetPosition();
    float delta = GetContext()->GetDeltaTime();
    auto r = GetRotation();
    glm::mat4 rotation = glm::eulerAngleYXZ(r.y, r.x, r.z);
    glm::vec4 initial(0, 0, -1, 0);
    glm::vec4 initial_x(-1, 0, 0, 0);
    initial = rotation * initial;
    initial_x = rotation * initial_x;

    BOOST_LOG_TRIVIAL(trace) << "new rot: " << r.x << ", " << r.y << ", " << r.z;
    SetPosition(w + glm::vec3(initial * (motion_z * delta)) + glm::vec3(initial_x * (motion_x * delta)));
    SetRotation(glm::vec3(r.x + rot_x * delta, r.y + rot_y * delta, r.z));
  }

  ~MovingCamera() {
    // call from destroy so we can ensure the context is live
    // though it still should be here
    auto manager = GetContext()->GetEventManager();
    manager->RemoveKeyListener(a_event);
    manager->RemoveKeyListener(s_event);
    manager->RemoveKeyListener(w_event);
    manager->RemoveKeyListener(d_event);

    manager->RemoveKeyListener(u_event);
    manager->RemoveKeyListener(do_event);
    manager->RemoveKeyListener(l_event);
    manager->RemoveKeyListener(r_event);
  }
 private:
  int a_event;
  int d_event;
  int s_event;
  int w_event;

  int u_event;
  int do_event;
  int l_event;
  int r_event;

  float motion_x;
  float motion_z;

  float rot_x;
  float rot_y;
};

class FrameText : public TextObject {
 public:
  FrameText(Context* ctx) : TextObject(ctx, "resources/montserrat-light.ttf") { a = 0; }
  void Update() override {
    a += GetContext()->GetDeltaTime();
    SetText(std::to_string(a));
    SetRotation(glm::vec3(0, a / 2.5, 0));
  }
 private:
  float a;
};

/**
 *  Simple test scene
 */ 
class TestScene : public Scene {
 public:
  TestScene(Context* ctx) {
    this->ctx = ctx;
  }
  void Initialize() override {
    // TODO: USE THE RAT! https://sketchfab.com/3d-models/rat-847629266c0f442da74fb132f46f3baf
    // just the rat model for now
    // subclass so that we can add some custom behavior
    game_object_root_ = std::make_shared<Empty>(ctx);
    auto cam = std::make_shared<MovingCamera>(ctx);
    game_object_root_->AddChild(cam);
    cam->SetPosition(glm::vec3(0, 0, -5));
    cam->SetRotation(glm::vec3(0, 1.6, 0));
    cam->SetFov(45.0f);
    cam->SetActive(true);
    auto rat = std::make_shared<RatModel>(ctx);
    rat->SetPosition(glm::vec3(0, 0, 3));
    game_object_root_->AddChild(rat);

    auto light = std::make_shared<SpotLight>(ctx);
    light->SetPosition(glm::vec3(1, 4, -2));
    light->SetDiffuseIntensity(1.0);
    game_object_root_->AddChild(light);

    auto rat_two = std::make_shared<RatModel2>(ctx);
    rat_two->SetScale(glm::vec3(0.5, 0.5, 0.5));
    rat_two->SetPosition(glm::vec3(0, 0, -1));

    auto t = std::make_shared<FrameText>(ctx);
    t->SetTextColor(glm::vec4(1.0, 0.5, 1.0, 1.0));
    t->SetTextSize(384.0f);
    t->SetPosition(glm::vec3(2, 0, 0));
    rat->AddChild(t);
    
    t->AddChild(rat_two);
  }

  std::shared_ptr<Object> GetGameObjectRoot() {
    return game_object_root_;
  }
 private:
  Context* ctx;
  std::shared_ptr<GameObject> game_object_root_;  // game object root lol
};

int main(int argc, char** argv) {
  GLFWwindow* main_win = InitializeGLFW(1280, 720, "and he never stoped playing, he always was keep beliving");
  auto ctx = std::make_shared<Context>(main_win);
  while (true) {
    auto prog = ctx->GetCachedFileLoader()->GetLoaderProgress();
    BOOST_LOG_TRIVIAL(trace) << "loading progress: " << (((float)prog.bytes_read * 100.0f) / prog.bytes_sum);
    if (prog.bytes_read == prog.bytes_sum) {
      break;
    }
    
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  auto scene = std::make_shared<TestScene>(ctx.get());
  GameLoop(scene, ctx, main_win);

  glfwDestroyWindow(main_win);
  glfwTerminate();
  return 0;
}
