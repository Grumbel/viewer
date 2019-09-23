#ifndef HEADER_MATERIAL_PARSER_HPP
#define HEADER_MATERIAL_PARSER_HPP

#include <iosfwd>
#include <string>
#include <filesystem>

#include "material.hpp"

class MaterialParser
{
private:
  std::filesystem::path m_filename;
  std::filesystem::path m_directory;
  MaterialPtr m_material;

public:
  static MaterialPtr from_file(const std::filesystem::path& filename);

  MaterialParser(const std::filesystem::path& filename);
  void parse(std::istream& in);
  MaterialPtr get_material() { return m_material; }

private:
  MaterialParser(const MaterialParser&);
  MaterialParser& operator=(const MaterialParser&);
};

#endif

/* EOF */
