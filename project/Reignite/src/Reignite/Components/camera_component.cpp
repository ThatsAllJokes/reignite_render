#include "camera_component.h"

#include <gtc/quaternion.hpp>
#include <gtc/matrix_transform.hpp>

#include "../state.h"

#include "transform_component.h"
#include "render_component.h"
#include "light_component.h"


void Reignite::Camera::setInputAccess(const std::shared_ptr<State> state) {

  this->state = state;
}

void Reignite::Camera::updateViewMatrix() {

  view = glm::lookAt(position, position + direction, vec3f(0.0f, 1.0f, 0.0f));

  /*mat4f rotM = glm::mat4(1.0f);
  mat4f transM;

  rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
  rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

  glm::vec3 translation = position;
  if (flipY) {
    translation.y *= -1.0f;
  }

  transM = glm::translate(glm::mat4(1.0f), translation);
  view = rotM * transM;*/
}

void Reignite::Camera::setPerspective(float fov, float aspect, float znear, float zfar) {

  this->fov = fov;
  this->Znear = znear;
  this->Zfar = zfar;
  projection = glm::perspective(glm::radians(fov), aspect, znear, zfar);
  if (flipY) {
    projection[1, 1] *= -1.0f;
  }
}

void Reignite::Camera::update(float deltaTime) {

  if (state->input->isMouseButtonDown(1)) {
    
    float rotSpeed = deltaTime * rotationSpeed;

    vec2f mouse = state->window->mousePosition();
    mouse.x /= state->window->width();
    mouse.y /= state->window->height();

    direction = vec3f(
      cos(mouse.x * (2.0f * 3.1415f)),
      sin(mouse.y * (2.0f * 3.1415f)),
      sin(mouse.x * (2.0f * 3.1415f)));

    if (state->input->isKeyDown(87) || state->input->isKeyDown(65) ||
        state->input->isKeyDown(83) || state->input->isKeyDown(68) ||
        state->input->isKeyDown(81) || state->input->isKeyDown(69)) {

      float moveSpeed = deltaTime * movementSpeed;

      if(state->input->isKeyDown(87))
        position += direction * moveSpeed;
      else if(state->input->isKeyDown(83))
        position -= direction * moveSpeed;

      if(state->input->isKeyDown(65))
        position -= glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
      else if(state->input->isKeyDown(68))
        position += glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

      if (state->input->isKeyDown(81))
        position -= glm::normalize(glm::cross(direction, glm::vec3(1.0f, 0.0f, 0.0f))) * moveSpeed;
      else if (state->input->isKeyDown(69))
        position += glm::normalize(glm::cross(direction, glm::vec3(1.0f, 0.0f, 0.0f))) * moveSpeed;

    }

    updateViewMatrix();
  }


  /*if (state->mouseButtons.left) {

    float rotSpeed = deltaTime * rotationSpeed;

    rotation.x += state->mousePos.x * 1.0f * rotSpeed;
    rotation.y -= state->mousePos.y * 1.0f * rotSpeed;
    rotate(vec3f(state->mousePos.y * rotSpeed, -state->mousePos.x * rotSpeed, 0.0f));
  }*/
}