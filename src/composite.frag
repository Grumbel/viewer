#version 420 core

uniform sampler2D tex;
uniform int offset;

uniform float barrel_power;

// http://github.prideout.net/barrel-distortion/
// Given a vec2 in [-1,+1], generate a texture coord in [0,+1]
vec2 barrel_distort(vec2 p)
{
  float theta  = atan(p.y, p.x);
  float radius = length(p);
  radius = pow(radius, barrel_power);
  p.x = radius * cos(theta);
  p.y = radius * sin(theta);
  return 0.5 * (p + 1.0);
}

void main(void)
{
  if ((int(gl_FragCoord.y) + offset) % 2 == 0)
  {
    gl_FragColor = texture(tex, barrel_distort(2.0 * (gl_TexCoord[0].xy - vec2(0.5, 0.5))));

    //gl_FragColor = texture(tex, gl_TexCoord[0].xy);
  }
  else
  {
    discard;
  }
}

/* EOF */
