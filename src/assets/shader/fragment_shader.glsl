#version 140

in vec2 vTex;
in float vFaceID;

out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    // get texture color
    vec4 texColor = texture(uTexture, vTex);
    // アルファテスト | 透明部分をカット
    if (texColor.a < 0.1) discard;

    // calculate lighting
    vec3 normal  = vec3(0.0, 1.0, 0.0); // all faces up
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    // 内積で拡散反射を計算 (最低光量を0.4程度にすると暗くなりすぎない)
    float diff = max(dot(normal, lightDir), 0.4);
    FragColor = vec4(texColor.rgb * diff, texColor.a);
}