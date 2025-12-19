#version 140

// in vec3 vNormal;
in vec2 vTex;
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

    // vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    // float diff = max(dot(normalize(vNormal), lightDir), 0.1);
    // vec3 base = vec3(0.8, 0.7, 0.5);
    // FragColor = vec4(base * diff, 1.0);
}

// #version 330 core
// out vec4 frag_color;

// // in vec3 color;
// in vec2 coordinate;

// uniform sampler2D our_texture;

// void main() {
//     frag_color = texture(our_texture, coordinate);
//     // frag_color = vec4(coordinate.x, coordinate.y, 0.0f, 1.0f);
// }