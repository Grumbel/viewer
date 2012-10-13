attribute vec3 texcoord;
attribute vec3 normal;
attribute vec3 position;

varying vec3 texcoord_var;
varying vec3 normal_var;

void main(void)
{
  normal_var = normal;
  texcoord_var = vec3(gl_TextureMatrix[0] * vec4(texcoord, 1.0));
  gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}

/* EOF */
