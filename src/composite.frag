#version 420 core

in vec2 frag_uv;

uniform sampler2D left_eye;
uniform sampler2D right_eye;
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
    gl_FragColor = texture(left_eye, 0.5 * (barrel_distort(2.0 * (frag_uv - vec2(0.5, 0.5))) + 1.0));
  }
  else
  {
    gl_FragColor = texture(right_eye, 0.5 * (barrel_distort(2.0 * (frag_uv - vec2(0.5, 0.5))) + 1.0));
  }
}

/* EOF */
