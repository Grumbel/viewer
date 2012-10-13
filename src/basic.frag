uniform sampler2D diffuse_texture;
uniform vec4 diffuse;

void main(void)
{
  gl_FragColor = diffuse * texture(diffuse_texture, gl_TexCoord[0].st);
}

/* EOF */
