#version 140

in vec3 aPos;
in vec2 aTex;
in float aFaceID;
in float aBlockID;

uniform mat4 uViewProj;
uniform vec3 uViewPos;

out vec2 vTex;
out float vDist;

void main() {
    gl_Position = uViewProj * vec4(aPos, 1.0);

    vDist = distance(aPos, uViewPos);

    int type = int(aBlockID);
    int texIndex = 0;
    if (type == 1) { // dirt block
        texIndex = 2;
    } else if (type == 2) { // grass block
        if (aFaceID < 0.5)      texIndex = 1; // other
        else if (aFaceID < 1.5) texIndex = 0; // top
        else                    texIndex = 2; // bottom
    } else if (type == 3) { // stone block
        texIndex = 3;
    } else if (type == 4) { // log block
        if (aFaceID == 1.0 || aFaceID == 2.0) texIndex = 5;
        else texIndex = 4;
    } else {
        texIndex = 3;
    }

    // atlas calculation
    float size = 1.0 / 16.0;
    float col = float(texIndex % 16);
    float row = floor(float(texIndex) / 16.0);

    vec2 correctedTex = vec2(aTex.x, 1.0 - aTex.y);
    vTex = (correctedTex * size) + vec2(col * size, row * size);
}