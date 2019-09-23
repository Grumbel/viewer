#ifndef HEADER_MATERIAL_PARSER_HPP
#define HEADER_MATERIAL_PARSER_HPP

#include <iosfwd>
#include <string>
#include <boost/filesystem/path.hpp>

#include "material.hpp"

class MaterialParser
{
private:
  boost::filesystem::path m_filename;
  boost::filesystem::path m_directory;
  MaterialPtr m_material;

public:
  static MaterialPtr from_file(const boost::filesystem::path& filename);

  MaterialParser(const boost::filesystem::path& filename);
  void parse(std::istream& in);
  MaterialPtr get_material() { return m_material; }

private:
  MaterialParser(const MaterialParser&);
  MaterialParser& operator=(const MaterialParser&);
};

#endif

/* EOF */
