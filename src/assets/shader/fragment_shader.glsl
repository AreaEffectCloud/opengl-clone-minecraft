#version 330 core

in vec2 vTex;
in float vDist;
in float vLight;
in float vLayer;

out vec4 fragColor;

uniform sampler2DArray uTextureArray;
uniform vec3 uFogColor;     // fog color
uniform float uFogNear;     // fog start distance
uniform float uFogFar;      // fog end distance

void main() {
    vec4 texColor = texture(uTextureArray, vec3(vTex, round(vLayer))); // get texture color

    float alpha = texColor.a;
    if (round(vLayer) == 7.0) {
        alpha = 0.6;
    }
    if (alpha < 0.1) discard; // discard transparent
    
    vec3 color = texColor.rgb * vLight;

    // 霧の濃さ(係数)を計算
    // 0.0 (霧なし)から 1.0 (完全に霧)の範囲に収める
    float fogFactor = clamp((uFogFar - vDist) / (uFogFar - uFogNear), 0.0, 1.0);
    vec3 finalColor = mix(uFogColor, color, fogFactor);

    fragColor = vec4(finalColor, alpha);
}