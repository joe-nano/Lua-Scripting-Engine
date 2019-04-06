#version 330

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoords;

struct DirectionalLight {

    vec3 color;
    vec3 colorAmbient;

    vec3 direction;

    float falloffConstant;
    float falloffLinear;
    float falloffQuadratic;
};

struct PointLight {

    vec3 color;
    vec3 colorAmbient;

    vec3 position;

    float falloffConstant;
    float falloffLinear;
    float falloffQuadratic;
};

struct SpotLight {

    vec3 color;
    vec3 colorAmbient;

    vec3 position;
    vec3 direction;

    float falloffConstant;
    float falloffLinear;
    float falloffQuadratic;
    float innerCutoff; // cos of inner angle
    float outerCutoff; // cos of outer angle
};

uniform DirectionalLight directionalLights[4];
uniform int numDirectionalLights = 0;

uniform PointLight pointLights[10];
uniform int numPointLights = 0;

uniform SpotLight spotLights[4];
uniform int numSpotLights = 0;

uniform vec3 viewPosition;

// Custom uniforms
uniform vec3 diffuseColor  = vec3(1,1,1);
uniform sampler2D diffuseMap;
uniform vec3 specularColor = vec3(1,1,1);
uniform sampler2D specularMap;
uniform float shininess = 1;

out vec4 fragmentColor;

vec3 CalculateDirectionalLightContribution(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 materialDiffuse, vec3 materialSpecular);
vec3 CalculatePointLightContribution(PointLight light, vec3 normal, vec3 viewDirection, vec3 materialDiffuse, vec3 materialSpecular);
vec3 CalculateSpotLightContribution(SpotLight light, vec3 normal, vec3 viewDirection, vec3 materialDiffuse, vec3 materialSpecular);

void main() {

    vec3 normal = normalize(worldNormal);
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    vec3 materialDiffuse  = diffuseColor  * vec3(texture(diffuseMap , texCoords));
    vec3 materialSpecular = specularColor * vec3(texture(specularMap, texCoords));

    vec3 color = vec3(0,0,0);

    for (int i = 0; i < numDirectionalLights; ++i) {
        color += CalculateDirectionalLightContribution(directionalLights[i], normal, viewDirection, materialDiffuse, materialSpecular);
    }

    for (int i = 0; i < numPointLights; ++i) {
        color += CalculatePointLightContribution(pointLights[i], normal, viewDirection, materialDiffuse, materialSpecular);
    }

	fragmentColor = vec4(color, 1);
}

vec3 CalculateDirectionalLightContribution(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 materialDiffuse, vec3 materialSpecular) {

    vec3 ambient = light.colorAmbient * materialDiffuse;

    vec3 diffuse = max(0, dot(normal, light.direction)) * light.color * materialDiffuse;

    vec3 reflectedDirection = reflect(-light.direction, normal);
    vec3 specular = pow(max(0, dot(reflectedDirection, viewDirection)), shininess) * light.color * materialSpecular;

    return ambient + diffuse + specular;
}

vec3 CalculatePointLightContribution(PointLight light, vec3 normal, vec3 viewDirection, vec3 materialDiffuse, vec3 materialSpecular) {

    vec3 ambient = light.colorAmbient * materialDiffuse;

    vec3 delta = light.position - worldPosition;
    float distance = length(delta);
    vec3 lightDirection = delta / distance;

    vec3 diffuse = max(0, dot(normal, lightDirection)) * light.color * materialDiffuse;

    vec3 reflectedDirection = reflect(-lightDirection, normal);
    vec3 specular = pow(max(0, dot(reflectedDirection, viewDirection)), shininess) * light.color * materialSpecular;

    float attenuation = 1.f / (light.falloffConstant + light.falloffLinear * distance + light.falloffQuadratic * distance * distance);

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;   

    return ambient + diffuse + specular;
}