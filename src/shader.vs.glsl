#version 330

layout (location = 0) in vec3 att_pos;
layout (location = 1) in vec3 att_color;
layout (location = 2) in vec3 att_normal;
layout (location = 3) in vec2 att_tex_coord;

out vec2 tex_coord;
out vec3 vertex_color;

struct Transform {
	mat4 model;
	mat4 view;
	mat4 projection;
};
uniform Transform trans;

// [TODO] passing uniform variable for texture coordinate offset

void main() {
	// [TODO]
	gl_Position = trans.projection * trans.view * trans.model * vec4(att_pos, 1.0);
	tex_coord = att_tex_coord;
	vertex_color = att_color;
}
