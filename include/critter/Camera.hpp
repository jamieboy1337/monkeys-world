#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>

namespace monkeysworld {
namespace critter {

/**
 *  Shader representation of camera.
 */ 
struct camera_info {
  glm::vec3 position;       // position of the camera
  glm::mat4 view_matrix;    // view matrix
};

/**
 *  Representation of a camera in space.
 */ 
class Camera {
 public:
  virtual glm::mat4 GetViewMatrix() const = 0;

  virtual void SetFov(float deg) = 0;

  /**
   *  Marks this camera as active.
   */ 
  virtual void SetActive(bool isActive) = 0;

  /**
   *  Returns a new camera info object, associated with this camera.
   */ 
  virtual camera_info GetCameraInfo() const = 0;

};

}
}

#endif