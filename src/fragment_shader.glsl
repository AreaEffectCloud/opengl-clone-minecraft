#version 330

in vec3 color;
ou vec4 frag_color;

void main() {
    frag_color = vec4(color, 1.0);
}