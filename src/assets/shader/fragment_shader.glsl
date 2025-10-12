#version 330 core
out vec4 frag_color;

// in vec3 color;
in vec2 coordinate;

uniform sampler2D our_texture;

void main() {
    frag_color = texture(our_texture, coordinate);
    // frag_color = vec4(coordinate.x, coordinate.y, 0.0f, 1.0f);
}
