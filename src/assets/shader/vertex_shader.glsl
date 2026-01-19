#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in float aFaceID;
layout (location = 3) in float aTextureLayer;

uniform mat4 uViewProj;
uniform vec3 uViewPos;
uniform vec3 uChunkPos;
uniform vec3 uSunDir;

out vec2 vTex;
out float vDist;
out float vLight;
out float vLayer;

void main() {
    vec3 worldPos = aPos + uChunkPos;
    gl_Position = uViewProj * vec4(worldPos, 1.0);
    
    vTex = aTex;
    vDist = distance(worldPos, uViewPos);
    vLayer = aTextureLayer;

    // vTex = aTex;
    vDist = distance(worldPos, uViewPos);

    vec3 normal;
    if (aFaceID == 0.0)      normal = vec3(0, 0, 1);  // FRONT
    else if (aFaceID == 1.0) normal = vec3(0, 0, -1); // BACK
    else if (aFaceID == 2.0) normal = vec3(0, 1, 0);  // TOP
    else if (aFaceID == 3.0) normal = vec3(0, -1, 0); // BOTTOM
    else if (aFaceID == 4.0) normal = vec3(1, 0, 0);  // RIGHT
    else if (aFaceID == 5.0) normal = vec3(-1, 0, 0); // LEFT
    else                     normal = vec3(0, 1, 0);

    vLight = max(dot(normal, normalize(uSunDir)), 0.5);
    // vLight = 1.0;
}