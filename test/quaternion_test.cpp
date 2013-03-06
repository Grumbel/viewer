#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
  printf("vec3(%8.2f %8.2f %8.2f)", vec.x, vec.y, vec.z);
  return os;
}

glm::vec3 degrees(const glm::vec3& euler)
{
  return glm::vec3(glm::degrees(euler.x),
                   glm::degrees(euler.y),
                   glm::degrees(euler.z));
}

std::ostream& operator<<(std::ostream& os, const glm::quat& quat)
{
  printf("quat(%8.2f %8.2f %8.2f %8.2f)", quat.w, quat.x, quat.y, quat.z);
  return os;
}

int main()
{
  glm::quat rot90(glm::vec3(glm::radians(90.0f),
                            glm::radians(0.0f), 
                            glm::radians(0.0f)));

  glm::quat rate(glm::vec3(glm::radians(10.0f),
                            glm::radians(20.0f), 
                            glm::radians(30.0f)));

  glm::quat total = rot90 * rate;
  
  glm::vec3 euler = glm::eulerAngles(total);

  //printf("%8.2f %8.2f %8.2f\n", glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));
  std::cout << rot90 << std::endl;
  //  std::cout << rot180 << std::endl;
  //std::cout << total << std::endl;
  std::cout << degrees(euler) << std::endl;

  return 0;
}

/* EOF */
