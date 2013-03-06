#include <yaml-cpp/yaml.h>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/node/parse.h>
#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
  YAML::Node node;
  node = "From a Node: Hello\n\n World";

  std::cout << node << std::endl;

  YAML::Emitter out;
  //out.SetIndent(4);
  //out.SetMapFormat(YAML::Flow);
  out.SetMapFormat(YAML::Block);
  out.SetStringFormat(YAML::SingleQuoted);
  
  out << YAML::BeginDoc;
  out << YAML::BeginSeq;
  out << node;
  out << YAML::Value << YAML::SingleQuoted << "Hello World\n\nTestest\n\n";
  out << YAML::Value << YAML::DoubleQuoted << "Hello World\n\nTestest\n\n";
  out << YAML::Value << YAML::Literal << "Hello World\n\nTestest";
  out << YAML::Value << YAML::Literal << "Value";
  out << YAML::BeginMap;
  //out << YAML::Flow;
  //out << YAML::Block;
  out << YAML::Key << "method";
  out << YAML::Value << "least\n\nsquares";
  //out << YAML::Comment("should we change this method?");
  out << YAML::EndMap;
  out << YAML::EndSeq;
  out << YAML::EndDoc;

  std::cout << out.c_str() << std::endl;

  if (!out.good())
  {
   std::cout << "Emitter error: " << out.GetLastError() << "\n";
  }

  return 0;
}

/* EOF */

