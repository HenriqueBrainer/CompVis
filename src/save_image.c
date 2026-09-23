#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

static const char *OUTPUT_IMAGE_PATH = "output_image.png";

//------------------------------------------------------------------------------
// Salva a imagem atualmente exibida na janela principal (o que 'renderer'
// tem desenhado no momento) em output_image.png, sobrescrevendo se ja
// existir. Imprime no terminal o resultado da operacao.
//------------------------------------------------------------------------------
void save_current_image(SDL_Renderer *renderer)
{
  SDL_Surface *renderedSurface = SDL_RenderReadPixels(renderer, NULL);
  if (renderedSurface == NULL)
  {
    fprintf(stderr, "Erro ao capturar a imagem exibida para salvar: %s\n", SDL_GetError());
    return;
  }

  // So para poder informar "criado" ou "sobrescrito" no terminal; o
  // IMG_SavePNG sobrescreve o arquivo de qualquer forma.
  bool fileAlreadyExists = false;
  FILE *existingFile = fopen(OUTPUT_IMAGE_PATH, "rb");
  if (existingFile != NULL)
  {
    fileAlreadyExists = true;
    fclose(existingFile);
  }

  if (!IMG_SavePNG(renderedSurface, OUTPUT_IMAGE_PATH))
  {
    fprintf(stderr, "Erro ao salvar o arquivo '%s': %s\n", OUTPUT_IMAGE_PATH, SDL_GetError());
  }
  else if (fileAlreadyExists)
  {
    printf("Arquivo '%s' sobrescrito.\n", OUTPUT_IMAGE_PATH);
  }
  else
  {
    printf("Arquivo '%s' criado.\n", OUTPUT_IMAGE_PATH);
  }

  SDL_DestroySurface(renderedSurface);
}
