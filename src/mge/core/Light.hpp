#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "glm.hpp"
#include "mge/core/GameObject.hpp"

/**
 * Exercise for the student: implement the Light class...
 * Imagine things like setLightType, setLightIntensity, setFalloffAngle etc, see the corresponding lectures
 */

enum LightType
{
    DirectionalLight,
    PointLight,
};

class Light : public GameObject
{
	public:
		Light(const std::string& aName = nullptr, const glm::vec3& aPosition = glm::vec3( 2.0f, 10.0f, 5.0f ) );
		virtual ~Light();

        //override set parent to register/deregister light...
        virtual void _setWorldRecursively (World* pWorld) override;

        LightType lightType     = LightType::DirectionalLight;
        glm::vec3 color         = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 ambientColor  = glm::vec3(0.5f, 0.5f, 0.5f);
        float intensity         = 1.0f;

        // Point Light
        float radius            = 7.0f;
        float constantFallOff   = 1.0f;
        float linearFallOff     = 0.35f;
        float quadraticFallOff  = 0.44f;
};

#endif // LIGHT_HPP
