varying vec2 frag_uv;

uniform sampler2D left_eye;
uniform sampler2D right_eye;


#if defined(BARREL_DISTORT)

uniform float barrel_power;

vec2 barrel_distort(vec2 p)
{
  return vec2(p.x * (1.0 + (barrel_power * p.y*p.y)),
              p.y * (1.0 + (barrel_power * p.x*p.x)));
}

vec4 left_eye_color(vec2 uv)
{
  return texture(left_eye, 0.5 * (barrel_distort(2.0 * (uv - vec2(0.5, 0.5))) + 1.0));
}

vec4 right_eye_color(vec2 uv)
{
  return texture(right_eye, 0.5 * (barrel_distort(2.0 * (uv - vec2(0.5, 0.5))) + 1.0));
}

#else

vec4 left_eye_color(vec2 uv)
{
  return texture(left_eye, uv);
}

vec4 right_eye_color(vec2 uv)
{
  return texture(right_eye, uv);
}

#endif

#if defined(INTERLACED_COMPOSITION)

vec4 fragment_color() // interlaced
{
  if ((int(gl_FragCoord.y)) % 2 == 0)
  {
    return left_eye_color(frag_uv);
  }
  else
  {
    return right_eye_color(frag_uv);
  }
}

#elif defined(ANAGLYPH_COMPOSITION)

vec4 fragment_color() // anaglyph
{
  vec4 left  = left_eye_color(frag_uv);
  vec4 right = right_eye_color(frag_uv);
  return vec4(left.r, right.g, 0.0, 1.0);
}

#elif defined(CROSSEYE_COMPOSITION)

vec4 fragment_color() // crosseye
{
  if (frag_uv.x > 0.5)
  {
    return right_eye_color(vec2(frag_uv.x*2.0 - 1.0, frag_uv.y));
  }
  else
  {
    return left_eye_color(vec2(frag_uv.x*2.0, frag_uv.y));
  }
}

#elif defined(DEPTH_COMPOSITION)

vec4 fragment_color() // from_depth
{
  return left_eye_color(frag_uv);
}

#else

vec4 fragment_color() // mono
{
  return left_eye_color(frag_uv);
}

#endif

void main()
{
  gl_FragColor = fragment_color();
}

/* EOF */
