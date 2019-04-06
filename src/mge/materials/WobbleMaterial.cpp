#include "glm.hpp"

#include "WobbleMaterial.hpp"
#include "mge/config.hpp"
#include "mge/core/GameObject.hpp"
#include "mge/core/Mesh.hpp"
#include "mge/core/ShaderProgram.hpp"

ShaderProgram* WobbleMaterial::_shader = NULL;

WobbleMaterial::WobbleMaterial(glm::vec3 pDiffuseColor):_diffuseColor (pDiffuseColor)
{
    //every time we create an instance of colormaterial we check if the corresponding shader has already been loaded
    _lazyInitializeShader();
}

void WobbleMaterial::_lazyInitializeShader() {
    //this shader contains everything the material can do (it can render something in 3d using a single color)
    if (!_shader) {
        _shader = new ShaderProgram();
        _shader->addShader(GL_VERTEX_SHADER, config::MGE_SHADER_PATH+"wobble.vs");
        _shader->addShader(GL_FRAGMENT_SHADER, config::MGE_SHADER_PATH+"color.fs");
        _shader->finalize();
    }
}

WobbleMaterial::~WobbleMaterial()
{
    //dtor
}

void WobbleMaterial::setDiffuseColor(glm::vec3 pDiffuseColor) {
    _diffuseColor = pDiffuseColor;
}

void WobbleMaterial::render(World* pWorld, Mesh* pMesh, const glm::mat4& pModelMatrix, const glm::mat4& pViewMatrix, const glm::mat4& pProjectionMatrix) {
    _shader->use();

    //set the material color
    glUniform3fv (_shader->getUniformLocation("diffuseColor"), 1, glm::value_ptr(_diffuseColor));

    //pass in all MVP matrices separately
    glUniformMatrix4fv ( _shader->getUniformLocation("projectionMatrix"),   1, GL_FALSE, glm::value_ptr(pProjectionMatrix));
    glUniformMatrix4fv ( _shader->getUniformLocation("viewMatrix"),         1, GL_FALSE, glm::value_ptr(pViewMatrix));
    glUniformMatrix4fv ( _shader->getUniformLocation("modelMatrix"),        1, GL_FALSE, glm::value_ptr(pModelMatrix));
    glUniformMatrix4fv ( _shader->getUniformLocation("mvpMatrix"),        1, GL_FALSE, glm::value_ptr(pProjectionMatrix * pViewMatrix * pModelMatrix));

    glUniform1f(_shader->getUniformLocation("timeNow"), clock() / 1000.0f);
    glUniform1f(_shader->getUniformLocation("timeScale"), 5);
    glUniform1f(_shader->getUniformLocation("amplitude"), 6);
    glUniform1f(_shader->getUniformLocation("wobbleMinThickness"), 0.6f);
    glUniform1f(_shader->getUniformLocation("wobbleMaxThickness"), 1.0f);

    //now inform mesh of where to stream its data
    pMesh->streamToOpenGL(
        _shader->getAttribLocation("vertex"),
        _shader->getAttribLocation("normal"),
        _shader->getAttribLocation("uv")
    );

}
