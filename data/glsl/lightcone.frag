uniform sampler2D diffuse_texture;

varying float frag_alpha;

void main(void)
{
  gl_FragColor = vec4(texture(diffuse_texture, gl_PointCoord.st).rgb, frag_alpha);
}

/* EOF */
