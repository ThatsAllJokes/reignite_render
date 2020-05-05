#version 450

layout (location = 0) out vec2 outUV;

vec2 uv[6] = vec2[](
  vec2(0.0, 0.0),
  vec2(1.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 1.0),
  vec2(0.0, 1.0),
  vec2(0.0, 0.0)
);

vec2 positions[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(1.0, 1.0),
  vec2(1.0, 1.0),
  vec2(-1.0, 1.0),
  vec2(-1.0, -1.0)
);

out gl_PerVertex {

	vec4 gl_Position;
};

void main() {

	outUV = vec2(uv[gl_VertexIndex]);
	gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
}
