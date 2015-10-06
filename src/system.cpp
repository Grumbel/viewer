#include "system.hpp"

#include <sstream>

#include "log.hpp"

System
System::create()
{
  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
  {
    std::ostringstream msg;
    msg << "Couldn't initialize SDL: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  else
  {
    atexit(SDL_Quit);
  }

  return System();
}

System::System()
{
}

Window
System::create_gl_window(std::string const& title, int width, int height, bool fullscreen, int anti_aliasing)
{
  //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1); // vsync
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  if (anti_aliasing)
  {
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 ); // boolean value, either it's enabled or not
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, anti_aliasing ); // 0, 2, or 4 for number of samples
  }

  //SDL_WM_SetIcon(IMG_Load(Pathname("icon.png").get_sys_path().c_str()), NULL);
  SDL_Window* window = SDL_CreateWindow(title.c_str(),
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        width, height,
                                        SDL_WINDOW_OPENGL |
                                        SDL_WINDOW_RESIZABLE |
                                        (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
  if (!window)
  {
    throw std::runtime_error("Couldn't create window");
  }
  else
  {
#ifdef HAVE_OPENGLES2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
      throw std::runtime_error("failed to create GLContext");
    }

    return Window(window, gl_context);
  }
}

Joystick
System::create_joystick()
{
  SDL_Joystick* joystick = nullptr;
  log_info("SDL_NumJoysticks: %d", SDL_NumJoysticks());
  if (SDL_NumJoysticks() > 0)
  {
    joystick = SDL_JoystickOpen(0);
    return Joystick(joystick);
  }
  else
  {
    return Joystick();
  }
}

GameController
System::create_gamecontroller()
{
  for (int i = 0; i < SDL_NumJoysticks(); ++i)
  {
    if (SDL_IsGameController(i))
    {
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller)
      {
        return GameController(controller);
      }
      else
      {
        std::ostringstream out;
        out << "Could not open gamecontroller: " << i << ": " << SDL_GetError();
        throw std::runtime_error(out.str());
      }
    }
  }

  return {};
}

/* EOF */
