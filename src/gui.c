#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

enum constants
{
  WINDOW_WIDTH = 1024,
  WINDOW_HEIGHT = 768,
  SECONDARY_WINDOW_WIDTH = 400,
  SECONDARY_WINDOW_HEIGHT = 620,
  HISTOGRAM_BIN_COUNT = 256,
};

typedef struct HistogramAnalysis
{
  Uint64 bins[HISTOGRAM_BIN_COUNT];
  Uint64 pixelCount;
  Uint64 largestBin;
  double mean;
  double standardDeviation;
  const char *brightnessClass;
  const char *contrastClass;
} HistogramAnalysis;

void render_histogram(SDL_Renderer *renderer, const HistogramAnalysis *analysis);

int gui_run(SDL_Surface *grayscaleSurface, const HistogramAnalysis *histogram)
{
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  if (!SDL_CreateWindowAndRenderer("CompVis", WINDOW_WIDTH, WINDOW_HEIGHT, 0,
    &window, &renderer))
  {
    fprintf(stderr, "Erro ao criar a janela e/ou renderizador: %s\n", SDL_GetError());
    SDL_DestroySurface(grayscaleSurface);
    return EXIT_FAILURE;
  }

  const SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();
  SDL_SetWindowPosition(window,
    SDL_WINDOWPOS_CENTERED_DISPLAY(primaryDisplay),
    SDL_WINDOWPOS_CENTERED_DISPLAY(primaryDisplay));

  SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, grayscaleSurface);
  if (imageTexture == NULL)
  {
    fprintf(stderr, "Erro ao criar a textura a partir da imagem: %s\n", SDL_GetError());
    SDL_DestroySurface(grayscaleSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return EXIT_FAILURE;
  }

  // grayscaleSurface nao e liberada aqui: alem da textura, ela ainda vai
  // ser usada mais adiante no programa.

  SDL_Window *secondaryWindow = SDL_CreateWindow("CompVis - Histograma",
    SECONDARY_WINDOW_WIDTH, SECONDARY_WINDOW_HEIGHT, 0);
  if (secondaryWindow == NULL)
  {
    fprintf(stderr, "Erro ao criar a janela secundaria: %s\n", SDL_GetError());
    SDL_DestroyTexture(imageTexture);
    SDL_DestroySurface(grayscaleSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return EXIT_FAILURE;
  }
  SDL_SetWindowPosition(secondaryWindow, 0, 0);
  if (!SDL_SetWindowParent(secondaryWindow, window))
  {
    fprintf(stderr, "Aviso: nao foi possivel associar a janela secundaria "
      "como filha da janela principal: %s\n", SDL_GetError());
  }

  SDL_Renderer *secondaryRenderer = SDL_CreateRenderer(secondaryWindow, NULL);
  if (secondaryRenderer == NULL)
  {
    fprintf(stderr, "Erro ao criar o renderizador da janela secundaria: %s\n", SDL_GetError());
    SDL_DestroyWindow(secondaryWindow);
    SDL_DestroyTexture(imageTexture);
    SDL_DestroySurface(grayscaleSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return EXIT_FAILURE;
  }

  SDL_FRect imageRect = { .x = 0.0f, .y = 0.0f };
  SDL_GetTextureSize(imageTexture, &imageRect.w, &imageRect.h);

  SDL_Event event;
  bool isRunning = true;
  while (isRunning)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_EVENT_QUIT:
          isRunning = false;
          break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          // As duas janelas trabalham juntas (imagem + histograma/controles),
          // entao fechar qualquer uma delas encerra o programa.
          isRunning = false;
          break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, imageTexture, NULL, &imageRect);
    SDL_RenderPresent(renderer);

    SDL_SetRenderDrawColor(secondaryRenderer, 24, 24, 28, 255);
    SDL_RenderClear(secondaryRenderer);
    render_histogram(secondaryRenderer, histogram);
    SDL_RenderPresent(secondaryRenderer);
  }

  SDL_DestroyTexture(imageTexture);
  imageTexture = NULL;

  SDL_DestroySurface(grayscaleSurface);
  grayscaleSurface = NULL;

  SDL_DestroyRenderer(secondaryRenderer);
  SDL_DestroyWindow(secondaryWindow);
  secondaryRenderer = NULL;
  secondaryWindow = NULL;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  renderer = NULL;
  window = NULL;

  return EXIT_SUCCESS;

}
