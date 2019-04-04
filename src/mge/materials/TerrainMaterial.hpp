#ifndef TERRAINMATERIAL_HPP
#define TERRAINMATERIAL_HPP

#include <vector>
#include "GL/glew.h"
#include "mge/materials/LitMaterial.hpp"
#include "mge/materials/AbstractMaterial.hpp"

class ShaderProgram;
class Texture;

class TerrainMaterial : public AbstractMaterial
{
    public:
        TerrainMaterial(glm::vec3 pColor = glm::vec3(1, 1, 1));
        virtual ~TerrainMaterial();

        virtual void render(World* pWorld, Mesh* pMesh, const glm::mat4& pModelMatrix, const glm::mat4& pViewMatrix, const glm::mat4& pProjectionMatrix) override;

        //in rgb values
        void setDiffuseColor (glm::vec3 pDiffuseColor);
        void setDiffuseTexture(Texture* diffuseTexture) { _diffuseTextures.push_back(diffuseTexture); }
        void setSpecularTexture(Texture* specularTexture) { _specularTexture = specularTexture; }
        void setHeightmapTexture(Texture* heightMapTexture) { _heightmapTexture = heightMapTexture; }
        void setSplatMapTexture(Texture* splatMapTexture) { _splatMapTexture = splatMapTexture; }
        void setMaxHeight(float height) { _maxHeight = height; }
        void setShininess(float shininess) { _shininess = shininess; }

        void CacheLightUniformLocations();

    private:
        //all the static properties are shared between instances of TerrainMaterial
        //note that they are all PRIVATE, we do not expose this static info to the outside world
        static ShaderProgram* _shader;
        static void _lazyInitializeShader();

        LightUniformLocations _lightUniformLocations;

        //this one is unique per instance of color material
        glm::vec3 _diffuseColor = glm::vec3(1);
        glm::vec3 _specularColor = glm::vec3(1, 1, 1);

        std::vector<Texture*> _diffuseTextures;
        Texture* _specularTexture = nullptr;
        Texture* _heightmapTexture = nullptr;
        Texture* _splatMapTexture = nullptr;

        float _maxHeight = 1.0f;
        float _shininess = 16.0f;
};

#endif // TERRAINMATERIAL_HPP
