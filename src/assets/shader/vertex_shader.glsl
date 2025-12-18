#version 140

in vec3 aPos;
in vec3 aNormal;
in vec2 aTex;

uniform mat4 uViewProj;

out vec3 vNormal;
out vec2 vTex;

void main() {
    vec3 p = aPos;
    gl_Position = uViewProj * vec4(p, 1.0);
    vNormal = aNormal;
    vTex = aTex;
}

// #version 310
// layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec3 aNormal;
// layout(location = 2) in vec2 aTex;
// layout(location = 3) in vec3 aInstancePos;

// uniform mat4 uViewProj;

// out vec3 vNormal;
// out vec2 vTex;

// void main() {
//     // translate cube by instance position (assume instance pos is world coords)
//     vec3 p = aPos + aInstancePos;
//     gl_Position = uViewProj * vec4(p, 1.0);
//     vNormal = aNormal;
//     vTex = aTex;
// }

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