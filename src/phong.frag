struct Light
{
  vec4  diffuse;
  vec4  ambient;
  vec4  specular;
  float shininess;
};

struct Material
{
  vec4  diffuse;
  vec4  ambient;
  vec4  specular;
  vec4  emission;
  float shininess;
};

// phong shading
uniform sampler2D tex;
varying vec3 normal;
varying vec3 lightDir[2];
varying vec3 eyeVec;

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

void main(void)
{
  gl_FragColor = vec4(1, 0, 0, 1); //phong_value();
}

/* EOF */
