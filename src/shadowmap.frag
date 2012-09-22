uniform sampler2DShadow ShadowMap;
varying vec4 ProjShadow;

float offset_lookup(sampler2DShadow map,
                    vec4 loc,
                    vec2 offset)
{
  //vec2 texmapscale = vec2(1.0/textureSize(map, 0).x, 1.0/textureSize(map, 0).y);
  //vec2 texmapscale = vec2(1.0/1600.0, 1.0/1000.0);
  vec2 texmapscale = vec2(0.02,0.02);
  
  return shadow2DProj(map, vec4(loc.st + offset * texmapscale, 
                                loc.p-0.006,
                                loc.q)).z;
}

void main (void)
{
  float shadowCoeff = 1.0f;
  if (ProjShadow.w)
  {
    float sum = 0;
    float count = 0;
    for (float y = -1.5; y <= 1.5; y += 1.0)
      for (float x = -1.5; x <= 1.5; x += 1.0)
      {
        sum += offset_lookup(ShadowMap, ProjShadow, vec2(x, y));
        count += 1;
      }

    shadowCoeff = sum / count;
  }
  gl_FragColor = vec4(1.0,1.0,1.0,1) * (shadowCoeff);

  //float shadowCoeff = textureProj(ShadowMap, ProjShadow).r;
  /*
    if (shadowCoeff < (ProjShadow.z-0.00005)/ProjShadow.w)
    {
    gl_FragColor = vec4(0.1,0.1,0.1,1);
    }
    else
    {
    gl_FragColor = vec4(0.9,0.9,0.9,1);
    }*/
}

/* EOF */
