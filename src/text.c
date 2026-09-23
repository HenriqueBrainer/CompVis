#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Fonte usada em todos os textos do programa (histograma e botoes):
// DejaVu Sans (licenca livre, ver assets/fonts/DejaVuSans-LICENSE.txt).
// Fica junto do projeto em assets/fonts/, entao o caminho relativo funciona
// da mesma forma em qualquer sistema operacional -- nao depende de nenhuma
// fonte que precise estar instalada no SO.
static const char *FONT_PATH = "assets/fonts/DejaVuSans.ttf";
static const float FONT_POINT_SIZE = 16.0f;

static TTF_Font *font = NULL;

//------------------------------------------------------------------------------
// Inicializa a SDL_ttf e carrega a fonte do programa. Deve ser chamada uma
// vez, depois de SDL_Init(), e antes de qualquer render_text()/
// text_measure(). Retorna false em caso de erro (mensagem ja impressa em
// stderr); os outros dois arquivos continuam funcionando sem travar, so que
// sem desenhar texto.
//------------------------------------------------------------------------------
bool text_init(void)
{
  if (!TTF_Init())
  {
    fprintf(stderr, "Erro ao iniciar a SDL_ttf: %s\n", SDL_GetError());
    return false;
  }

  font = TTF_OpenFont(FONT_PATH, FONT_POINT_SIZE);
  if (font == NULL)
  {
    fprintf(stderr, "Erro ao carregar a fonte '%s': %s\n", FONT_PATH, SDL_GetError());
    TTF_Quit();
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// Libera a fonte e finaliza a SDL_ttf. Chamar uma vez, no encerramento do
// programa (mesmo que text_init() tenha falhado, e seguro chamar).
//------------------------------------------------------------------------------
void text_shutdown(void)
{
  if (font != NULL)
  {
    TTF_CloseFont(font);
    font = NULL;
  }
  TTF_Quit();
}

//------------------------------------------------------------------------------
// Renderiza 'text' na posicao (x, y), usando a cor de desenho atual do
// renderer (SDL_SetRenderDrawColor) -- mesma convencao que SDL_RenderDebugText
// ja usava, entao os pontos de chamada nao precisam mudar a logica de cor.
// Se a fonte nao foi carregada (text_init() falhou), nao desenha nada.
//------------------------------------------------------------------------------
void render_text(SDL_Renderer *renderer, float x, float y, const char *text)
{
  if (font == NULL || text == NULL)
  {
    return;
  }

  Uint8 r, g, b, a;
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
  const SDL_Color color = { r, g, b, a };

  SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, 0, color);
  if (textSurface == NULL)
  {
    fprintf(stderr, "Erro ao renderizar o texto '%s': %s\n", text, SDL_GetError());
    return;
  }

  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  if (textTexture == NULL)
  {
    fprintf(stderr, "Erro ao criar a textura do texto '%s': %s\n", text, SDL_GetError());
    SDL_DestroySurface(textSurface);
    return;
  }

  const SDL_FRect destRect = { x, y, (float)textSurface->w, (float)textSurface->h };
  SDL_RenderTexture(renderer, textTexture, NULL, &destRect);

  SDL_DestroyTexture(textTexture);
  SDL_DestroySurface(textSurface);
}

//------------------------------------------------------------------------------
// Mede a largura/altura (em pixels) que 'text' ocupa com a fonte do
// programa, sem desenhar nada. Usado para centralizar texto em botoes.
//------------------------------------------------------------------------------
void text_measure(const char *text, float *outWidth, float *outHeight)
{
  if (outWidth != NULL)
  {
    *outWidth = 0.0f;
  }
  if (outHeight != NULL)
  {
    *outHeight = 0.0f;
  }

  if (font == NULL || text == NULL)
  {
    return;
  }

  int width = 0;
  int height = 0;
  if (!TTF_GetStringSize(font, text, 0, &width, &height))
  {
    fprintf(stderr, "Erro ao medir o texto '%s': %s\n", text, SDL_GetError());
    return;
  }

  if (outWidth != NULL)
  {
    *outWidth = (float)width;
  }
  if (outHeight != NULL)
  {
    *outHeight = (float)height;
  }
}
