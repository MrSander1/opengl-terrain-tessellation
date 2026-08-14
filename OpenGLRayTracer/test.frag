#version 460 core
out vec4 FragColor;

in float Height;
in vec2 TexCoord;
in vec3 outNormal;
in vec3 FragPos;

struct Material {
    sampler2D diffuseGrass;
    sampler2D diffuseMud;
    vec3 specular;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
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

struct Fog {
    float maxDist;
    float minDist;
    vec3 color;
};

#define NR_POINT_LIGHTS 4  

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;
uniform Fog fog;

uniform vec3 viewPos;
uniform vec2 uvScale = vec2(256.0);

uniform bool fogToogle;
uniform bool grassToogle;

uniform float peakSharpness = 2.0;



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  

vec3 CalcFog(Fog fog, vec3 fragPos, vec3 result);
vec3 CalcGrass(float peakSharpness, vec2 uvScale, vec3 normal);

vec3 upVector = vec3(0.0, 0.0, 1.0);

void main()
{
    // properties
    vec3 norm = outNormal;
    vec3 viewDir = normalize(viewPos - FragPos);

    // directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);  
        
    // fog
    vec3 fogResult = CalcFog(fog, FragPos.xyz, result);

    if(fogToogle) {
        FragColor = vec4(fogResult, 1.0);
    } else {
        FragColor = vec4(result, 1.0);
    }

}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Grass and Mud blending
    vec3 matDiffuse = CalcGrass(peakSharpness, uvScale, normal);

    // combine results
    vec3 ambient  = light.ambient  * matDiffuse;
    vec3 diffuse  = light.diffuse  * diff * matDiffuse;
    vec3 specular = light.specular * (spec * material.specular); 
    return (ambient + diffuse + specular) * light.intensity;
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec2 tiledUV = TexCoord * 256.0; 
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // grass and mud blending
    vec3 matDiffuse = CalcGrass(peakSharpness, uvScale, normal);
    // combine results
    vec3 ambient  = light.ambient  * matDiffuse;
    vec3 diffuse  = light.diffuse  * diff * matDiffuse;
    vec3 specular = light.specular * (spec * material.specular); 
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

vec3 CalcFog(Fog fog, vec3 fragPos, vec3 result)
{
// imma be real this aint mine linear fog approach 
    float dist = length(fragPos.xyz);
    float fogFactor = (fog.maxDist - dist) / (fog.maxDist - fog.minDist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 fogResult = mix(fog.color, result, fogFactor);

    return fogResult;

}

vec3 CalcGrass(float peakSharpness, vec2 uvScale, vec3 normal) 
{
    // calculates the slope between the normals
    vec3 dNdx = dFdx(normal);
    vec3 dNdy = dFdy(normal);

    // represents the slope
    float curvature = length(dNdx) + length(dNdy);

    // textures
    vec4 grassColor = texture(material.diffuseGrass, TexCoord * uvScale);
    vec4 mudColor = texture(material.diffuseMud, TexCoord * uvScale);
    
    // compares how flat
    float flatness = dot(normal, upVector);
    
    // sets what is a peak i.e how curvy 
    float peakFactor = smoothstep(0.0, peakSharpness, curvature * flatness);

    // mixed textures
    return mix(mudColor, grassColor, peakFactor).rgb; 

}
