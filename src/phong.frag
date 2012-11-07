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

in vec3 world_normal;
in vec3 frag_normal;
in vec3 frag_position;

uniform samplerCube LightMap;

// ---------------------------------------------------------------------------
// shadow map
uniform sampler2DShadow ShadowMap;
in vec4 shadow_position;

float offset_lookup(sampler2DShadow map,
                    vec4 loc,
                    vec2 offset)
{
  vec2 texmapscale = vec2(1.0/textureSize(map, 0).x, 1.0/textureSize(map, 0).y);
  //vec2 texmapscale = vec2(1.0/1600.0, 1.0/1000.0);
  //vec2 texmapscale = vec2(0.02, 0.02) / loc.q;
  
  return textureProj(map, vec4(loc.st + offset * texmapscale * loc.q, 
                               //loc.p-0.0001,//ortho
                               loc.p, //perspective
                               loc.q));
}

float shadow_value_1()
{
  return textureProj(ShadowMap, shadow_position);
}

float shadow_value_16()
{
  float shadowCoeff = 1.0f;
  if (shadow_position.w)
  {
    float sum = 0;
    for (float y = -1.5; y <= 1.5; y += 1.0)
      for (float x = -1.5; x <= 1.5; x += 1.0)
      {
        sum += offset_lookup(ShadowMap, shadow_position, vec2(x, y));
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
  return (
    offset_lookup(ShadowMap, shadow_position, offset + vec2(-1.5,  0.5)) +
    offset_lookup(ShadowMap, shadow_position, offset + vec2( 0.5,  0.5)) +
    offset_lookup(ShadowMap, shadow_position, offset + vec2(-1.5, -1.5)) +
    offset_lookup(ShadowMap, shadow_position, offset + vec2( 0.5, -1.5)) 
    ) * 0.25;
}

// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
void main(void)
{
  float light = texture(LightMap, world_normal, 3);
  float shadow = max(0.5, shadow_value_4());
  gl_FragColor = vec4(phong_model(frag_position, frag_normal) * shadow + light * shadow * 0.5, 1.0);
}

/* EOF */
