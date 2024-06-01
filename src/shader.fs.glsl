#version 330

in vec2 tex_coord;
in vec3 vertex_color;

out vec4 color;

// [TODO] passing texture from main.cpp
// Hint: sampler2D
uniform sampler2D model_texture;

void main() {
	// [TODO] sampling from texture
	// Hint: texture
	color = texture(model_texture, tex_coord) * vec4(vertex_color, 1.0);
}
