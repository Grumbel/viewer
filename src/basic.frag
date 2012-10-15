#version 420

uniform sampler2D diffuse_texture;
uniform vec4 diffuse;

in vec3 texcoord_var;

void main(void)
{
  gl_FragColor = diffuse * texture(diffuse_texture, texcoord_var.st);
}

/* EOF */
