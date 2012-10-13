uniform samplerCube diffuse_texture;
uniform vec4 diffuse;

void main(void)
{
  gl_FragColor = diffuse * textureCube(diffuse_texture, gl_TexCoord[0]);
}

/* EOF */
