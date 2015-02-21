#version 330 core
// ---------------------------------------------------------------------------
in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec2 frag_uv;

uniform mat4 MVP;

void main(void)
{
  frag_uv = texcoord;

  gl_Position = MVP * vec4(position, 1.0);
}

/* EOF */
