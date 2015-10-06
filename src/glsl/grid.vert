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

#version 330 core

attribute vec3 position;
attribute vec3 normal;

varying vec3 frag_position;
varying vec3 frag_world_position;
varying vec3 frag_normal;

uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

void main(void)
{
  frag_position = vec3(ModelViewMatrix * vec4(position, 1.0));
  frag_world_position = vec3(ModelMatrix * vec4(position, 1.0));
  frag_normal = NormalMatrix * normal;

  gl_Position = MVP * vec4(position, 1.0);
}

/* EOF */
