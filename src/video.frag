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

#version 420 core

uniform mat4 MVP;

in vec2 frag_uv;

uniform sampler2D texture_diff;

// ---------------------------------------------------------------------------
void main(void)
{
  vec3 diff = texture2D(texture_diff, frag_uv).rgb;
  gl_FragColor = vec4(diff, 1.0);
}

/* EOF */
