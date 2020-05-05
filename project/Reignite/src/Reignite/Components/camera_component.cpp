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

  mat4f rotM = glm::mat4(1.0f);
  mat4f transM;

  rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
  rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

  glm::vec3 translation = position;
  if (flipY) {
    translation.y *= -1.0f;
  }

  transM = glm::translate(glm::mat4(1.0f), translation);
  view = rotM * transM;

  updated = true;
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

  updated = false;
  if (state->input->isKeyDown(87) || state->input->isKeyDown(65) ||
      state->input->isKeyDown(83) || state->input->isKeyDown(68) ||
      state->input->isKeyDown(81) || state->input->isKeyDown(69)) {

    glm::vec3 camFront;
    camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
    camFront.y = sin(glm::radians(rotation.x));
    camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
    camFront = glm::normalize(camFront);

    float moveSpeed = deltaTime * movementSpeed;

    if(state->input->isKeyDown(87))
      position += camFront * moveSpeed;
    else if(state->input->isKeyDown(83))
      position -= camFront * moveSpeed;

    if(state->input->isKeyDown(65))
      position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
    else if(state->input->isKeyDown(68))
      position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

    if (state->input->isKeyDown(81))
      position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 0.0f, 1.0f))) * moveSpeed;
    else if (state->input->isKeyDown(69))
      position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 0.0f, 1.0f))) * moveSpeed;

    updateViewMatrix();
  }

  if (state->input->isMouseButtonDown(0) || state->input->isMouseButtonDown(1)) {

    float rotSpeed = deltaTime * rotationSpeed;

    if (state->input->isMouseButtonDown(0))
      rotation -= (glm::radians(45.0f) * glm::vec3(0.0f, 1.0f, 0.0f)) * rotSpeed;
    else if (state->input->isMouseButtonDown(1))
      rotation += (glm::radians(45.0f) * glm::vec3(0.0f, 1.0f, 0.0f)) * rotSpeed;

    updateViewMatrix();
  }


  /*if (state->mouseButtons.left) {

    float rotSpeed = deltaTime * rotationSpeed;

    rotation.x += state->mousePos.x * 1.0f * rotSpeed;
    rotation.y -= state->mousePos.y * 1.0f * rotSpeed;
    rotate(vec3f(state->mousePos.y * rotSpeed, -state->mousePos.x * rotSpeed, 0.0f));
  }*/
}