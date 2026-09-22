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
      path, SDL_GetError());
    return NULL;
  }

  SDL_Log("Imagem '%s' carregada com sucesso (%dx%d).", path, surface->w, surface->h);
  return surface;
}

//------------------------------------------------------------------------------
// Verifica se uma surface no formato RGBA32 esta em escala de cinza, ou seja,
// se todo pixel tem R == G == B. Percorre os pixels diretamente (sem SDL_*
// helper por pixel) para nao pesar em imagens grandes.
//------------------------------------------------------------------------------
static bool image_is_grayscale(SDL_Surface *rgbaSurface)
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
static SDL_Surface *convert_to_grayscale(SDL_Surface *rgbaSurface)
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

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  if (!SDL_CreateWindowAndRenderer("CompVis", WINDOW_WIDTH, WINDOW_HEIGHT, 0,
    &window, &renderer))
  {
    fprintf(stderr, "Erro ao criar a janela e/ou renderizador: %s\n", SDL_GetError());
    SDL_DestroySurface(grayscaleSurface);
    return EXIT_FAILURE;
  }

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

  SDL_DestroySurface(grayscaleSurface);
  grayscaleSurface = NULL;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  renderer = NULL;
  window = NULL;

  return EXIT_SUCCESS;
}
