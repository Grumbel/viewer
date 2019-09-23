varying vec2 frag_uv;

uniform sampler2D left_eye;
uniform sampler2D right_eye;

const float steps = 100.0;

void main(void)
{
  vec2 uv_trunc;
  vec2 uv_fract = modf(frag_uv * steps, uv_trunc);

  uv_trunc /= steps;

  uv_fract -= 0.5;
  uv_fract *= 2.0;

  vec4 color = texture(left_eye, uv_trunc);

  if (length(uv_fract.xy) > length(color.rgb)-0.5)
  {
    discard;
  }
  else
  {
    gl_FragColor = color + color;
  }
}

/* EOF */
