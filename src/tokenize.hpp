#ifndef HEADER_TOKENIZE_HPP
#define HEADER_TOKENIZE_HPP

#include <string>
#include <vector>

/**
    Split a string on whitespace, quotes can be used to form strings
    with whitespace in them..

    'Hello World'   -> [ "Hello", "World" ]
    '"Hello World"' -> [ "Hello World" ]
    'Hello"123"World' -> [ "Hello123World" ]
 */
std::vector<std::string> argument_parse(const std::string& line);

#endif

/* EOF */
