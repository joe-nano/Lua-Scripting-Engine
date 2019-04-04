#pragma once
#include "glm.hpp"
#include "mge/behaviours/AbstractBehaviour.hpp"
#include <SFML/Graphics.hpp>

class GameObject;
class Camera;

class CameraController : public AbstractBehaviour
{
    public:
        CameraController(GameObject* target, float distanceToTarget = 10.0f, float rotationSpeed = 2.0f, float minTilt = -45.0f, float maxTilt = 45.0f) :
            _target(target), _distanceToTarget(distanceToTarget), _rotationSpeed(rotationSpeed), _minTilt(minTilt), _maxTilt(maxTilt)
        {
            _previousMousePosition = sf::Mouse::getPosition();
        }

        ~CameraController() override { AbstractBehaviour::~AbstractBehaviour(); }

        virtual void update(float deltaTime) override;
        void setTarget(GameObject* target) { _target = target; }

        sf::Vector2i GetMouseInput();
    private:
        CameraController() {}

        GameObject* _target = nullptr;

        /* Mouse Movement*/
        sf::Vector2i _previousMousePosition;

        /* Camera Settings */
        float _minTilt;
        float _maxTilt;

        float _distanceToTarget;

        /* Camera Movement */
        float _rotationSpeed = 2.0f;
};