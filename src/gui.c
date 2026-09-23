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
bool analyze_histogram(SDL_Surface *grayscaleSurface, HistogramAnalysis *analysis);
SDL_Surface *equalize_histogram(SDL_Surface *grayscaleSurface);
SDL_FRect image_display_rect(bool showOriginalResolution, int nativeWidth, int nativeHeight);
void apply_main_window_size(SDL_Window *window, bool showOriginalResolution,
  int nativeWidth, int nativeHeight);
void save_current_image(SDL_Renderer *renderer);
void render_text(SDL_Renderer *renderer, float x, float y, const char *text);
void text_measure(const char *text, float *outWidth, float *outHeight);

//------------------------------------------------------------------------------
// Botao desenhado com primitivas da SDL, com 3 estados visuais.
//------------------------------------------------------------------------------
typedef enum ButtonState
{
  BUTTON_STATE_NEUTRAL,
  BUTTON_STATE_HOVER,
  BUTTON_STATE_PRESSED,
} ButtonState;

typedef struct Button
{
  SDL_FRect rect;
  const char *label;
  ButtonState state;
} Button;

static bool point_in_rect(float x, float y, SDL_FRect rect)
{
  return x >= rect.x && x <= (rect.x + rect.w) &&
    y >= rect.y && y <= (rect.y + rect.h);
}

static void draw_button(SDL_Renderer *renderer, const Button *button)
{
  Uint8 r, g, b;
  switch (button->state)
  {
    case BUTTON_STATE_PRESSED:
      r = 20; g = 60; b = 140; // azul escuro
      break;
    case BUTTON_STATE_HOVER:
      r = 110; g = 170; b = 255; // azul claro
      break;
    case BUTTON_STATE_NEUTRAL:
    default:
      r = 50; g = 110; b = 220; // azul
      break;
  }

  SDL_SetRenderDrawColor(renderer, r, g, b, 255);
  SDL_RenderFillRect(renderer, &button->rect);
  SDL_SetRenderDrawColor(renderer, 235, 238, 245, 255);
  SDL_RenderRect(renderer, &button->rect);

  // Mede o texto de verdade (com a fonte do projeto) para centralizar,
  // em vez de estimar largura por caractere.
  float textWidth = 0.0f;
  float textHeight = 0.0f;
  text_measure(button->label, &textWidth, &textHeight);
  const float textX = button->rect.x + ((button->rect.w - textWidth) / 2.0f);
  const float textY = button->rect.y + ((button->rect.h - textHeight) / 2.0f);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  render_text(renderer, textX, textY, button->label);
}

//------------------------------------------------------------------------------
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

  int imageNativeWidth = 0;
  int imageNativeHeight = 0;
  {
    float nativeW = 0.0f;
    float nativeH = 0.0f;
    SDL_GetTextureSize(imageTexture, &nativeW, &nativeH);
    imageNativeWidth = (int)nativeW;
    imageNativeHeight = (int)nativeH;
  }

  // A janela principal comeca em 1024x768 (item 3), entao o modo de
  // resolucao inicial e o "fixo" (nao o original da imagem).
  bool isShowingOriginalResolution = false;
  SDL_FRect imageRect = image_display_rect(isShowingOriginalResolution, imageNativeWidth, imageNativeHeight);

  // Estado da equalizacao: a surface e a analise originais (recebidas pelo
  // gui_run) ficam guardadas; a versao equalizada e calculada sob demanda,
  // no primeiro clique, e reaproveitada nos cliques seguintes.
  SDL_Surface *originalSurface = grayscaleSurface;
  HistogramAnalysis originalAnalysis = *histogram;

  SDL_Surface *equalizedSurface = NULL;
  HistogramAnalysis equalizedAnalysis = { 0 };
  bool isShowingEqualized = false;

  HistogramAnalysis activeAnalysis = originalAnalysis;

  Button equalizeButton =
  {
    .rect = { .x = 80.0f, .y = 480.0f, .w = 240.0f, .h = 44.0f },
    .label = "Equalizar",
    .state = BUTTON_STATE_NEUTRAL,
  };

  Button resolutionButton =
  {
    .rect = { .x = 80.0f, .y = 536.0f, .w = 240.0f, .h = 44.0f },
    .label = "Resolucao original",
    .state = BUTTON_STATE_NEUTRAL,
  };

  const SDL_WindowID secondaryWindowID = SDL_GetWindowID(secondaryWindow);

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

        case SDL_EVENT_KEY_DOWN:
          // Item 7: tecla S salva a imagem atualmente exibida na janela
          // principal. Ignora key repeat para nao salvar varias vezes se a
          // tecla ficar pressionada.
          if (event.key.key == SDLK_s && !event.key.repeat)
          {
            save_current_image(renderer);
          }
          break;

        case SDL_EVENT_MOUSE_MOTION:
          if (event.motion.windowID == secondaryWindowID)
          {
            if (equalizeButton.state != BUTTON_STATE_PRESSED)
            {
              equalizeButton.state = point_in_rect(event.motion.x, event.motion.y, equalizeButton.rect)
                ? BUTTON_STATE_HOVER : BUTTON_STATE_NEUTRAL;
            }
            if (resolutionButton.state != BUTTON_STATE_PRESSED)
            {
              resolutionButton.state = point_in_rect(event.motion.x, event.motion.y, resolutionButton.rect)
                ? BUTTON_STATE_HOVER : BUTTON_STATE_NEUTRAL;
            }
          }
          break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
          if (event.button.button == SDL_BUTTON_LEFT && event.button.windowID == secondaryWindowID)
          {
            if (point_in_rect(event.button.x, event.button.y, equalizeButton.rect))
            {
              equalizeButton.state = BUTTON_STATE_PRESSED;
            }
            else if (point_in_rect(event.button.x, event.button.y, resolutionButton.rect))
            {
              resolutionButton.state = BUTTON_STATE_PRESSED;
            }
          }
          break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
          if (event.button.button != SDL_BUTTON_LEFT)
          {
            break;
          }

          if (equalizeButton.state == BUTTON_STATE_PRESSED)
          {
            const bool releasedInsideButton =
              event.button.windowID == secondaryWindowID &&
              point_in_rect(event.button.x, event.button.y, equalizeButton.rect);

            if (releasedInsideButton)
            {
              isShowingEqualized = !isShowingEqualized;

              if (isShowingEqualized && equalizedSurface == NULL)
              {
                equalizedSurface = equalize_histogram(originalSurface);
                if (equalizedSurface == NULL)
                {
                  fprintf(stderr, "Erro: equalizacao falhou; mantendo a imagem original.\n");
                  isShowingEqualized = false;
                }
                else if (!analyze_histogram(equalizedSurface, &equalizedAnalysis))
                {
                  fprintf(stderr, "Erro: nao foi possivel analisar o histograma equalizado.\n");
                  SDL_DestroySurface(equalizedSurface);
                  equalizedSurface = NULL;
                  isShowingEqualized = false;
                }
              }

              SDL_Surface *activeSurface = isShowingEqualized ? equalizedSurface : originalSurface;
              activeAnalysis = isShowingEqualized ? equalizedAnalysis : originalAnalysis;
              equalizeButton.label = isShowingEqualized ? "Ver original" : "Equalizar";

              SDL_DestroyTexture(imageTexture);
              imageTexture = SDL_CreateTextureFromSurface(renderer, activeSurface);
              if (imageTexture == NULL)
              {
                fprintf(stderr, "Erro ao recriar a textura da imagem: %s\n", SDL_GetError());
              }
              else
              {
                // O tamanho da textura nao muda com a equalizacao; mantemos
                // o modo de resolucao (original ou 1024x768) que ja estava
                // ativo, em vez de resetar o tamanho de desenho.
                imageRect = image_display_rect(isShowingOriginalResolution,
                  imageNativeWidth, imageNativeHeight);
              }
            }

            equalizeButton.state = releasedInsideButton
              ? BUTTON_STATE_HOVER : BUTTON_STATE_NEUTRAL;
          }

          if (resolutionButton.state == BUTTON_STATE_PRESSED)
          {
            const bool releasedInsideButton =
              event.button.windowID == secondaryWindowID &&
              point_in_rect(event.button.x, event.button.y, resolutionButton.rect);

            if (releasedInsideButton)
            {
              isShowingOriginalResolution = !isShowingOriginalResolution;

              apply_main_window_size(window, isShowingOriginalResolution,
                imageNativeWidth, imageNativeHeight);
              imageRect = image_display_rect(isShowingOriginalResolution,
                imageNativeWidth, imageNativeHeight);

              resolutionButton.label = isShowingOriginalResolution ? "1024x768" : "Resolucao original";
            }

            resolutionButton.state = releasedInsideButton
              ? BUTTON_STATE_HOVER : BUTTON_STATE_NEUTRAL;
          }
          break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderClear(renderer);
    if (imageTexture != NULL)
    {
      SDL_RenderTexture(renderer, imageTexture, NULL, &imageRect);
    }
    SDL_RenderPresent(renderer);

    SDL_SetRenderDrawColor(secondaryRenderer, 24, 24, 28, 255);
    SDL_RenderClear(secondaryRenderer);
    render_histogram(secondaryRenderer, &activeAnalysis);
    draw_button(secondaryRenderer, &equalizeButton);
    draw_button(secondaryRenderer, &resolutionButton);
    SDL_RenderPresent(secondaryRenderer);
  }

  if (imageTexture != NULL)
  {
    SDL_DestroyTexture(imageTexture);
    imageTexture = NULL;
  }

  if (equalizedSurface != NULL)
  {
    SDL_DestroySurface(equalizedSurface);
    equalizedSurface = NULL;
  }

  SDL_DestroySurface(originalSurface);
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
