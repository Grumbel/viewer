#ifndef HEADER_LOG_HPP
#define HEADER_LOG_HPP

#include <iostream>
#include <boost/format.hpp>

namespace logging {

void unpack(boost::format& fmt)
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
}

template<typename Out, typename ...Arg>
void print(Out& out, const std::string& str, Arg... arg)
{
  boost::format fmt(str);
  unpack(fmt, arg...);
  out << fmt;
}

template<typename Out, typename Head>
void print(Out& out, const Head& head)
{
  out << head;
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

#define log_info(...) logging::print(std::cout, __VA_ARGS__)

#endif

/* EOF */
