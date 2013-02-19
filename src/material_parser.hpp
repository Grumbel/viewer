#ifndef HEADER_MATERIAL_PARSER_HPP
#define HEADER_MATERIAL_PARSER_HPP

#include <iosfwd>
#include <string>
#include <boost/filesystem/path.hpp>

#include "material.hpp"

class MaterialParser
{
private:
  std::string m_filename;
  MaterialPtr m_material;

public:
  static MaterialPtr from_file(const boost::filesystem::path& filename);
  static MaterialPtr from_stream(std::istream& in);

  MaterialParser(const std::string& filename);
  void parse(std::istream& in);
  MaterialPtr get_material() { return m_material; }

private:
  MaterialParser(const MaterialParser&);
  MaterialParser& operator=(const MaterialParser&);
};

#endif

/* EOF */
