#begin VERTEX

#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out VS_OUT {
	vec2 uv;
} vs_out;


void main() {
	vs_out.uv = uv;

	gl_Position = vec4(position, 0.0, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 330 core

in VS_OUT {
	vec2 uv;
} fs_in;

out vec4 out_color;

uniform sampler2D in_texture;
uniform vec2 screen_size;

void main() {
	vec4 tex_col = texture(in_texture, fs_in.uv);
	out_color = vec4(1.0 - tex_col.rgb, tex_col.a);
}

#end FRAGMENT
