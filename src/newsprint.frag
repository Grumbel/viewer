#version 420 core

uniform sampler2D tex;

const float steps = 100.0;

void main(void)
{
  vec2 uv_trunc; 
  vec2 uv_fract = modf(gl_TexCoord[0].xy * steps, uv_trunc);

  uv_trunc /= steps;

  uv_fract -= 0.5;
  uv_fract *= 2.0;

  vec4 color = texture(tex, uv_trunc);

  if (length(uv_fract.xy) > length(color.rgb)-0.5)
    discard;
  else
    gl_FragColor = color + color;
}

/* EOF */
