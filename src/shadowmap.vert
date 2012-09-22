// shadowmap
uniform mat4 ShadowMapMatrix;
varying vec4 ProjShadow;
varying vec4 position;

// phong
varying vec3 normal;
varying vec3 eyeVec;
varying vec3 lightDir[2];


void main(void)
{
  // shadowmap
  ProjShadow  = ShadowMapMatrix * gl_Vertex;

  // phong
  normal = gl_NormalMatrix * gl_Normal;

  vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

  for(int i = 0; i < 2; ++i)
  {
    lightDir[i] = vec3(gl_LightSource[i].position.xyz - vVertex);
  }

  eyeVec = -vVertex;

  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  gl_Position = position = ftransform();
}

/* EOF */
