uniform mat4 ShadowMapMatrix;
varying vec4 ProjShadow;

void main(void)
{
  ProjShadow  = ShadowMapMatrix * gl_Vertex;
  gl_Position = ftransform();
}

/* EOF */
