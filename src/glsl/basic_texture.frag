#version 330 core

varying vec2 frag_uv;

uniform sampler2D texture_diff;

void main()
{
  gl_FragColor = texture(texture_diff, frag_uv);
}

/* EOF */
