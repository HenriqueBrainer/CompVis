//
// Uso:
//   programa caminho_da_imagem.ext

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

//------------------------------------------------------------------------------
enum constants
{
  WINDOW_WIDTH = 1024,
  WINDOW_HEIGHT = 768,
};

//------------------------------------------------------------------------------
static void shutdown_app(void)
{
  SDL_Log("shutdown()");
  SDL_Quit();
}

//------------------------------------------------------------------------------
// Carrega uma imagem a partir de 'path' usando SDL_image, tratando de forma
// robusta os cenarios de erro pedidos no enunciado:
//   - arquivo inexistente / nao pode ser aberto;
//   - conteudo que nao seja um formato de imagem valido/suportado.
// Retorna a SDL_Surface carregada, ou NULL em caso de falha. A mensagem de
// erro correspondente ja e exibida em stderr antes do retorno.
//------------------------------------------------------------------------------
static SDL_Surface *load_image(const char *path)
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
      path, IMG_GetError());
    return NULL;
  }

  SDL_Log("Imagem '%s' carregada com sucesso (%dx%d).", path, surface->w, surface->h);
  return surface;
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

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  if (!SDL_CreateWindowAndRenderer("CompVis", WINDOW_WIDTH, WINDOW_HEIGHT, 0,
    &window, &renderer))
  {
    fprintf(stderr, "Erro ao criar a janela e/ou renderizador: %s\n", SDL_GetError());
    SDL_DestroySurface(imageSurface);
    return EXIT_FAILURE;
  }

  SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
  if (imageTexture == NULL)
  {
    fprintf(stderr, "Erro ao criar a textura a partir da imagem: %s\n", SDL_GetError());
    SDL_DestroySurface(imageSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return EXIT_FAILURE;
  }

  // A partir daqui a surface original nao e mais necessaria (ja temos a
  // textura pronta para desenho); mantenha-a se ela for usada como base para
  // as proximas etapas (escala de cinza, histograma etc.).
  SDL_DestroySurface(imageSurface);
  imageSurface = NULL;

  SDL_FRect imageRect = { .x = 0.0f, .y = 0.0f };
  SDL_GetTextureSize(imageTexture, &imageRect.w, &imageRect.h);

  SDL_Event event;
  bool isRunning = true;
  while (isRunning)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT)
      {
        isRunning = false;
      }
    }

    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, imageTexture, NULL, &imageRect);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(imageTexture);
  imageTexture = NULL;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  renderer = NULL;
  window = NULL;

  return EXIT_SUCCESS;
}
