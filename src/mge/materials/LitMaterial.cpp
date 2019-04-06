#include "glm.hpp"

#include "LitMaterial.hpp"
#include "mge/core/Texture.hpp"
#include "mge/core/Light.hpp"
#include "mge/core/World.hpp"
#include "mge/core/Camera.hpp"
#include "mge/config.hpp"
#include "mge/core/GameObject.hpp"
#include "mge/core/Mesh.hpp"
#include "mge/core/ShaderProgram.hpp"

ShaderProgram* LitMaterial::_shader = NULL;

LitMaterial::LitMaterial(glm::vec3 pDiffuseColor):_diffuseColor (pDiffuseColor)
{
    static Texture* white = Texture::load(config::MGE_TEXTURE_PATH + "white.png");

    _diffuseTexture = white;
    _specularTexture = white;

    //every time we create an instance of colormaterial we check if the corresponding shader has already been loaded
    _lazyInitializeShader();
    CacheLightUniformLocations();
}

void LitMaterial::_lazyInitializeShader() {
    //this shader contains everything the material can do (it can render something in 3d using a single color)
    if (!_shader) {
        _shader = new ShaderProgram();
        _shader->addShader(GL_VERTEX_SHADER, config::MGE_SHADER_PATH+"lit.vs");
        _shader->addShader(GL_FRAGMENT_SHADER, config::MGE_SHADER_PATH+"lit.fs");
        _shader->finalize();

        _shader->use();

        //tell the shader the texture slot for the diffuse texture is slot 0
        glUniform1i(_shader->getUniformLocation("diffuseTexture"), 0);

        glUniform1i(_shader->getUniformLocation("specularTexture"), 1);
    }
}

LitMaterial::~LitMaterial()
{
    //dtor
}

void LitMaterial::setDiffuseColor(glm::vec3 pDiffuseColor) {
    _diffuseColor = pDiffuseColor;
}

void LitMaterial::render(World* pWorld, Mesh* pMesh, const glm::mat4& pModelMatrix, const glm::mat4& pViewMatrix, const glm::mat4& pProjectionMatrix) {
    _shader->use();

    //set the material color
    glUniform3fv (_shader->getUniformLocation("diffuseColor"), 1, glm::value_ptr(_diffuseColor));
    glUniform3fv(_shader->getUniformLocation("specularColor"), 1, glm::value_ptr(_specularColor));

    //pass in a precalculate mvp matrix (see texture material for the opposite)
    glUniformMatrix4fv(_shader->getUniformLocation("projectionViewMatrix"), 1, GL_FALSE, glm::value_ptr(pProjectionMatrix * pViewMatrix));
    glUniformMatrix4fv(_shader->getUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(pModelMatrix));
    glUniformMatrix3fv(_shader->getUniformLocation("worldMatrix"), 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(pModelMatrix)))));

    glUniform3fv(_shader->getUniformLocation("cameraPosition"), 1, glm::value_ptr(pWorld->getMainCamera()->getLocalPosition()));

    int directionalLightsCount  = 0;
    int pointLightsCount        = 0;

    for (GLint i = 0; i < pWorld->getLightCount(); ++i)
    {
        Light* light = pWorld->getLightAt(i);

        switch (light->lightType)
        {
            case LightType::DirectionalLight:
            {
                directionalLightsCount++;

                if (directionalLightsCount >= MAX_NUM_DIRECTIONAL_LIGHTS)
                    break;

                glUniform3fv(_lightUniformLocations.directionalLights[directionalLightsCount - 1].direction, 1, glm::value_ptr(glm::normalize(glm::vec3(light->getWorldTransform()[2]))));
                glUniform3fv(_lightUniformLocations.directionalLights[directionalLightsCount - 1].color, 1, glm::value_ptr(light->color * light->intensity));
                glUniform3fv(_lightUniformLocations.directionalLights[directionalLightsCount - 1].ambientColor, 1, glm::value_ptr(light->ambientColor * light->intensity));
                break;
            }
            case LightType::PointLight:
            {
                pointLightsCount++;

                if (pointLightsCount >= MAX_NUM_POINT_LIGHTS)
                    break;

                glUniform3fv(_lightUniformLocations.pointLights[pointLightsCount - 1].position, 1, glm::value_ptr(light->getWorldPosition()));
                glUniform3fv(_lightUniformLocations.pointLights[pointLightsCount - 1].color, 1, glm::value_ptr(light->color * light->intensity));
                glUniform3fv(_lightUniformLocations.pointLights[pointLightsCount - 1].ambientColor, 1, glm::value_ptr(light->ambientColor * light->intensity));

                glUniform1f(_lightUniformLocations.pointLights[pointLightsCount - 1].constantFalloff, light->constantFallOff);
                glUniform1f(_lightUniformLocations.pointLights[pointLightsCount - 1].linearFalloff, light->linearFallOff);
                glUniform1f(_lightUniformLocations.pointLights[pointLightsCount - 1].quadraticFalloff, light->quadraticFallOff);
                break;
            }
            default:
                break;
        }
    }

    glUniform1i(_lightUniformLocations.directionalLightsCount, directionalLightsCount);
    glUniform1i(_lightUniformLocations.pointLightsCount, pointLightsCount);

    glUniform1f(_shader->getUniformLocation("specularStrength"), _shininess);

    if (_diffuseTexture)
    {
        //setup texture slot 0
        glActiveTexture(GL_TEXTURE0);
        //bind the texture to the current active slot
        glBindTexture(GL_TEXTURE_2D, _diffuseTexture->getId());
    }

    if (_specularTexture)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _specularTexture->getId());
    }

    //now inform mesh of where to stream its data
    pMesh->streamToOpenGL(
        _shader->getAttribLocation("vertex"),
        _shader->getAttribLocation("normal"),
        _shader->getAttribLocation("uv")
    );
}

void LitMaterial::CacheLightUniformLocations()
{
    for (int i = 0; i < MAX_NUM_DIRECTIONAL_LIGHTS; ++i)
    {
        std::string prefix = "directionalLights[" + std::to_string(i) + "].";
        auto& directionalLight = _lightUniformLocations.directionalLights[i];

        directionalLight.direction       = _shader->getUniformLocation(prefix + "direction");
        directionalLight.color           = _shader->getUniformLocation(prefix + "color");
        directionalLight.ambientColor    = _shader->getUniformLocation(prefix + "ambientColor");
    }

    _lightUniformLocations.directionalLightsCount = _shader->getUniformLocation("directionalLightsCount");

    for (int i = 0; i < MAX_NUM_POINT_LIGHTS; ++i)
    {
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        auto& pointLight = _lightUniformLocations.pointLights[i];

        pointLight.position         = _shader->getUniformLocation(prefix + "position");
        pointLight.color            = _shader->getUniformLocation(prefix + "color");
        pointLight.ambientColor     = _shader->getUniformLocation(prefix + "ambientColor");

        pointLight.constantFalloff  = _shader->getUniformLocation(prefix + "constantFalloff");
        pointLight.linearFalloff    = _shader->getUniformLocation(prefix + "linearFalloff");
        pointLight.quadraticFalloff = _shader->getUniformLocation(prefix + "quadraticFalloff");
    }

    _lightUniformLocations.pointLightsCount = _shader->getUniformLocation("pointLightsCount");
}