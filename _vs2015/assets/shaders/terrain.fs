#version 330 // for glsl version (12 is for older versions , say opengl 2.1

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoords;

out vec4 fragmentColor;

struct DirectionalLight
{
	vec3 direction;
	
	vec3 color;
	vec3 ambientColor;
};

struct PointLight
{
	vec3 position;

	vec3 color;
	vec3 ambientColor;

	float constantFalloff;
	float linearFalloff;
	float quadraticFalloff;
};

// Lights setup
uniform DirectionalLight directionalLights[4];
uniform int directionalLightsCount = 0;

uniform PointLight pointLights[4];
uniform int pointLightsCount;

uniform vec3 diffuseColor	= vec3(0.5f, 0.5f, 0.5f);
uniform vec3 specularColor	= vec3(1.0f, 1.0f, 1.0f);

uniform sampler2D splatMapTexture;
uniform sampler2D diffuseTexture1;
uniform sampler2D diffuseTexture2;
uniform sampler2D diffuseTexture3;
uniform sampler2D diffuseTexture4;

uniform sampler2D specularTexture;

uniform vec3 cameraPosition;

uniform float specularStrength = 16;
uniform float timeNow;
uniform float timeScale;

vec3 CalculateDirectionalLightContribution(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseMaterial, vec3 specularMaterial)
{
	vec3 ambient			= light.ambientColor * diffuseMaterial;
    vec3 lightDir 			= normalize(-light.direction);
	vec3 diffuse			= clamp(dot(normal, lightDir), 0, 1) * light.color * diffuseMaterial;
	vec3 reflectedDir		= reflect(-lightDir, normal);
	vec3 specular			= pow(clamp(dot(reflectedDir, viewDir), 0, 1), specularStrength) * light.color * specularMaterial;
	return ambient + diffuse + specular;
}

vec3 CalculatePointLightContribution(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseMaterial, vec3 specularMaterial)
{
	vec3 ambient			= light.ambientColor * diffuseMaterial;
    vec3 delta				= light.position - worldPosition;
	float distance			= length(delta);
    vec3 lightDir			= delta / distance;
	vec3 diffuse			= clamp(dot(normal, lightDir), 0, 1) * light.color * diffuseMaterial;
	vec3 reflectedDir		= reflect(-lightDir, normal);
	vec3 specular			= pow(clamp(dot(reflectedDir, viewDir), 0, 1), specularStrength) * light.color * specularMaterial;
	float attenuation		= 1.0f / (light.constantFalloff + light.linearFalloff * distance + light.quadraticFalloff * (distance * distance));
	
	return attenuation * (ambient + diffuse + specular);
}

vec4 animateTexture(sampler2D sampler, vec2 uv)
{
	vec2 velocity = normalize(texCoords + vec2(0.5f, 0.5f)) * cos(timeNow * 0.5f) * 0.4f;
	velocity.y *= -1;
	float smoothFactor = sin(timeNow * timeScale);
	return mix(texture(sampler, uv + velocity), texture(sampler, uv - velocity), smoothstep(-1, 1,smoothFactor));
}

vec3 CalculateDiffuseSample(vec2 uv)
{
	vec4 splatMap = texture(splatMapTexture, texCoords);

	return	vec3(
			splatMap.r * texture(diffuseTexture1, uv) +
			splatMap.g * animateTexture(diffuseTexture2, uv) +
			splatMap.b * texture(diffuseTexture3, uv) +
			splatMap.a * texture(diffuseTexture4, uv)
			);
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

void main()
{
	vec3 normal				= normalize(worldNormal);
    vec3 viewDirection	    = normalize(cameraPosition - worldPosition);
    vec3 diffuseMaterial	= diffuseColor  * CalculateDiffuseSample(texCoords);
    vec3 specularMaterial	= specularColor * vec3(texture(specularTexture, texCoords));

	vec3 color				= GetLightsContribution(normal, viewDirection, diffuseMaterial, specularMaterial);

	fragmentColor = vec4 (color, 1);
}