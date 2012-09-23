// shadow map
uniform sampler2DShadow ShadowMap;
varying vec4 ProjShadow;
varying vec4 position;

// phong shading
uniform sampler2D tex;
varying vec3 normal;
varying vec3 lightDir[2];
varying vec3 eyeVec;

// normal mapping
uniform sampler2D normalMap;
varying vec3 lightDir_;
varying vec3 viewDir;

float offset_lookup(sampler2DShadow map,
                    vec4 loc,
                    vec2 offset)
{
  vec2 texmapscale = vec2(1.0/textureSize(map, 0).x, 1.0/textureSize(map, 0).y);
  //vec2 texmapscale = vec2(1.0/1600.0, 1.0/1000.0);
  //vec2 texmapscale = vec2(0.02, 0.02) / loc.q;
  
  return shadow2DProj(map, vec4(loc.st + offset * texmapscale * loc.q, 
                                //loc.p-0.0001,//ortho
                                loc.p-0.015, //perspective
                                loc.q)).z;
}

float shadow_value_1()
{
  return shadow2DProj(ShadowMap, ProjShadow - vec4(0,0,0.0001,0.0)).z;
}

float shadow_value_16()
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

float shadow_value_4()
{
  vec2 offset = vec2(greaterThan(fract(gl_FragCoord.xy * 0.5),
                                 vec2(0.25, 0.25)));
  //vec2 offset;
  //offset.x = fract(gl_FragCoord.xy * 0.5);
  //offset.y = fract(gl_FragCoord.xy * 0.5);
  offset.y += offset.x;  // y ^= x in floating point

  if (offset.y > 1.1)
    offset.y = 0;
  return (offset_lookup(ShadowMap, ProjShadow, offset +
                        vec2(-1.5, 0.5)) +
          offset_lookup(ShadowMap, ProjShadow, offset +
                        vec2(0.5, 0.5)) +
          offset_lookup(ShadowMap, ProjShadow, offset +
                        vec2(-1.5, -1.5)) +
          offset_lookup(ShadowMap, ProjShadow, offset +
                        vec2(0.5, -1.5)) ) * 0.25;
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

// normal vector is in tangent space, not world space
vec3 normal_value()
{
  vec3 l = lightDir_;
  float atten = max(0.0, 1.0 - dot(l, l));

  l = normalize(l);

  vec3 n = normalize(texture2D(normalMap, gl_TexCoord[0].st).xyz * 2.0 - 1.0);
  vec3 v = normalize(viewDir);
  vec3 h = normalize(l + v);

  return h;
}

void main (void)
{
  float shadow = shadow_value_4();
  vec4  color  = phong_value();
  gl_FragColor = color * (shadow + 0.5)/2.0;

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
