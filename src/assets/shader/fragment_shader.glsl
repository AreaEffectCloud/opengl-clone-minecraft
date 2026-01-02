#version 140

in vec2 vTex;
in float vDist;

uniform sampler2D uTexture;
uniform vec3 uFogColor;     // fog color
uniform float uFogNear;     // fog start distance
uniform float uFogFar;      // fog end distance

out vec4 fragColor;

void main() {
    vec4 texColor = texture(uTexture, vTex); // get texture color
    if (texColor.a < 0.1) discard; // discard transparent pixels

    // 霧の濃さ(係数)を計算
    // 0.0 (霧なし)から 1.0 (完全に霧)の範囲に収める
    float fogFactor = (uFogFar - vDist) / (uFogFar - uFogNear);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    fragColor = mix(vec4(uFogColor, 1.0), texColor, fogFactor);
}