#ifndef HEADER_MATERIAL_PARSER_HPP
#define HEADER_MATERIAL_PARSER_HPP

#include <iosfwd>
#include <string>
#include <boost/filesystem/path.hpp>

#include "material.hpp"

class MaterialParser
{
private:

public:
  static MaterialPtr from_file(const boost::filesystem::path& filename);
  static MaterialPtr from_stream(std::istream& filename);

  MaterialParser(std::istream& in);

private:
  MaterialParser(const MaterialParser&);
  MaterialParser& operator=(const MaterialParser&);
};

#endif

/* EOF */
