#ifndef HEADER_SCENE_HPP
#define HEADER_SCENE_HPP

#include <iostream>
#include <string>

class SceneNode;

class Scene
{
public:
  static SceneNode* from_istream(MaterialPtr material, std::istream& in);
  static SceneNode* from_file(MaterialPtr material, const std::string& filename);

private:
  Scene(const Scene&);
  Scene& operator=(const Scene&);
};

#endif

/* EOF */
