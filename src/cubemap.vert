#version 330 core

in vec3 texcoord;
in vec3 normal;
in vec3 position;

out vec3 texcoord_var;
out vec3 normal_var;

uniform mat4 MVP;

void main(void)
{
  normal_var = normal;
  texcoord_var = texcoord; //;vec3(gl_TextureMatrix[0] * vec4(texcoord, 1.0));
  gl_Position = MVP * vec4(position, 1.0);
}

/* EOF */
