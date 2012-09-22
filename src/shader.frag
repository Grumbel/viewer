uniform sampler2D tex;

varying vec3 normal, lightDir, eyeVec;

void main (void)
{
  vec4 color = texture2D(tex, gl_TexCoord[0].st);

  vec4 final_color = 
    (gl_FrontLightModelProduct.sceneColor * gl_FrontMaterial.ambient) + 
    (gl_LightSource[0].ambient * gl_FrontMaterial.ambient);
							
  vec3 N = normalize(normal);
  vec3 L = normalize(lightDir);
	
  float lambertTerm = dot(N,L);
	
  if(lambertTerm > 0.0)
  {
    final_color += gl_LightSource[0].diffuse * 
      gl_FrontMaterial.diffuse * 
      color *
      lambertTerm;	
		
    vec3 E = normalize(eyeVec);
    vec3 R = reflect(-L, N);
    float specular = pow( max(dot(R, E), 0.0), 
                          gl_FrontMaterial.shininess );
    final_color += gl_LightSource[0].specular * 
      gl_FrontMaterial.specular * 
      color *
      specular;	
  }

  gl_FragColor = final_color;			
}

/*
  varying vec4 position;
  varying vec3 normal;
	
  void main()
  {

  
  float dist = cross(normal, normalize(position - gl_LightSource[0].position).xyz);

  gl_FragColor = color * dist;
  }
*/

/* EOF */
