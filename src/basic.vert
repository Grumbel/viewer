#version 330 core

in vec3 texcoord;
in vec3 normal;
in vec3 position;

out vec3 texcoord_var;

void main(void)
{
  texcoord_var = gl_TextureMatrix[0] * vec4(texcoord, 1.0) + vec4(normal*0.1, 0.0);
  gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}

/* EOF */
