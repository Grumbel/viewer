#include "tokenize.hpp"

#include <stdexcept>

std::vector<std::string> argument_parse(const std::string& line)
{
  std::string arg;
  std::vector<std::string> args;

  enum State { kSkipSpace, kReadArg, kReadQuote, kReadComment };
  State state = kSkipSpace;

  for(auto it = line.begin(); it != line.end();)
  {
    switch(state)
    {
      case kSkipSpace:
        {
          if (isspace(*it))
          {
            // continue skipping space
            ++it;
          }
          else if (*it == '#')
          {
            state = kReadComment;
          }
          else
          {
            state = kReadArg;
          }
        }
        break;

      case kReadArg:
        {
          if (*it == '"')
          {
            state = kReadQuote;
            ++it;
          }
          else if (isspace(*it))
          {
            state = kSkipSpace;
            args.push_back(arg);
            arg.clear();
          }
          else
          {
            arg += *it++;
          }
        }
        break;

      case kReadQuote:
        if (*it == '"')
        {
          state = kReadArg;
          ++it;
        }
        else
        {
          arg += *it++;
        }
        break;

      case kReadComment:
        if (*it == '\n')
        {
          state = kSkipSpace;
        }
        else
        {
          ++it;
        }
        break;
    }
  }

  if (state == kReadQuote)
  {
    throw std::runtime_error("unterminated quoted string");
  }

  if (!arg.empty())
  {
    args.push_back(arg);
  }

  return args;
}

/* EOF */
