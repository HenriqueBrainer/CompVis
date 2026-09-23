/*
---Turma 7G - Disciplina: Computação Visual – Projeto 1---
  Integrantes:
  
  Bruna Gonçalves Corte David
  10425696

  Henrique Brainer Costa
  10420717

  João Pedro Queiroz de Andrade
  10425822

  Júlia Andrade
  10428513
*/

#include <stdbool.h>
#include <SDL3/SDL.h>

enum constants
{
  WINDOW_WIDTH = 1024,
  WINDOW_HEIGHT = 768,
};

//------------------------------------------------------------------------------
// Reposiciona a janela principal: centralizada no monitor primario, exceto
// se 'width'/'height' excederem a resolucao atual do monitor primario, caso
// em que o canto superior esquerdo vai para (0, 0) da tela.
//------------------------------------------------------------------------------
static void reposition_main_window(SDL_Window *window, int width, int height)
{
  const SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();
  SDL_Rect displayBounds;
  bool exceedsDisplay = false;

  if (SDL_GetDisplayBounds(primaryDisplay, &displayBounds))
  {
    exceedsDisplay = (width > displayBounds.w) || (height > displayBounds.h);
  }

  if (exceedsDisplay)
  {
    SDL_SetWindowPosition(window, 0, 0);
  }
  else
  {
    SDL_SetWindowPosition(window,
      SDL_WINDOWPOS_CENTERED_DISPLAY(primaryDisplay),
      SDL_WINDOWPOS_CENTERED_DISPLAY(primaryDisplay));
  }
}

//------------------------------------------------------------------------------
// Retangulo de desenho da imagem para o modo de exibicao atual:
//   - showOriginalResolution == true:  tamanho nativo da imagem;
//   - showOriginalResolution == false: fixo em WINDOW_WIDTH x WINDOW_HEIGHT.
// Nao mexe na janela; so calcula o retangulo (usado tanto ao trocar de modo
// quanto ao recriar a textura por outro motivo, ex. equalizacao).
//------------------------------------------------------------------------------
SDL_FRect image_display_rect(bool showOriginalResolution, int nativeWidth, int nativeHeight)
{
  SDL_FRect rect = { .x = 0.0f, .y = 0.0f };

  if (showOriginalResolution)
  {
    rect.w = (float)nativeWidth;
    rect.h = (float)nativeHeight;
  }
  else
  {
    rect.w = (float)WINDOW_WIDTH;
    rect.h = (float)WINDOW_HEIGHT;
  }

  return rect;
}

//------------------------------------------------------------------------------
// Redimensiona a janela principal para o modo de exibicao (tamanho nativo da
// imagem ou fixo WINDOW_WIDTH x WINDOW_HEIGHT) e a reposiciona de acordo.
//------------------------------------------------------------------------------
void apply_main_window_size(SDL_Window *window, bool showOriginalResolution,
  int nativeWidth, int nativeHeight)
{
  const int windowWidth = showOriginalResolution ? nativeWidth : WINDOW_WIDTH;
  const int windowHeight = showOriginalResolution ? nativeHeight : WINDOW_HEIGHT;

  SDL_SetWindowSize(window, windowWidth, windowHeight);
  reposition_main_window(window, windowWidth, windowHeight);
}
