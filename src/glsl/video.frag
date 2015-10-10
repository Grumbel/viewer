//  Simple 3D Model Viewer
//  Copyright (C) 2012-2013 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

uniform mat4 MVP;

#if defined(REFLECTION_TEXTURE)
varying vec3 frag_position;
varying vec3 frag_normal;
#endif

varying vec2 frag_uv;

uniform sampler2D texture_diff;

#ifdef REFLECTION_TEXTURE
struct MaterialInfo
{
  samplerCube reflection_texture;
};

uniform MaterialInfo material;
#endif

#if defined(VIDEO3D)
uniform int eye_index;
#endif

#if !defined(VIDEO_LEFT_OFFSET)
#  define VIDEO_LEFT_OFFSET vec2(0, 0)
#endif

#if !defined(VIDEO_LEFT_SCALE)
#  define VIDEO_LEFT_SCALE vec2(1, 1)
#endif

#if !defined(VIDEO_RIGHT_OFFSET)
#  define VIDEO_RIGHT_OFFSET vec2(0, 0)
#endif

#if !defined(VIDEO_RIGHT_SCALE)
#  define VIDEO_RIGHT_SCALE vec2(1, 1)
#endif

// ---------------------------------------------------------------------------
void main(void)
{
#if defined(VIDEO3D)
  vec2 uv;
  if (eye_index == 0)
  {
    uv = VIDEO_LEFT_OFFSET + VIDEO_LEFT_SCALE * frag_uv;
  }
  else
  {
    uv = VIDEO_RIGHT_OFFSET + VIDEO_RIGHT_SCALE * frag_uv;
  }
  vec3 diff = texture2D(texture_diff, uv).rgb;
#else
  vec3 diff = texture2D(texture_diff, frag_uv).rgb;
#endif

#if defined(REFLECTION_TEXTURE)
  vec3 o = reflect(frag_position, normalize(frag_normal));
  vec4 col = textureCube(material.reflection_texture, normalize(o));
  diff = mix(diff, col.rgb,
             0.25 + 0.5 * (1.0 - dot(vec3(0, 0, 1), normalize(frag_normal))));
#endif

  gl_FragColor = vec4(diff, 1.0);
}

/* EOF */
