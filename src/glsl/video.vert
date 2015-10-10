// ---------------------------------------------------------------------------
attribute vec3 position;
attribute vec3 normal;
attribute vec2 texcoord;

#if defined(REFLECTION_TEXTURE)
varying vec3 frag_position;
varying vec3 frag_normal;
#endif

varying vec2 frag_uv;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

void main(void)
{
  frag_uv = texcoord;

#if defined(REFLECTION_TEXTURE)
  frag_position = vec3(ModelViewMatrix * vec4(position, 1.0));
  frag_normal = NormalMatrix * normal;
#endif

  gl_Position = MVP * vec4(position, 1.0);
}

/* EOF */
