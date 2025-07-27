#version 330 core
out vec4 FragColor;

struct Material {
    // Texture samplers
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;
    sampler2D texture_emission1;
    sampler2D texture_roughness1;
    sampler2D texture_metallic1;
    sampler2D texture_ao1;
    
    // Texture availability flags
    bool hasDiffuse;
    bool hasSpecular;
    bool hasNormal;
    bool hasHeight;
    bool hasEmission;
    bool hasRoughness;
    bool hasMetallic;
    bool hasAO;
    
    // Fallback material properties
   vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;
    float opacity;
    float roughness;
    float metallic;
}; 

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Tangent;
in vec3 Bitangent;
in mat3 TBN;

uniform vec3 viewPos;
uniform DirectionalLight dirLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;
uniform Material material;

// Light enable/disable uniforms
uniform bool dirLightEnabled;
uniform bool pointLightEnabled;
uniform bool spotLightEnabled;

// Function prototypes
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 specularColor, float roughness, float metallic, float ao);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specularColor, float roughness, float metallic, float ao);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specularColor, float roughness, float metallic, float ao);
vec3 getNormalFromMap();

void main()
{    
    // Sample textures
    vec3 albedo = material.hasDiffuse ? texture(material.texture_diffuse1, TexCoords).rgb : material.diffuse;
    vec3 specularColor = material.hasSpecular ? texture(material.texture_specular1, TexCoords).rgb : material.specular;
    vec3 emission = material.hasEmission ? texture(material.texture_emission1, TexCoords).rgb : material.emission;
    float roughness = material.hasRoughness ? texture(material.texture_roughness1, TexCoords).r : material.roughness;
    float metallic = material.hasMetallic ? texture(material.texture_metallic1, TexCoords).r : material.metallic;
    float ao = material.hasAO ? texture(material.texture_ao1, TexCoords).r : 1.0;
    
    // Calculate normal (either from normal map or interpolated normal)
    vec3 normal = material.hasNormal ? getNormalFromMap() : normalize(Normal);
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);
    
    // Phase 1: directional lighting
    if (dirLightEnabled) {
        result += CalcDirLight(dirLight, normal, viewDir, albedo, specularColor, roughness, metallic, ao);
    }
    
    // Phase 2: point lights
    if (pointLightEnabled) {
        result += CalcPointLight(pointLight, normal, FragPos, viewDir, albedo, specularColor, roughness, metallic, ao);
    }
    
    // Phase 3: spot light
    if (spotLightEnabled) {
        result += CalcSpotLight(spotLight, normal, FragPos, viewDir, albedo, specularColor, roughness, metallic, ao);
    }
    
    // Add emission
    result += emission;
    
    // Apply ambient occlusion
    result *= ao;
    
    // If no lights are enabled, use basic ambient lighting
    if (!dirLightEnabled && !pointLightEnabled && !spotLightEnabled) {
        result = albedo * 0.1 * ao;
    }
    
    FragColor = vec4(result, 1.0);
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(material.texture_normal1, TexCoords).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

// Calculates the color when using a directional light.
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 specularColor, float roughness, float metallic, float ao)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * (1.0 - roughness));
    
    // Combine results
    vec3 ambient = light.ambient * albedo * material.ambient;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specularColor;
    
    // Apply metallic workflow adjustments
    diffuse *= (1.0 - metallic);
    specular = mix(specular, albedo * spec, metallic);
    
    return (ambient + diffuse + specular);
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specularColor, float roughness, float metallic, float ao)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * (1.0 - roughness));
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Combine results
    vec3 ambient = light.ambient * albedo * material.ambient;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specularColor;
    
    // Apply metallic workflow adjustments
    diffuse *= (1.0 - metallic);
    specular = mix(specular, albedo * spec, metallic);
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specularColor, float roughness, float metallic, float ao)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * (1.0 - roughness));
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Combine results
    vec3 ambient = light.ambient * albedo * material.ambient;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specularColor;
    
    // Apply metallic workflow adjustments
    diffuse *= (1.0 - metallic);
    specular = mix(specular, albedo * spec, metallic);
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return (ambient + diffuse + specular);
}