#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>

enum constants
{
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

static const double DARK_IMAGE_LIMIT = 85.0;
static const double LIGHT_IMAGE_LIMIT = 170.0;
static const double LOW_CONTRAST_LIMIT = 42.5;
static const double HIGH_CONTRAST_LIMIT = 85.0;

//------------------------------------------------------------------------------
// Calcula o histograma da surface em escala de cinza e, a partir dele, a
// media e o desvio padrao populacional das intensidades. Como a surface de
// trabalho e RGBA32 e possui R == G == B, basta ler o canal vermelho.
//------------------------------------------------------------------------------
bool analyze_histogram(SDL_Surface *grayscaleSurface,
  HistogramAnalysis *analysis)
{
  if (grayscaleSurface == NULL || analysis == NULL ||
    grayscaleSurface->w <= 0 || grayscaleSurface->h <= 0)
  {
    fprintf(stderr, "Erro: imagem invalida para o calculo do histograma.\n");
    return false;
  }

  SDL_memset(analysis, 0, sizeof(*analysis));

  if (!SDL_LockSurface(grayscaleSurface))
  {
    fprintf(stderr, "Erro ao acessar os pixels para calcular o histograma: %s\n",
      SDL_GetError());
    return false;
  }

  const Uint8 *pixels = (const Uint8 *)grayscaleSurface->pixels;
  for (int y = 0; y < grayscaleSurface->h; y++)
  {
    const Uint8 *row = pixels + (y * grayscaleSurface->pitch);
    for (int x = 0; x < grayscaleSurface->w; x++)
    {
      const Uint8 intensity = row[x * 4];
      analysis->bins[intensity]++;
    }
  }

  SDL_UnlockSurface(grayscaleSurface);

  analysis->pixelCount = (Uint64)grayscaleSurface->w *
    (Uint64)grayscaleSurface->h;

  // Os acumuladores long double evitam perda de precisao em imagens grandes.
  long double intensitySum = 0.0L;
  long double squaredIntensitySum = 0.0L;
  for (int intensity = 0; intensity < HISTOGRAM_BIN_COUNT; intensity++)
  {
    const Uint64 frequency = analysis->bins[intensity];
    if (frequency > analysis->largestBin)
    {
      analysis->largestBin = frequency;
    }

    intensitySum += (long double)intensity * (long double)frequency;
    squaredIntensitySum += (long double)intensity * (long double)intensity *
      (long double)frequency;
  }

  analysis->mean = (double)(intensitySum / analysis->pixelCount);
  double variance = (double)(squaredIntensitySum / analysis->pixelCount) -
    (analysis->mean * analysis->mean);
  if (variance < 0.0)
  {
    // Pode ocorrer apenas por arredondamento de ponto flutuante.
    variance = 0.0;
  }
  analysis->standardDeviation = sqrt(variance);

  if (analysis->mean < DARK_IMAGE_LIMIT)
  {
    analysis->brightnessClass = "ESCURA";
  }
  else if (analysis->mean <= LIGHT_IMAGE_LIMIT)
  {
    analysis->brightnessClass = "MEDIA";
  }
  else
  {
    analysis->brightnessClass = "CLARA";
  }

  if (analysis->standardDeviation < LOW_CONTRAST_LIMIT)
  {
    analysis->contrastClass = "BAIXO";
  }
  else if (analysis->standardDeviation < HIGH_CONTRAST_LIMIT)
  {
    analysis->contrastClass = "MEDIO";
  }
  else
  {
    analysis->contrastClass = "ALTO";
  }

  return true;
}
//------------------------------------------------------------------------------
// Desenha o histograma normalizado pelo maior bin. Assim, imagens pequenas e
// grandes usam toda a altura disponivel sem que as barras sejam cortadas.
// SDL_RenderDebugText faz parte da propria SDL3 e evita depender de uma fonte
// instalada no sistema para mostrar os resultados da analise.
//------------------------------------------------------------------------------
void render_histogram(SDL_Renderer *renderer,
  const HistogramAnalysis *analysis)
{
  const float graphLeft = 32.0f;
  const float graphTop = 42.0f;
  const float graphWidth = 336.0f;
  const float graphHeight = 260.0f;

  SDL_SetRenderDrawColor(renderer, 224, 228, 236, 255);
  SDL_RenderDebugText(renderer, 24.0f, 16.0f, "HISTOGRAMA (0-255)");

  SDL_FRect graphBackground = {
    graphLeft - 1.0f, graphTop - 1.0f, graphWidth + 2.0f, graphHeight + 2.0f
  };
  SDL_SetRenderDrawColor(renderer, 12, 14, 20, 255);
  SDL_RenderFillRect(renderer, &graphBackground);

  // A moldura fica fora da area dos bins para nao esconder as barras das
  // intensidades 0 e 255.
  SDL_SetRenderDrawColor(renderer, 190, 197, 211, 255);
  SDL_RenderRect(renderer, &graphBackground);

  // Linhas de referencia ajudam a comparar visualmente as frequencias.
  SDL_SetRenderDrawColor(renderer, 48, 53, 64, 255);
  for (int division = 1; division < 4; division++)
  {
    const float y = graphTop + (graphHeight * division / 4.0f);
    SDL_RenderLine(renderer, graphLeft, y, graphLeft + graphWidth, y);
  }

  if (analysis->largestBin > 0)
  {
    SDL_SetRenderDrawColor(renderer, 92, 173, 255, 255);
    for (int intensity = 0; intensity < HISTOGRAM_BIN_COUNT; intensity++)
    {
      const float x0 = graphLeft +
        ((float)intensity * graphWidth / HISTOGRAM_BIN_COUNT);
      const float x1 = graphLeft +
        ((float)(intensity + 1) * graphWidth / HISTOGRAM_BIN_COUNT);
      const float barHeight = (float)(
        ((double)analysis->bins[intensity] / analysis->largestBin) * graphHeight);

      if (barHeight > 0.0f)
      {
        SDL_FRect bar = {
          x0,
          graphTop + graphHeight - barHeight,
          SDL_max(1.0f, x1 - x0),
          barHeight
        };
        SDL_RenderFillRect(renderer, &bar);
      }
    }
  }

  SDL_SetRenderDrawColor(renderer, 190, 197, 211, 255);
  SDL_RenderDebugText(renderer, graphLeft, graphTop + graphHeight + 8.0f, "0");
  SDL_RenderDebugText(renderer, graphLeft + (graphWidth / 2.0f) - 12.0f,
    graphTop + graphHeight + 8.0f, "128");
  SDL_RenderDebugText(renderer, graphLeft + graphWidth - 24.0f,
    graphTop + graphHeight + 8.0f, "255");

  SDL_FRect informationPanel = { 20.0f, 338.0f, 360.0f, 124.0f };
  SDL_SetRenderDrawColor(renderer, 35, 39, 49, 255);
  SDL_RenderFillRect(renderer, &informationPanel);
  SDL_SetRenderDrawColor(renderer, 190, 197, 211, 255);
  SDL_RenderRect(renderer, &informationPanel);

  char text[96];
  SDL_snprintf(text, sizeof(text), "MEDIA: %.2f - %s",
    analysis->mean, analysis->brightnessClass);
  SDL_SetRenderDrawColor(renderer, 235, 238, 245, 255);
  SDL_RenderDebugText(renderer, 36.0f, 360.0f, text);

  SDL_snprintf(text, sizeof(text), "DESVIO PADRAO: %.2f - %s",
    analysis->standardDeviation, analysis->contrastClass);
  SDL_RenderDebugText(renderer, 36.0f, 388.0f, text);

  SDL_snprintf(text, sizeof(text), "PIXELS: %llu",
    (unsigned long long)analysis->pixelCount);
  SDL_RenderDebugText(renderer, 36.0f, 416.0f, text);
}
