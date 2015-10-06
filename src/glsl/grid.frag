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

varying vec3 frag_position;
varying vec3 frag_world_position;
varying vec3 frag_normal;

uniform vec3  grid_offset;
uniform float grid_line_width;
uniform float grid_size;

uniform mat3 NormalMatrix;

struct MaterialInfo
{
  samplerCube reflection_texture;
};

uniform MaterialInfo material;

float grid_value()
{
  //float grid_line_width = 0.001;
  vec3 v = abs(fract((frag_world_position + grid_offset) * grid_size) - vec3(0.5, 0.5, 0.5));
  float grid_dist = 1.0 - float(min(min(v.x, v.y), v.z));

  float attenuation = 1.0; //max(0.0, (1.0f - pow(length(position)/25.0, 1)));
  float p = pow(grid_dist, 12.0) * attenuation + 0.2 * (1.0 - attenuation);

#if defined(TRANSPARENT_GRID)
  if (p < 0.6)
  {
    discard;
  }
#endif

  return p;
}

vec4 reflection()
{
  vec3 o = reflect(frag_position, normalize(frag_normal));

  vec4 col = textureCube(material.reflection_texture, normalize(o));
  vec4 specular = vec4(pow(col.rgb, vec3(20,20,20)) * 5, 1.0);

  vec4 diffuse = textureCubeLod(material.reflection_texture, normalize(o), 5);

  return specular + diffuse;
}

void main (void)
{
  float grid = grid_value();
  gl_FragColor = vec4(grid, grid, grid, 1.0) + reflection();
}

/* EOF */
