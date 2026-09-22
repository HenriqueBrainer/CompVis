#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

//------------------------------------------------------------------------------
// Carrega uma imagem a partir de 'path' usando SDL_image, tratando de forma
// robusta os cenarios de erro pedidos no enunciado:
//   - arquivo inexistente / nao pode ser aberto;
//   - conteudo que nao seja um formato de imagem valido/suportado.
// Retorna a SDL_Surface carregada, ou NULL em caso de falha. A mensagem de
// erro correspondente ja e exibida em stderr antes do retorno.
//------------------------------------------------------------------------------
SDL_Surface *load_image(const char *path)
{
  if (path == NULL || path[0] == '\0')
  {
    fprintf(stderr, "Erro: nenhum caminho de imagem foi informado.\n");
    return NULL;
  }

  // Abrimos o arquivo manualmente primeiro. Isso permite diferenciar um erro
  // de "arquivo nao encontrado" (problema de I/O) de um erro de "formato de
  // imagem invalido" (o arquivo existe, mas o conteudo nao e uma imagem que
  // o SDL_image consiga decodificar).
  SDL_IOStream *io = SDL_IOFromFile(path, "rb");
  if (io == NULL)
  {
    fprintf(stderr,
      "Erro: nao foi possivel abrir o arquivo '%s'. "
      "Verifique se o caminho esta correto e se o arquivo existe. (%s)\n",
      path, SDL_GetError());
    return NULL;
  }

  // 'true' faz o IMG_Load_IO fechar o SDL_IOStream automaticamente,
  // mesmo em caso de erro, evitando vazamento de recursos.
  SDL_Surface *surface = IMG_Load_IO(io, true);
  if (surface == NULL)
  {
    fprintf(stderr,
      "Erro: nao foi possivel carregar a imagem '%s'. "
      "O arquivo pode estar corrompido ou em um formato de imagem "
      "nao suportado pelo SDL_image. (%s)\n",
      path, SDL_GetError());
    return NULL;
  }

  SDL_Log("Imagem '%s' carregada com sucesso (%dx%d).", path, surface->w, surface->h);
  return surface;
}
