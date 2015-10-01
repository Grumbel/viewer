#include "game_controller.hpp"

GameController::GameController() :
  m_controller(nullptr)
{
}

GameController::GameController(SDL_GameController* controller) :
  m_controller(controller)
{
}

GameController::GameController(GameController&& other) :
  m_controller(other.m_controller)
{
  other.m_controller = nullptr;
}

GameController::~GameController()
{
  if (m_controller)
  {
    SDL_GameControllerClose(m_controller);
  }
}

/* EOF */
