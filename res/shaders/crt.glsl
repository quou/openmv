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

/* CRT distort from: https://gist.github.com/reidrac/6754834 */

#define distortion 0.2

in VS_OUT {
	vec2 uv;
} fs_in;

out vec4 out_color;

uniform sampler2D in_texture;
uniform vec2 screen_size;

vec2 distort(vec2 coord) {
	vec2 cc = coord - vec2(0.5);
	float dist = dot(cc, cc) * distortion;
	return coord + cc * (1.0 - dist) * dist;
}

void main() {
	vec2 uv = distort(fs_in.uv);

	if (uv.x > 1.0f) { discard; }
	if (uv.x < 0.0f) { discard; }
	if (uv.y > 1.0f) { discard; }
	if (uv.y < 0.0f) { discard; }

	vec2 ps = uv * screen_size;

	vec4 color = texture2D(in_texture, uv);

	out_color = vec4(color.r * 0.7, color.g * 0.7, color.b * 0.7, 1);
	int pp = int(ps.x) % 3;
	if (pp == 1) { out_color.r = color.r; }
	else if (pp == 2) { out_color.g = color.g; }
	else { out_color.b = color.b; }
}

#end FRAGMENT
