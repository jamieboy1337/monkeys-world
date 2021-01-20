#ifndef SCENE_H_
#define SCENE_H_

#include <critter/Object.hpp>
#include <critter/GameObject.hpp>
#include <critter/ui/UIObject.hpp>

namespace monkeysworld {
namespace engine {

// TODO: create includes which forward decl all members of a folder
class Context;

/**
 *  A scene represents a series of deployed assets.
 *  For the time being, scenes expose root-level assets,
 *  allowing the engine to pass functions to these assets recursively.
 * 
 *  Implementors should pass in context on ctor.
 *  Note too that the context should never change, so this should be ok. We just need our
 *  new objects to have a reference to it.
 */ 
class Scene {
 public:

  /**
   *  Used by the scene to set up all objects.
   */ 
  virtual void Initialize(Context* ctx) = 0;

  /**
   *  Our engine only really needs to know that it's an object,
   *  and our components will know better. So this should be OK.
   */ 
  virtual std::shared_ptr<critter::GameObject> GetGameObjectRoot() = 0;
  virtual std::shared_ptr<critter::ui::UIObject> GetUIObjectRoot() = 0;
};

}
}

#endif