#ifndef HEADER_MATERIAL_FACTORY_HPP
#define HEADER_MATERIAL_FACTORY_HPP

#include <string>
#include <unordered_map>

#include "material.hpp"

class MaterialFactory
{
private:
  std::unordered_map<std::string, MaterialPtr> m_materials;

public:
  MaterialFactory();

  MaterialPtr create(const std::string& name);
  MaterialPtr create_phong();
  MaterialPtr create_skybox();

  static MaterialFactory& get() 
  {
    static MaterialFactory* instance = 0;
    if (!instance)
    {
      instance = new MaterialFactory;
    }
    return *instance;
  }

private:
  MaterialFactory(const MaterialFactory&);
  MaterialFactory& operator=(const MaterialFactory&);
};

#endif

/* EOF */
