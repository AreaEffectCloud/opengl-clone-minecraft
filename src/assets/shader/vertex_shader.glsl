#version 140

in vec3 aPos;
in vec2 aTex;
in float aFaceID;

uniform mat4 uViewProj;
uniform vec3 uModelPos;
uniform int uBlockType;

out vec2 vTex;

void main() {
    vec3 p = aPos + uModelPos;
    gl_Position = uViewProj * vec4(p, 1.0);

    int texIndex = 0;
    if (uBlockType == 3) { // grass block
        if (aFaceID < 0.5)      texIndex = 1; // top
        else if (aFaceID < 1.5) texIndex = 0; // bottom
        else                    texIndex = 2; // other side
    } else if (uBlockType == 4) {
        if (aFaceID == 1.0 || aFaceID == 2.0) texIndex = 10;
        else texIndex = 11;
    } else {
        texIndex = uBlockType;
    }

    // atlas calculation
    float size = 1.0 / 16.0;
    float col = float(texIndex % 16);
    float row = floor(float(texIndex) / 16.0);

    vec2 correctedTex = vec2(aTex.x, 1.0 - aTex.y);
    vTex = (correctedTex * size) + vec2(col * size, row * size);
}