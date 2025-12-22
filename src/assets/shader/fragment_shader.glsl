#version 140

in vec2 vTex;
in float vDist;

uniform sampler2D uTexture;
uniform vec3 uFogColor;     // fog color
uniform float uFogNear;     // fog start distance
uniform float uFogFar;      // fog end distance

out vec4 fragColor;

void main() {
    // get texture color
    vec4 texColor = texture(uTexture, vTex);
    // アルファテスト | 透明部分をカット
    if (texColor.a < 0.1) discard;

    // 霧の濃さ(係数)を計算
    // 0.0 (霧なし)から 1.0 (完全に霧)の範囲に収める
    float fogFactor = (uFogFar - vDist) / (uFogFar - uFogNear);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    fragColor = mix(vec4(uFogColor, 1.0), texColor, fogFactor);

    // // calculate lighting
    // vec3 normal  = vec3(0.0, 1.0, 0.0); // all faces up
    // vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    // // 内積で拡散反射を計算 (最低光量を0.4程度にすると暗くなりすぎない)
    // float diff = max(dot(normal, lightDir), 0.4);
    // fragColor = vec4(texColor.rgb * diff, texColor.a);
}