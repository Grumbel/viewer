#version 420

in vec3  position;
in vec3  point_size;
in float alpha;

out float frag_alpha;

void main(void)
{
  vec4 pos = gl_ModelViewMatrix * vec4(position, 1.0);
  gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
  gl_PointSize = point_size * 1.0f/pos.z;
  frag_alpha = alpha;
}

/* EOF */
