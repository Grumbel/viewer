attribute vec3 texcoord;
attribute vec3 normal;
attribute vec3 position;

varying vec3 texcoord_var;

void main(void)
{
  texcoord_var = gl_TextureMatrix[0] * vec4(texcoord, 1.0) + vec4(normal*0.1, 0.0);
  gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}

/* EOF */
