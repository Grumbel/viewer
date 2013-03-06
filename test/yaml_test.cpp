#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/parse.h>
#include <iostream>
#include <fstream>

void traverse(const YAML::Node& node, const std::string& indent = std::string())
{
  switch(node.Type())
  {
    case YAML::NodeType::Undefined:
      std::cout << indent << "undefined" << std::endl;
      break;

    case YAML::NodeType::Null:
      std::cout << indent << "null" << std::endl;
      break;

    case YAML::NodeType::Scalar:
      std::cout << indent << '"' << node.as<std::string>() << '"' << std::endl;
      break;

    case YAML::NodeType::Sequence:
      for(const auto& it : node)
      {
        if (it.Type() == YAML::NodeType::Scalar)
        {
          std::cout << indent << " - \"" << it.as<std::string>() << '"' << std::endl;
        }
        else
        {
          std::cout << indent << " - " << "[" << std::endl;
          traverse(it, indent + "  ");
          std::cout << indent << "]" << std::endl;
        }
      }
      break;

    case YAML::NodeType::Map:
      for(const auto& it : node)
      {
        std::cout << indent << it.first.as<std::string>() << ": ";
        if (it.second.Type() == YAML::NodeType::Scalar)
        {
          std::cout << '"' << it.second.as<std::string>() << '"' << std::endl;
        }
        else
        {
          std::cout << std::endl;
          traverse(it.second, indent + "  ");
        }
      }
      break;
  }
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " FILENAME..." << std::endl;
  }
  else
  {
    for(int i = 1; i < argc; ++i)
    {
      std::ifstream in(argv[i]);
      if (!in)
      {
        std::cout << "ignoring: " << argv[i] << std::endl;
      }
      else
      {
        YAML::Node node = YAML::Load(in);
        std::cout << "---" << std::endl;
        traverse(node);
        std::cout << "..." << std::endl;
      }
    }
  }

  return 0;
}

/* EOF */

