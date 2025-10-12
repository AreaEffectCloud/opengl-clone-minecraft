#version 330 core
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec2 vertex_coordinate;

out vec3 color;
out vec2 coordinate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(vertex_position, 1.0);
    color = vertex_color;
    coordinate = vertex_coordinate;
}
