#version 330 core

attribute vec3 position;

uniform mat4 MVP;

void main(void)
{
  gl_Position = MVP * vec4(position, 1.0);
}

/* EOF */
