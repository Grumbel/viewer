uniform samplerCube diffuse_texture;
uniform vec4 diffuse;

varying vec3 texcoord_var;

void main(void)
{
  gl_FragColor = diffuse * textureCube(diffuse_texture, texcoord_var);
}

/* EOF */
