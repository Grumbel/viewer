// shadow map
uniform sampler2DShadow ShadowMap;
varying vec4 ProjShadow;

// phong shading
uniform sampler2D tex;
varying vec3 normal;
varying vec3 lightDir[2];
varying vec3 eyeVec;

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

float shadow_value()
{
  float shadowCoeff = 1.0f;
  if (ProjShadow.w)
  {
    float sum = 0;
    for (float y = -1.5; y <= 1.5; y += 1.0)
      for (float x = -1.5; x <= 1.5; x += 1.0)
      {
        sum += offset_lookup(ShadowMap, ProjShadow, vec2(x, y));
      }

    shadowCoeff = sum / 16;
  }
  return shadowCoeff;
}

vec4 phong_value()
{
  vec4 color = texture2D(tex, gl_TexCoord[0].st);

  vec4 final_color = 
    (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient);
    

  for(int i = 0; i < 2; ++i)
  {
    final_color += gl_LightSource[i].ambient * gl_FrontMaterial.ambient;

    vec3 N = normalize(normal);
    vec3 L = normalize(lightDir[i]);
	
    float lambertTerm = dot(N,L);
	
    if(lambertTerm > 0.0)
    {
      final_color += gl_LightSource[i].diffuse * 
        gl_FrontMaterial.diffuse * 
        color *
        lambertTerm;	
		
      vec3 E = normalize(eyeVec);
      vec3 R = reflect(-L, N);
      float specular = pow( max(dot(R, E), 0.0), 
                            gl_FrontMaterial.shininess );
      final_color += gl_LightSource[i].specular * 
        gl_FrontMaterial.specular * 
        color *
        specular;	
    }
  }

  return final_color;
}

void main (void)
{
  float shadow = shadow_value();
  vec4  color  = phong_value();
  gl_FragColor = color * shadow;

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
