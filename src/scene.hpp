#ifndef HEADER_SCENE_HPP
#define HEADER_SCENE_HPP

#include <iostream>
#include <string>

class SceneNode;

class Scene
{
public:
  static std::unique_ptr<SceneNode> from_istream(std::istream& in);
  static std::unique_ptr<SceneNode> from_file(const std::string& filename);

private:
  Scene(const Scene&);
  Scene& operator=(const Scene&);
};

#endif

/* EOF */
