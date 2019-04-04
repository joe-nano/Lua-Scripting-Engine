#include "mge/behaviours/CameraController.h"
#include "mge/core/GameObject.hpp"
#include "mge/core/Camera.hpp"
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

void CameraController::update(float deltaTime)
{
    if (!_target)
        return;

    sf::Vector2i mouseInput = GetMouseInput();

    glm::vec3 worldUp = glm::vec3(0, 1, 0);
    glm::vec3 targetPosition    = _target->getWorldPosition();
    // Word direction is improperly used here maybe inverseDirection would make more sense
    glm::vec3 direction         = glm::normalize(_owner->getLocalPosition() - targetPosition);
    direction                   = glm::rotate(direction, -mouseInput.x *  _rotationSpeed * deltaTime, worldUp);

    if (glm::any(glm::isnan(direction)))
        direction = glm::vec3(0, 0, -1.0f);

    glm::vec3 directionXZ = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));

    if (glm::any(glm::isnan(directionXZ)))
        directionXZ = glm::vec3(0, 0, 1.0f);

    glm::vec3 right = glm::cross(worldUp, directionXZ);

    float pitchAngle = glm::orientedAngle(direction, directionXZ, right);
    pitchAngle      += mouseInput.y * _rotationSpeed * deltaTime;
    pitchAngle      = glm::clamp(pitchAngle, glm::radians(_minTilt), glm::radians(_maxTilt));
    direction       = glm::rotate(directionXZ, -pitchAngle, right);

    glm::vec3 forward   = direction;
    glm::vec3 up        = glm::cross(forward, right);

    _owner->setTransform(glm::mat4(
        glm::vec4(right, 0),
        glm::vec4(up, 0),
        glm::vec4(forward, 0),
        glm::vec4(targetPosition + forward * _distanceToTarget, 1)));
}

sf::Vector2i CameraController::GetMouseInput()
{
    sf::Vector2i mousePosition  = sf::Mouse::getPosition();
    sf::Vector2i delta          = mousePosition - _previousMousePosition;

    _previousMousePosition = mousePosition;

    return delta;
}