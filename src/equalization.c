#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

enum constants
{
  HISTOGRAM_BIN_COUNT = 256,
};

//------------------------------------------------------------------------------
// Equaliza o histograma de uma imagem em escala de cinza (RGBA32, com
// R == G == B em cada pixel), usando a funcao de distribuicao acumulada
// (CDF) do histograma normalizada para o intervalo [0, 255]. Retorna uma
// nova surface RGBA32 equalizada (mantendo o canal alfa original), ou NULL
// em caso de erro. A surface de entrada nao e modificada.
//------------------------------------------------------------------------------
SDL_Surface *equalize_histogram(SDL_Surface *grayscaleSurface)
{
  if (grayscaleSurface == NULL || grayscaleSurface->w <= 0 || grayscaleSurface->h <= 0)
  {
    fprintf(stderr, "Erro: imagem invalida para equalizacao do histograma.\n");
    return NULL;
  }

  if (!SDL_LockSurface(grayscaleSurface))
  {
    fprintf(stderr, "Erro ao acessar os pixels para equalizar o histograma: %s\n",
      SDL_GetError());
    return NULL;
  }

  Uint64 histogram[HISTOGRAM_BIN_COUNT] = { 0 };
  const Uint8 *srcPixels = (const Uint8 *)grayscaleSurface->pixels;
  for (int y = 0; y < grayscaleSurface->h; y++)
  {
    const Uint8 *row = srcPixels + (y * grayscaleSurface->pitch);
    for (int x = 0; x < grayscaleSurface->w; x++)
    {
      histogram[row[x * 4]]++;
    }
  }

  const Uint64 totalPixels = (Uint64)grayscaleSurface->w * (Uint64)grayscaleSurface->h;

  // Funcao de distribuicao acumulada (CDF), normalizada para [0, 255] e
  // arredondada para o inteiro mais proximo, usada como tabela de mapeamento
  // de intensidade original -> intensidade equalizada.
  Uint8 equalizationMap[HISTOGRAM_BIN_COUNT];
  Uint64 cumulative = 0;
  for (int intensity = 0; intensity < HISTOGRAM_BIN_COUNT; intensity++)
  {
    cumulative += histogram[intensity];
    const double normalized = (totalPixels > 0)
      ? (((double)cumulative / (double)totalPixels) * 255.0)
      : 0.0;
    equalizationMap[intensity] = (Uint8)(normalized + 0.5);
  }

  SDL_Surface *equalized = SDL_CreateSurface(grayscaleSurface->w, grayscaleSurface->h,
    SDL_PIXELFORMAT_RGBA32);
  if (equalized == NULL)
  {
    fprintf(stderr, "Erro ao criar a imagem equalizada: %s\n", SDL_GetError());
    SDL_UnlockSurface(grayscaleSurface);
    return NULL;
  }

  Uint8 *dstPixels = (Uint8 *)equalized->pixels;
  for (int y = 0; y < grayscaleSurface->h; y++)
  {
    const Uint8 *srcRow = srcPixels + (y * grayscaleSurface->pitch);
    Uint8 *dstRow = dstPixels + (y * equalized->pitch);

    for (int x = 0; x < grayscaleSurface->w; x++)
    {
      const Uint8 originalIntensity = srcRow[x * 4];
      const Uint8 alpha = srcRow[(x * 4) + 3];
      const Uint8 equalizedIntensity = equalizationMap[originalIntensity];

      dstRow[(x * 4) + 0] = equalizedIntensity;
      dstRow[(x * 4) + 1] = equalizedIntensity;
      dstRow[(x * 4) + 2] = equalizedIntensity;
      dstRow[(x * 4) + 3] = alpha;
    }
  }

  SDL_UnlockSurface(grayscaleSurface);
  return equalized;
}
