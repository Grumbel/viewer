//  Simple 3D Model Viewer
//  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

#include "model.hpp"

#include <fstream>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>

#include "log.hpp"
#include "render_context.hpp"

void
Model::draw(RenderContext const& context)
{
  if (!m_material)
  {
    log_error("Model::draw: no material set");
  }
  else
  {
    OpenGLState state;

    MaterialPtr material;

    if (context.get_override_material())
    {
      if (m_material->cast_shadow())
      {
        material = context.get_override_material();
      }
    }
    else
    {
      material = m_material;
    }

    if (material)
    {
      material->apply(context);

      for (MeshLst::iterator i = m_meshes.begin(); i != m_meshes.end(); ++i)
      {
        (*i)->draw();
      }
    }

    glUseProgram(0);
  }
}

/* EOF */
