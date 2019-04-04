#version 330

in vec3 vertex;
in vec3 normal;
in vec2 uv;

uniform	mat4 matrixProjection;
uniform	mat4 matrixView;
uniform	mat4 matrixModel;

out vec3 worldPosition;
out vec3 worldNormal;
out vec2 texCoords;

void main(void) {

    worldPosition = vec3(matrixModel * vec4(vertex, 1));
    gl_Position = matrixProjection * matrixView * vec4(worldPosition, 1);

    worldNormal = mat3(transpose(inverse(matrixModel))) * normal;
    texCoords = uv;
}
