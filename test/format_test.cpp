#include "format.hpp"

int main()
{
  std::cout << format("Hello World") << std::endl;
  std::cout << format("Hello %s", "World") << std::endl;
  std::cout << format("Hello %d %d", 1, 2) << std::endl;
  std::cout << format("Hello %d %d %d", 1, 2, 3) << std::endl;

  return 0;
}

/* EOF */
