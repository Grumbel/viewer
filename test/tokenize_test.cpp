#include <iostream>

#include "tokenize.hpp"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "Usage: " << argv[0] << " STRING" << std::endl;
    return 1;
  }
  else
  {
    auto args = argument_parse(argv[1]);
    for(const auto& arg : args)
    {
      std::cout << '"' << arg << '"' << std::endl;
    }
    return 0;
  }
}

/* EOF */
