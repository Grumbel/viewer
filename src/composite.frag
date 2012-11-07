#version 420 core

uniform sampler2D tex;
uniform int offset;

void main(void)
{
  if ((int(gl_FragCoord.y) + offset) % 2 == 0)
  {
    gl_FragColor = texture(tex, gl_TexCoord[0].xy);
  }
  else
  {
    discard;
  }
}

/* EOF */
