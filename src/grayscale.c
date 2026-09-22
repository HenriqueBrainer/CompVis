#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

//------------------------------------------------------------------------------
// Verifica se uma surface no formato RGBA32 esta em escala de cinza, ou seja,
// se todo pixel tem R == G == B. Percorre os pixels diretamente (sem SDL_*
// helper por pixel) para nao pesar em imagens grandes.
//------------------------------------------------------------------------------
bool image_is_grayscale(SDL_Surface *rgbaSurface)
{
  const Uint8 *pixels = (const Uint8 *)rgbaSurface->pixels;
  const int pitch = rgbaSurface->pitch;

  for (int y = 0; y < rgbaSurface->h; y++)
  {
    const Uint8 *row = pixels + (y * pitch);
    for (int x = 0; x < rgbaSurface->w; x++)
    {
      const Uint8 r = row[(x * 4) + 0];
      const Uint8 g = row[(x * 4) + 1];
      const Uint8 b = row[(x * 4) + 2];
      if (r != g || g != b)
      {
        return false;
      }
    }
  }

  return true;
}

//------------------------------------------------------------------------------
// Converte uma surface RGBA32 colorida para escala de cinza usando
// Y = 0.2125*R + 0.7154*G + 0.0721*B, mantendo o canal alfa original.
// Retorna uma nova surface (RGBA32, com R == G == B == Y em cada pixel), ou
// NULL em caso de erro.
//------------------------------------------------------------------------------
SDL_Surface *convert_to_grayscale(SDL_Surface *rgbaSurface)
{
  SDL_Surface *gray = SDL_CreateSurface(rgbaSurface->w, rgbaSurface->h, SDL_PIXELFORMAT_RGBA32);
  if (gray == NULL)
  {
    fprintf(stderr, "Erro ao criar a imagem em escala de cinza: %s\n", SDL_GetError());
    return NULL;
  }

  const Uint8 *src = (const Uint8 *)rgbaSurface->pixels;
  Uint8 *dst = (Uint8 *)gray->pixels;
  const int srcPitch = rgbaSurface->pitch;
  const int dstPitch = gray->pitch;

  for (int y = 0; y < rgbaSurface->h; y++)
  {
    const Uint8 *srcRow = src + (y * srcPitch);
    Uint8 *dstRow = dst + (y * dstPitch);

    for (int x = 0; x < rgbaSurface->w; x++)
    {
      const Uint8 r = srcRow[(x * 4) + 0];
      const Uint8 g = srcRow[(x * 4) + 1];
      const Uint8 b = srcRow[(x * 4) + 2];
      const Uint8 a = srcRow[(x * 4) + 3];

      const double yValue = (0.2125 * r) + (0.7154 * g) + (0.0721 * b);
      Uint8 grayValue = (Uint8)(yValue + 0.5); // yValue esta sempre em [0, 255]

      dstRow[(x * 4) + 0] = grayValue;
      dstRow[(x * 4) + 1] = grayValue;
      dstRow[(x * 4) + 2] = grayValue;
      dstRow[(x * 4) + 3] = a;
    }
  }

  return gray;
}
