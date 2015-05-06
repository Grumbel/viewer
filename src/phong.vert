#version 330 core
// ---------------------------------------------------------------------------
attribute vec3 position;
attribute vec3 normal;

varying vec3 world_normal;
varying vec3 frag_normal;
varying vec3 frag_position;

// ---------------------------------------------------------------------------
uniform mat4 ShadowMapMatrix;
varying vec4 shadow_position;
// ---------------------------------------------------------------------------

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

void main(void)
{
  shadow_position = ShadowMapMatrix * vec4(position, 1.0);

  frag_position = vec3(ModelViewMatrix * vec4(position, 1.0));
  frag_normal = NormalMatrix * normal;
  world_normal = normal; 

  gl_Position = MVP * vec4(position, 1.0);
}

/* EOF */
