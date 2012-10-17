#version 420 core

struct LightInfo
{
  vec3  diffuse;
  vec3  ambient;
  vec3  specular;

  vec3 position;
};

struct MaterialInfo
{
  vec3  diffuse;
  vec3  ambient;
  vec3  specular;
  vec3  emission;
  float shininess;
};

uniform LightInfo light;
uniform MaterialInfo material;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
//uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

in vec3 frag_normal;
in vec3 frag_position;

vec3 phong_model(vec3 position, vec3 normal)
{
  vec3 intensity = light.ambient * material.ambient;

  vec3 N = normalize(normal);
  vec3 L = normalize(light.position - position); // eye dir
	
  float lambertTerm = dot(N, L);

  if(lambertTerm > 0.0)
  {
    intensity += light.diffuse * material.diffuse * lambertTerm;
		
    vec3 E = normalize(-position); // eye vec
    vec3 R = reflect(-L, N);

    float specular = pow( max(dot(R, E), 0.0), material.shininess );

    intensity += light.specular * material.specular * specular;	
  }

  return intensity;
}

void main(void)
{
  gl_FragColor = vec4(phong_model(frag_position, frag_normal), 1.0);
}

/* EOF */
