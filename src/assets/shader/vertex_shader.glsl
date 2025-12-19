#version 140

in vec3 aPos;
// in vec3 aNormal;
in vec2 aTex;

// uniform vec3 uOffsets[500];
uniform mat4 uViewProj;
uniform int uTexIndex; // インスタンスごとに割り当てられるテクスチャの番号

// out vec3 vNormal;
out vec2 vTex;

void main() {
    vec3 p = aPos;
    gl_Position = uViewProj * vec4(p, 1.0);
    
    // atlas calculation
    float tileSize= 1.0 / 16.0;
    
    // 列と行
    float col = float(uTexIndex % 16);
    float row = floor(float(uTexIndex) / 16.0);
    // float col = 0.0;
    // float row = 0.0;

    // UV座標: 1.0 基準から 1/分割数 基準にスケールダウンし、オフセットを取得
    vTex = (aTex * tileSize) + (vec2(col * tileSize, row * tileSize));
}

// #version 330 core
// layout(location = 0) in vec3 vertex_position;
// layout(location = 1) in vec3 vertex_color;
// layout(location = 2) in vec2 vertex_coordinate;

// out vec3 color;
// out vec2 coordinate;

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// void main() {
//     gl_Position = projection * view * model * vec4(vertex_position, 1.0);
//     color = vertex_color;
//     coordinate = vertex_coordinate;
// }