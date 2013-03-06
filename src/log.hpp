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

#ifndef HEADER_LOG_HPP
#define HEADER_LOG_HPP

#include <iostream>

#include "format.hpp"

#define log_info(...)  do { ::format(std::cout, "[INF] " __VA_ARGS__); std::endl(std::cout); } while(false)
#define log_warn(...)  do { ::format(std::cout, "[WAR] " __VA_ARGS__); std::endl(std::cout); } while(false)
#define log_error(...) do { ::format(std::cout, "[ERR] " __VA_ARGS__); std::endl(std::cout); } while(false)
#define log_debug(...) do { ::format(std::cout, "[DBG] " __VA_ARGS__); std::endl(std::cout); } while(false)

#endif

/* EOF */
