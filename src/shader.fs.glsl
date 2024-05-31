#version 330

in vec2 tex_coord;

out vec4 fragColor;

// [TODO] passing texture from main.cpp
// Hint: sampler2D

void main() {
	fragColor = vec4(tex_coord.xy, 0, 1);

	// [TODO] sampleing from texture
	// Hint: texture
}
