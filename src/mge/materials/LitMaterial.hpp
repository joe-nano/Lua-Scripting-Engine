#ifndef LITMATERIAL_HPP
#define LITMATERIAL_HPP

#include "GL/glew.h"
#include "mge/materials/AbstractMaterial.hpp"

class ShaderProgram;
class Texture;

/**
 * This is about the simplest material we can come up with, it demonstrates how to
 * render a single color material without caching, passing in all the matrices we require separately.
 */

static constexpr int MAX_NUM_POINT_LIGHTS = 4;
static constexpr int MAX_NUM_DIRECTIONAL_LIGHTS = 4;

struct LightUniformLocations {
    struct DirectionalLightLocation {
        GLint direction = -1;

        GLint color = -1;
        GLint ambientColor = -1;

    } directionalLights[MAX_NUM_DIRECTIONAL_LIGHTS];

    GLint directionalLightsCount = -1;

    struct PointLightLocation {
        GLint position = -1;

        GLint color = -1;
        GLint ambientColor = -1;

        GLint constantFalloff = -1;
        GLint linearFalloff = -1;
        GLint quadraticFalloff = -1;

    } pointLights[MAX_NUM_POINT_LIGHTS];

    GLint pointLightsCount = -1;
};

class LitMaterial : public AbstractMaterial
{
    public:
        LitMaterial(glm::vec3 pColor = glm::vec3(1,0,0));
        virtual ~LitMaterial();

        virtual void render(World* pWorld, Mesh* pMesh, const glm::mat4& pModelMatrix, const glm::mat4& pViewMatrix, const glm::mat4& pProjectionMatrix) override;

        //in rgb values
        void setDiffuseColor (glm::vec3 pDiffuseColor);
        void setDiffuseTexture(Texture* diffuseTexture) { _diffuseTexture = diffuseTexture; }
        void setSpecularTexture(Texture* specularTexture) { _specularTexture = specularTexture; }
        void setShininess(float shininess) { _shininess = shininess; }

        void CacheLightUniformLocations();

    private:
        //all the static properties are shared between instances of LitMaterial
        //note that they are all PRIVATE, we do not expose this static info to the outside world
        static ShaderProgram* _shader;
        static void _lazyInitializeShader();

        LightUniformLocations _lightUniformLocations;

        //this one is unique per instance of color material
        glm::vec3 _diffuseColor = glm::vec3(1, 1, 1);
        glm::vec3 _specularColor = glm::vec3(1, 1, 1);

        Texture* _diffuseTexture = nullptr;
        Texture* _specularTexture = nullptr;

        float _shininess = 16.0f;
};

#endif // LITMATERIAL_HPP
