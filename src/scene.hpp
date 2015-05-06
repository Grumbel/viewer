#ifndef HEADER_SCENE_HPP
#define HEADER_SCENE_HPP

#include <iostream>
#include <string>
#include <boost/filesystem/path.hpp>

class SceneNode;

class Scene
{
public:
  static std::unique_ptr<SceneNode> from_istream(std::istream& in);
  static std::unique_ptr<SceneNode> from_file(const std::string& filename);

private:
  boost::filesystem::path m_directory;
  std::unique_ptr<SceneNode> m_node;

public:
  Scene();

  void set_directory(const boost::filesystem::path& path);
  void parse_istream(std::istream& in);
  std::unique_ptr<SceneNode> get_node();

private:
  Scene(const Scene&);
  Scene& operator=(const Scene&);
};

#endif

/* EOF */
