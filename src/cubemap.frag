#version 420

uniform samplerCube diffuse_texture;
uniform vec4 diffuse;

in vec3 texcoord_var;

void main(void)
{
  gl_FragColor = diffuse * textureCube(diffuse_texture, texcoord_var);
}

/* EOF */
