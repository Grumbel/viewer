#ifndef HEADER_GAME_CONTROLLER_HPP
#define HEADER_GAME_CONTROLLER_HPP

#include <SDL.h>

class GameController
{
private:
  SDL_GameController* m_controller;

public:
  GameController();
  GameController(SDL_GameController* controller);
  GameController(GameController&& other);
  ~GameController();

  SDL_GameController* get_handle() { return m_controller; }

private:
  GameController(const GameController&) = delete;
  GameController& operator=(const GameController&) = delete;
};

#endif

/* EOF */
