#version 330

in vec3 vertex;
in vec3 normal;
in vec3 uv;

uniform mat4 mvpMatrix;

uniform float timeNow;

uniform float timeScale;
uniform float amplitude;
uniform float wobbleMinThickness;
uniform float wobbleMaxThickness;

void main() {

 float wobbleFactor = sin(timeNow * timeScale + vertex.y * amplitude);
 vec3 position = vertex * mix(wobbleMinThickness, wobbleMaxThickness, smoothstep(-1, 1, wobbleFactor));
 gl_Position = mvpMatrix * vec4(position, 1.f);
}