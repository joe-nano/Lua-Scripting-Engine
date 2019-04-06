#version 330

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoords;

struct DirectionalLight {

    vec3 color;
    vec3 ambientColor;

    vec3 direction;
};

struct PointLight {

    vec3 color;
    vec3 ambientColor;

    vec3 position;

    float constantFalloff;
    float linearFalloff;
    float quadraticFalloff;
};

uniform DirectionalLight directionalLights[4];
uniform int directionalLightsCount = 0;

uniform PointLight pointLights[4];
uniform int pointLightsCount = 0;

uniform vec3 cameraPosition;

// Custom uniforms
uniform vec3 diffuseColor  = vec3(1,1,1);
uniform sampler2D diffuseMap;
uniform vec3 specularColor = vec3(1,1,1);
uniform sampler2D specularMap;
uniform float specularStrength = 1;

out vec4 fragmentColor;

vec3 CalculateDirectionalLightContribution(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 diffuseMaterial, vec3 specularMaterial) {

    vec3 ambient = light.ambientColor * diffuseMaterial;
    vec3 diffuse = max(0, dot(normal, light.direction)) * light.color * diffuseMaterial;

    vec3 reflectedDirection = reflect(-light.direction, normal);
    vec3 specular 			= pow(max(0, dot(reflectedDirection, viewDirection)), specularStrength) * light.color * specularMaterial;

    return ambient + diffuse + specular;
}

vec3 CalculatePointLightContribution(PointLight light, vec3 normal, vec3 viewDirection, vec3 diffuseMaterial, vec3 specularMaterial) {

    vec3 ambient = light.ambientColor * diffuseMaterial;

    vec3 delta 			= light.position - worldPosition;
    float distance 		= length(delta);
    vec3 lightDirection = delta / distance;

    vec3 diffuse = max(0, dot(normal, lightDirection)) * light.color * diffuseMaterial;

    vec3 reflectedDirection = reflect(-lightDirection, normal);
    vec3 specular 			= pow(max(0, dot(reflectedDirection, viewDirection)), specularStrength) * light.color * specularMaterial;

    float attenuation = 1.f / (light.constantFalloff + light.linearFalloff * distance + light.quadraticFalloff * distance * distance);

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;   

    return ambient + diffuse + specular;
}

vec3 GetLightsContribution(vec3 normal, vec3 viewDirection, vec3 diffuseMaterial, vec3 specularMaterial) {

    vec3 color = vec3(0,0,0);

    for (int index = 0; index < directionalLightsCount; ++index) {
        color += CalculateDirectionalLightContribution(directionalLights[index], normal, viewDirection, diffuseMaterial, specularMaterial);
    }

    for (int index = 0; index < pointLightsCount; ++index) {
        color += CalculatePointLightContribution(pointLights[index], normal, viewDirection, diffuseMaterial, specularMaterial);
    }

	return color;
}

void main() {

    vec3 normal 			= normalize(worldNormal);
    vec3 viewDirection 		= normalize(cameraPosition - worldPosition);
    vec3 diffuseMaterial  	= diffuseColor  * vec3(texture(diffuseMap , texCoords));
    vec3 specularMaterial 	= specularColor * vec3(texture(specularMap, texCoords));

    vec3 color = GetLightsContribution(normal, viewDirection, diffuseMaterial, specularMaterial);

	fragmentColor = vec4(color, 1);
}