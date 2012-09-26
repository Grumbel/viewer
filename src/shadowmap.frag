#define SHADOW_MAPPING 1
#define NORMAL_MAPPING 1

// phong shading
uniform sampler2D tex;
varying vec3 normal;
varying vec3 lightDir[2];
varying vec3 eyeVec;

#ifdef SHADOW_MAPPING
// shadow map
uniform sampler2DShadow ShadowMap;
uniform float shadowmap_bias;
varying vec4 ProjShadow;

#  define SHADOW_MAPPING_FUNCTION shadow_value_4()
#else
#  define SHADOW_MAPPING_FUNCTION 1.0f
#endif

#ifdef NORMAL_MAPPING
// normal mapping
uniform sampler2D normalMap;
varying vec3 lightDir_;
varying vec3 viewDir;
#endif

// grid
varying vec4 vertex_position;
varying vec4 position;
uniform vec4 grid_offset;
uniform float grid_line_width;
uniform float grid_size;

// cubemap
uniform samplerCube cubemap;
uniform vec4 world_eye_pos;
varying vec4 world_position;
varying vec3 world_normal;

uniform sampler2D grass;
uniform sampler2D cliff;

#ifdef SHADOW_MAPPING
float offset_lookup(sampler2DShadow map,
                    vec4 loc,
                    vec2 offset)
{
  vec2 texmapscale = vec2(1.0/textureSize(map, 0).x, 1.0/textureSize(map, 0).y);
  //vec2 texmapscale = vec2(1.0/1600.0, 1.0/1000.0);
  //vec2 texmapscale = vec2(0.02, 0.02) / loc.q;
  
  return shadow2DProj(map, vec4(loc.st + offset * texmapscale * loc.q, 
                                //loc.p-0.0001,//ortho
                                loc.p + shadowmap_bias, //perspective
                                loc.q)).z;
}

float shadow_value_1()
{
  return shadow2DProj(ShadowMap, ProjShadow - vec4(0, 0, shadowmap_bias, 0)).z;
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
  return (
    offset_lookup(ShadowMap, ProjShadow, offset + vec2(-1.5,  0.5)) +
    offset_lookup(ShadowMap, ProjShadow, offset + vec2( 0.5,  0.5)) +
    offset_lookup(ShadowMap, ProjShadow, offset + vec2(-1.5, -1.5)) +
    offset_lookup(ShadowMap, ProjShadow, offset + vec2( 0.5, -1.5)) 
    ) * 0.25;
}
#endif

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

float grid_value()
{
  //float grid_line_width = 0.001;
  vec4 v = abs(fract((vertex_position + grid_offset) * grid_size) - vec4(0.5, 0.5, 0.5, 0.0));
  float grid_dist = 1.0 - float(min(min(v.x, v.y), v.z));

  float attenuation = 1.0; //max(0.0, (1.0f - pow(length(position)/25.0, 1)));
  float p = pow(grid_dist, 12.0) * attenuation + 0.2 * (1.0 - attenuation);
  //if (p < 0.6)
  //  discard;
  return p;
}

#ifdef NORMAL_MAPPING
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
#endif

vec4 cube_map()
{
  vec3 o = reflect(world_position - world_eye_pos, normalize(world_normal));
  
  vec4 col = textureCubeLod(cubemap, normalize(o), 3);// * textureCube(cubemap, world_normal);
  vec4 specular = vec4(pow(col.rgb, vec3(20,20,20)), 1.0);
  
  vec4 diffuse = textureCubeLod(cubemap, normalize(world_normal), 5);

  return /*specular + */diffuse;
  
  //return textureCube(cubemap, world_position - world_eye_pos);
  //return vec4(world_normal, 1.0); 
  //return world_eye_pos;
  //return world_position;
  //return vec4(o, 1.0);
}

void main (void)
{
  float shadow = SHADOW_MAPPING_FUNCTION;
  vec4 grid = grid_value();
  /*
  //vec4  color  = phong_value() + cube_map(); // + vec4(0, 1, 0, 1) * grid_value();
  vec4 color = phong_value() + cube_map() + (grid * ((vec4(1,1,1,1) - pow(length(fwidth(position)), 0.2))));
  gl_FragColor = color * (shadow + 0.5)/2.0;
  */

  vec3 f = abs(normalize(world_normal * vec3(1,1,1)));
  vec4 color = 
    texture(grass, world_position.xy*0.2) * f.z +
    texture(grass, world_position.yz*0.2) * f.x +
    texture(cliff, world_position.xz*0.2) * f.y;

  gl_FragColor = color * cube_map();
}

/* EOF */
