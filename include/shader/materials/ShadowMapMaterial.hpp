#ifndef SHADOW_MAP_MATERIAL_H_
#define SHADOW_MAP_MATERIAL_H_

#include <file/CachedFileLoader.hpp>

#include <shader/Material.hpp>
#include <shader/ShaderProgram.hpp>

#include <engine/Context.hpp>

#include <glm/glm.hpp>

#include <memory>

namespace monkeysworld {
namespace shader {
namespace materials {

class ShadowMapMaterial : public ::monkeysworld::shader::Material {
 public:
  /**
   *  Creates a new ShadowMapMaterial.
   *  @param ctx - Context ptr.
   */ 
  ShadowMapMaterial(engine::Context* ctx);

  void UseMaterial() override;

  void SetCameraTransforms(const glm::mat4& vp_matrix);
  void SetModelTransforms(const glm::mat4& model_matrix);

 private:
  ShaderProgram shadow_prog_;
};

}
}
}

#endif