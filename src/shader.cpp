#include "shader.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <sstream>
#include <fstream>
#include <regex>

#include "log.hpp"

namespace {

void include_file(boost::filesystem::path const& filename, std::ostream& os)
{
  std::ifstream in(filename.string());
  if (!in)
  {
    throw std::runtime_error((boost::format("%s: failed to open file") % filename).str());
  }
  else
  {
    std::string line;
    while(std::getline(in, line))
    {
      os << line << '\n';
    }
  }
}

} // namespace

ShaderPtr
Shader::from_file(GLenum type, boost::filesystem::path const& filename,
                  std::vector<std::string> const& defines)
{
  std::ifstream in(filename.string());
  if (!in)
  {
    throw std::runtime_error((boost::format("%s: failed to open file") % filename).str());
  }
  else
  {
    std::vector<std::string> sources;

    // add version declaration
#ifdef HAVE_OPENGLES2
    sources.emplace_back("#version 100\n");
#else
    sources.emplace_back("#version 330 core\n");
#endif

    { // add custom defines
      std::ostringstream os;
      for(auto const& def : defines)
      {
        auto equal_pos = def.find('=');
        if (equal_pos == std::string::npos)
        {
          os << "#define " << def << '\n';
        }
        else
        {
          os << "#define " << def.substr(0, equal_pos) << ' ' << def.substr(equal_pos+1) << '\n';
        }
      }
      sources.emplace_back(os.str());
    }

    { // add the actual source file
      std::regex include_rx("^#\\s*include\\s+\"([^\"]*)\".*$");
      int line_count = 0;
      std::ostringstream os;
      std::string line;
      while(std::getline(in, line))
      {
        std::smatch rx_results;
        if (std::regex_match(line, rx_results, include_rx))
        {
          boost::filesystem::path include_filename(rx_results[1]);
          if (include_filename.is_absolute())
          {
            include_file(include_filename.string(), os);
          }
          else
          {
            include_file((boost::filesystem::path(filename).parent_path() / include_filename).string(),
                         os);
          }
          os << "#line " << line_count << '\n';
        }
        else
        {
          os << line << '\n';
        }

        line_count += 1;
      }
      sources.emplace_back(os.str());
    }

    ShaderPtr shader = std::make_shared<Shader>(type);

    shader->source(sources);
    shader->compile();

    if (!shader->get_compile_status())
    {
      throw std::runtime_error((boost::format("%s: error:\n %s") % filename % shader->get_info_log()).str());
    }

    //log_debug("%s: shader compile successful", filename);

    return shader;
  }
}

Shader::Shader(GLenum type) :
  m_shader()
{
  m_shader = glCreateShader(type);
}

Shader::~Shader()
{
  glDeleteShader(m_shader);
}

void
Shader::source(std::vector<std::string> const& sources)
{
  std::vector<GLint> length_lst(sources.size());
  std::vector<const char*> source_lst(sources.size());
  for(size_t i = 0; i < sources.size(); ++i)
  {
    source_lst[i] = sources[i].c_str();
    length_lst[i] = static_cast<GLint>(sources[i].size());
  }

  glShaderSource(m_shader, sources.size(), source_lst.data(), length_lst.data());
}

void
Shader::source(const std::string& str)
{
  const char* source_lst[] = {str.c_str()};
  GLint length_lst[] = {static_cast<GLint>(str.size())};
  glShaderSource(m_shader, 1, source_lst, length_lst);
}

void
Shader::compile()
{
  glCompileShader(m_shader);
}

std::string
Shader::get_info_log() const
{
  GLint length;
  glGetShaderiv(m_shader, GL_INFO_LOG_LENGTH, &length);

  if (length == 0)
  {
    return std::string();
  }
  else
  {
    GLsizei out_length;
    std::vector<char> str(length);
    glGetShaderInfoLog(m_shader, str.size(), &out_length, str.data());
    return std::string(str.begin(), str.end());
  }
}

bool
Shader::get_compile_status() const
{
  GLint compile_status;
  glGetShaderiv(m_shader, GL_COMPILE_STATUS, &compile_status);
  return compile_status == GL_TRUE;
}

/* EOF */
