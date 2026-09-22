//
// Uso:
//   programa caminho_da_imagem.ext

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

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

void shutdown_app(void);
SDL_Surface *load_image(const char *path);
bool image_is_grayscale(SDL_Surface *rgbaSurface);
SDL_Surface *convert_to_grayscale(SDL_Surface *rgbaSurface);
bool analyze_histogram(SDL_Surface *grayscaleSurface, HistogramAnalysis *analysis);
void render_histogram(SDL_Renderer *renderer, const HistogramAnalysis *analysis);
int gui_run(SDL_Surface *grayscaleSurface, const HistogramAnalysis *histogram);

void shutdown_app(void)
{
  SDL_Log("shutdown()");
  SDL_Quit();
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  atexit(shutdown_app);

  if (argc < 2)
  {
    fprintf(stderr, "Uso: %s caminho_da_imagem.ext\n", argv[0]);
    return EXIT_FAILURE;
  }
  const char *imagePath = argv[1];

  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    fprintf(stderr, "Erro ao iniciar a SDL: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Surface *imageSurface = load_image(imagePath);
  if (imageSurface == NULL)
  {
    // A mensagem de erro especifica ja foi impressa em load_image().
    return EXIT_FAILURE;
  }

  // Normaliza para um formato de pixel conhecido (RGBA32), independente do
  // formato original do arquivo, para poder ler/escrever pixels diretamente
  // de forma consistente daqui em diante.
  SDL_Surface *workingSurface = SDL_ConvertSurface(imageSurface, SDL_PIXELFORMAT_RGBA32);
  SDL_DestroySurface(imageSurface);
  imageSurface = NULL;
  if (workingSurface == NULL)
  {
    fprintf(stderr, "Erro ao converter a imagem para o formato de trabalho: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  const bool inputIsGrayscale = image_is_grayscale(workingSurface);
  printf("Imagem de entrada: %s.\n", inputIsGrayscale ? "escala de cinza" : "colorida");

  // A partir daqui, apenas a versao em escala de cinza e usada como base
  // para o restante do programa.
  SDL_Surface *grayscaleSurface = NULL;
  if (inputIsGrayscale)
  {
    grayscaleSurface = workingSurface;
  }
  else
  {
    grayscaleSurface = convert_to_grayscale(workingSurface);
    SDL_DestroySurface(workingSurface);
    workingSurface = NULL;
    if (grayscaleSurface == NULL)
    {
      return EXIT_FAILURE;
    }
  }

  HistogramAnalysis histogram;
  if (!analyze_histogram(grayscaleSurface, &histogram))
  {
    SDL_DestroySurface(grayscaleSurface);
    return EXIT_FAILURE;
  }
  printf("Media de intensidade: %.2f - imagem %s.\n",
    histogram.mean, histogram.brightnessClass);
  printf("Desvio padrao: %.2f - contraste %s.\n",
    histogram.standardDeviation, histogram.contrastClass);


  return gui_run(grayscaleSurface, &histogram);
}
