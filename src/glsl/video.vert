#version 330 core
// ---------------------------------------------------------------------------
attribute vec3 position;
attribute vec3 normal;
attribute vec2 texcoord;

varying vec2 frag_uv;

uniform mat4 MVP;

void main(void)
{
  frag_uv = texcoord;

  gl_Position = MVP * vec4(position, 1.0);
}

/* EOF */
