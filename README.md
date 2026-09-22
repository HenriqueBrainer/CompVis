# CompVis

Projeto 1 — Computação Visual (Mackenzie). O código foi reorganizado em módulos, preservando a implementação existente.

## Estrutura

```text
├── .gitignore
├── README.md
├── Makefile
├── Makefile.local
├── assets/
└── src/
    ├── main.c
    ├── load_image.c
    ├── grayscale.c
    ├── gui.c
    ├── histogram.c
    ├── equalization.c
    ├── display_image.c
    ├── save_image.c
    └── text.c
```

## Bibliotecas

- SDL3
- SDL3_image
- Compilador GCC com suporte a C99 ou mais recente

## Configuração

O `Makefile` usa `C:/msys64/ucrt64` como padrão para SDL3 e SDL3_image. Para outra instalação, ajuste `SDL3_DIR` e `SDL3_IMAGE_DIR` no `Makefile.local`. Esse arquivo é ignorado pelo Git para permitir configurações diferentes em cada máquina.

## Compilação e execução

No terminal do MSYS2/UCRT64, execute:

```bash
make
make run IMG=assets/morango.png
make clean
```

Também é possível executar diretamente:

```bash
./main.exe caminho_da_imagem.ext
```

No Windows, mantenha `SDL3.dll` e `SDL3_image.dll` na mesma pasta do executável ou em um diretório presente no `PATH`.

## Módulos

`main.c` coordena a aplicação; `load_image.c` carrega imagens com SDL_image; `grayscale.c` verifica e converte imagens para escala de cinza; `gui.c` contém a lógica atual das duas janelas; e `histogram.c` calcula, analisa e renderiza o histograma.

Os arquivos `equalization.c`, `display_image.c`, `save_image.c` e `text.c` foram criados vazios como estrutura para as funcionalidades correspondentes do enunciado. Eles já são incluídos automaticamente na compilação pelo `Makefile`, mas ainda não alteram o comportamento atual do programa.
