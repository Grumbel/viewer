#version 420 core

uniform sampler2D tex;
uniform int offset;

uniform float barrel_power;

vec2 barrel_distort(vec2 p)
{
  return vec2(p.x * (1.0 + (barrel_power * p.y*p.y)),
              p.y * (1.0 + (barrel_power * p.x*p.x)));
}

void main(void)
{
  if ((int(gl_FragCoord.y) + offset) % 2 == 0)
  {
    gl_FragColor = texture(tex, 0.5 * (barrel_distort(2.0 * (gl_TexCoord[0].xy - vec2(0.5, 0.5))) + 1.0));

    //gl_FragColor = texture(tex, gl_TexCoord[0].xy);
  }
  else
  {
    discard;
  }
}

/* EOF */
