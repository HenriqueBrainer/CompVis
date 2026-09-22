//
// Uso:
//   programa caminho_da_imagem.ext

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

//------------------------------------------------------------------------------
enum constants
{
  WINDOW_WIDTH = 1024,
  WINDOW_HEIGHT = 768,
  SECONDARY_WINDOW_WIDTH = 400,
  SECONDARY_WINDOW_HEIGHT = 620,
  HISTOGRAM_BIN_COUNT = 256,
};

// Os limites de luminosidade dividem a faixa de intensidade [0, 255] em
// tres partes. Para o contraste, a mesma ideia e aplicada a faixa teorica
// do desvio padrao [0, 127.5] de uma imagem de 8 bits.
static const double DARK_IMAGE_LIMIT = 85.0;
static const double LIGHT_IMAGE_LIMIT = 170.0;
static const double LOW_CONTRAST_LIMIT = 42.5;
static const double HIGH_CONTRAST_LIMIT = 85.0;

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
// Calcula o histograma da surface em escala de cinza e, a partir dele, a
// media e o desvio padrao populacional das intensidades. Como a surface de
// trabalho e RGBA32 e possui R == G == B, basta ler o canal vermelho.
//------------------------------------------------------------------------------
static bool analyze_histogram(SDL_Surface *grayscaleSurface,
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
static void render_histogram(SDL_Renderer *renderer,
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
    render_histogram(secondaryRenderer, &histogram);
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
