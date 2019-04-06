#version 330

in vec3 vertex;
in vec3 normal;
in vec2 uv;

uniform	mat4 projectionMatrix;
uniform	mat4 viewMatrix;
uniform	mat4 modelMatrix;
uniform mat3 worldMatrix;

out vec3 worldPosition;
out vec3 worldNormal;
out vec2 texCoords;

void main(void) {

    worldPosition = vec3(matrixModel * vec4(vertex, 1));
    gl_Position = projectionMatrix * viewMatrix * vec4(worldPosition, 1);

	-- 
    worldNormal = worldMatrix * normal;
    texCoords = uv;
}
