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
#include <boost/format.hpp>

namespace logging {

inline void unpack(boost::format& fmt)
{
}

template<typename Head, typename ...Rest> 
void unpack(boost::format& fmt, const Head& head, Rest... rest)
{
  unpack(fmt % head, rest...);
}

template<typename ...Arg>
void format(const std::string& str)
{
  std::cout << str;
}

template<typename ...Arg>
void print(const std::string& str, Arg... arg)
{
  boost::format fmt(str);
  unpack(fmt, arg...);
  std::cout << fmt;
  std::cout << std::endl;
}

template<typename Out, typename ...Arg>
void print(Out& out, const std::string& str, Arg... arg)
{
  boost::format fmt(str);
  unpack(fmt, arg...);
  out << fmt;
  out << std::endl;
}

template<typename Out, typename Head>
void print(Out& out, const Head& head)
{
  out << head << std::endl;
}

/*
template<typename ...Arg>
void print(Arg... arg)
{
  print(std::cout, arg...);
}

template<typename Out, typename Head, typename ...Arg>
void print(Out& out, const Head& head, Arg... arg)
{
  out << head;
  print(out, arg...);
}
*/

} // namespace logging

#define log_info(...)  logging::print(std::cout, "[INFO] " __VA_ARGS__)
#define log_warn(...)  logging::print(std::cout, "[WARN] " __VA_ARGS__)
#define log_error(...)  logging::print(std::cout, "[ERROR] " __VA_ARGS__)
#define log_debug(...) logging::print(std::cout, "[DEBUG]" __VA_ARGS__)

#endif

/* EOF */
