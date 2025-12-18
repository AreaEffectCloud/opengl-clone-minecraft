#version 140

in vec3 vNormal;
in vec2 vTex;
out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(vNormal), lightDir), 0.1);
    vec3 base = vec3(0.8, 0.7, 0.5);
    FragColor = vec4(base * diff, 1.0);
}

// #version 310
// in vec3 vNormal;
// in vec2 vTex;
// out vec4 FragColor;
// void main() {
//     // simple lambert-like shading with constant light dir
//     vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
//     float diff = max(dot(normalize(vNormal), lightDir), 0.1);
//     vec3 base = vec3(0.8, 0.7, 0.5); // color (can be replaced per-block by texture)
//     FragColor = vec4(base * diff, 1.0);
// }

// #version 330 core
// out vec4 frag_color;

// // in vec3 color;
// in vec2 coordinate;

// uniform sampler2D our_texture;

// void main() {
//     frag_color = texture(our_texture, coordinate);
//     // frag_color = vec4(coordinate.x, coordinate.y, 0.0f, 1.0f);
// }