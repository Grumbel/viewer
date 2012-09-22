uniform sampler2DShadow ShadowMap;
varying vec4 ProjShadow;

void main (void)
{
  if (shadow2DProj(ShadowMap, ProjShadow).r < (ProjShadow.z-0.00005)/ProjShadow.w)
  {
    gl_FragColor = vec4(0.1,0.1,0.1,1);
  }
  else
  {
    gl_FragColor = vec4(1,1,1,1);
  }
}

/* EOF */
