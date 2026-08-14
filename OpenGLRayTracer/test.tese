#version 460 core

layout (quads, fractional_odd_spacing, ccw) in;

struct Terrain
{
    float frequency;
    float amplitude;
    float gain;
    float lacunarity;
    int octaves;
    int max;
    int min;
};

uniform mat4 model;           // the model matrix
uniform mat4 view;            // the view matrix
uniform mat4 projection;      // the projection matrix
uniform Terrain terrain;    // terrain 

// received from Tessellation Control Shader - all texture coordinates for the patch vertices
in vec2 TextureCoord[];
out vec2 TexCoord;
in vec3 aNormal[];
out vec3 outNormal;
out vec3 FragPos;

// send to Fragment Shader for coloring
out float Height;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
    vec2(12.9898,78.233)))*
    43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
    (c - a)* u.y * (1.0 - u.x) +
    (d - b) * u.x * u.y;
}

float map(float value, float istart, float istop, float ostart, float ostop) {
    return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

float getFBM(vec3 pos) {
    float frequency = terrain.frequency;
    float amplitude = terrain.amplitude;
    float fbmValue = 0.0f;
    float normalization = 0.0f;

    for (int i = 0; i < terrain.octaves; ++i){
        fbmValue += (amplitude * noise(vec2(pos.x * frequency, pos.z * frequency)));
        normalization += amplitude;
        frequency *= terrain.lacunarity;
        amplitude *= terrain.gain;
    }
    fbmValue /= normalization;
    return map(fbmValue, 0.0, 1.0, -float(terrain.min), float(terrain.max));
}
// normals need to be figured out later, look into it deeply do not gloss over it it is an interesting thing i havent figured out
void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 p0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, u);
    vec4 p1 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, u);
    vec4 p  = mix(p0, p1, v);

    vec2 t0 = mix(TextureCoord[0], TextureCoord[1], u);
    vec2 t1 = mix(TextureCoord[2], TextureCoord[3], u);
    TexCoord = mix(t0, t1, v);

    vec3 n0 = mix(aNormal[0], aNormal[1], u);
    vec3 n1 = mix(aNormal[2], aNormal[3], u);
    outNormal = normalize(mix(n0, n1, v));

    // Normals
    vec3 position = p.xyz;
    float delta = 0.01;
    vec3 normalCalculated;

    FragPos = vec3(model * vec4(position, 1.0)); // could be wrong position != aPos could be true
    Height = getFBM(position);
    p.y += Height;
    
    float hL = getFBM(position - vec3(delta, 0.0, 0.0));
    float hR = getFBM(position + vec3(delta, 0.0, 0.0));
    float hD = getFBM(position - vec3(0.0, 0.0, delta));
    float hU = getFBM(position + vec3(0.0, 0.0, delta));

    normalCalculated = normalize(vec3(hL - hR, 2.0 * delta, hD - hU));
    outNormal = normalize(view * model * vec4(normalCalculated, 0.0)).xyz;
    gl_Position = projection * view * model * p;
}