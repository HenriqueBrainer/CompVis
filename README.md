# CompVis

Projeto 1 — Processamento de Imagens da disciplina de **Computação Visual** da Faculdade de Computação e Informática da Universidade Presbiteriana Mackenzie.

O projeto consiste no desenvolvimento de um software em linguagem C para carregamento, processamento e análise de imagens utilizando SDL3, SDL3_image e SDL3_ttf. A aplicação recebe o caminho de uma imagem pela linha de comando, converte imagens coloridas para escala de cinza, calcula e exibe o histograma e disponibiliza operações de processamento por meio de uma interface com duas janelas.

## Objetivos e funcionamento

O programa carrega a imagem informada pelo usuário usando SDL_image. Após o carregamento, verifica se a imagem já está em escala de cinza ou se é colorida. Quando necessário, realiza a conversão utilizando a fórmula definida no enunciado:

```text
Y = 0.2125 × R + 0.7154 × G + 0.0721 × B
```

A imagem em escala de cinza é usada como base para as operações seguintes. O programa calcula um histograma com 256 níveis de intensidade, de 0 a 255, e utiliza esses dados para calcular a média de intensidade e o desvio padrão.

A média classifica a imagem como **escura**, **média** ou **clara**. O desvio padrão classifica o contraste como **baixo**, **médio** ou **alto**. Essas informações são exibidas na janela secundária junto com o histograma.

A interface possui uma janela principal para exibição da imagem e uma janela secundária para o histograma, as informações da análise e os controles da aplicação.

## Estrutura do projeto

```text
├── .gitignore
├── README.md
├── Makefile
├── Makefile.local
│
├── SDL3.dll
├── SDL3_image.dll
├── SDL3_ttf.dll
│
├── assets/
│   ├── fonts/
│   │   ├── DejaVuSans.ttf
│   │   └── DejaVuSans-LICENSE.txt
│   │
│   └── imgs/
│       ├── morango.avif
│       ├── morango.jpg
│       └── morango.png
│
├── docs/
│   └── Projeto 1_ Etapa 2 - Análise final e implementação.docx
│
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

## Responsabilidade dos módulos

| Arquivo | Responsabilidade |
|---|---|
| `src/main.c` | Inicializa a aplicação, valida os argumentos, coordena o processamento inicial e chama os demais módulos. |
| `src/load_image.c` | Carrega imagens com SDL_image e trata erros de caminho, arquivo inexistente, arquivo corrompido ou formato não suportado. |
| `src/grayscale.c` | Verifica se uma imagem está em escala de cinza e realiza a conversão de imagens coloridas. |
| `src/gui.c` | Gerencia as duas janelas, os renderizadores, o ciclo de eventos e os controles da interface. |
| `src/histogram.c` | Calcula o histograma, a média, o desvio padrão, as classificações de luminosidade e contraste e a renderização do histograma. |
| `src/equalization.c` | Implementa a equalização do histograma e a restauração da imagem original em escala de cinza. |
| `src/display_image.c` | Controla a exibição na resolução original ou em 1024×768 e o posicionamento da janela principal. |
| `src/save_image.c` | Salva a imagem atualmente exibida em `output_image.png`. |
| `src/text.c` | Inicializa a SDL3_ttf, carrega a fonte e renderiza os textos da interface. |
| `assets/imgs/` | Contém as imagens utilizadas nos testes. |
| `assets/fonts/` | Contém a fonte usada pela aplicação e o arquivo de licença correspondente. |
| `docs/` | Contém a documentação da análise final e implementação. |
| `Makefile` | Compila automaticamente todos os arquivos `.c` da pasta `src` e vincula as bibliotecas SDL. |
| `Makefile.local` | Informa os caminhos das bibliotecas instaladas localmente em cada máquina. |

## Bibliotecas e versões

O projeto utiliza as seguintes versões:

| Biblioteca | Versão |
|---|---:|
| SDL3 | 3.4.0 |
| SDL3_image | 3.4.0 |
| SDL3_ttf | 3.2.2 |
| GCC | C99 ou mais recente |

A SDL3 é utilizada para janelas, renderização, superfícies, texturas e eventos. A SDL3_image é utilizada para carregar imagens. A SDL3_ttf é utilizada para carregar a fonte e exibir textos na interface.

## Fonte utilizada

Os textos da aplicação utilizam a família **DejaVu Sans**, armazenada no próprio projeto em:

```text
assets/fonts/DejaVuSans.ttf
```

A fonte é carregada pelo programa por meio da SDL3_ttf. Como o arquivo está dentro do repositório, a aplicação não depende de uma fonte previamente instalada no sistema operacional. A licença da fonte está disponível em:

```text
assets/fonts/DejaVuSans-LICENSE.txt
```

## Configuração do Makefile.local

O `Makefile.local` deve conter os caminhos das bibliotecas SDL instaladas na máquina. Como esses caminhos podem ser diferentes entre os integrantes do grupo, o arquivo é mantido separado do `Makefile` principal e não deve ser substituído por uma configuração fixa compartilhada.

Exemplo:

```make
SDL3_DIR       := C:/Users/SEU_USUARIO/Desktop/Projeto/libs/SDL3
SDL3_IMAGE_DIR := C:/Users/SEU_USUARIO/Desktop/Projeto/libs/SDL3_image
SDL3_TTF_DIR   := C:/Users/SEU_USUARIO/Desktop/Projeto/libs/SDL3_ttf
```

O `Makefile` também possui `C:/msys64/ucrt64` como caminho padrão. Se as bibliotecas estiverem instaladas nesse diretório, o `Makefile.local` pode ser omitido ou ajustado conforme necessário.

## Compilação

A compilação deve ser realizada no terminal **MSYS2 UCRT64**, com GCC e Make disponíveis no `PATH`.

Para compilar:

```bash
make
```

Para remover o executável e os arquivos objeto:

```bash
make clean
```

O `Makefile` utiliza:

```make
SRC := $(wildcard src/*.c)
```

Assim, todos os arquivos `.c` presentes em `src` são compilados automaticamente, sem a necessidade de adicionar cada módulo manualmente às regras de build.

## Execução

O caminho correto para a imagem de teste incluída no projeto é:

```text
assets/imgs/morango.png
```

Para compilar e executar com essa imagem:

```bash
make run IMG=assets/imgs/morango.png
```

Também é possível executar diretamente:

```bash
./main.exe assets/imgs/morango.png
```

A forma geral de execução é:

```bash
./main.exe caminho_da_imagem.ext
```

## DLLs no Windows

Para executar o programa no Windows, mantenha as seguintes DLLs na mesma pasta do `main.exe` ou em um diretório incluído na variável de ambiente `PATH`:

```text
SDL3.dll
SDL3_image.dll
SDL3_ttf.dll
```

## Controles

A janela secundária possui os controles previstos no projeto:

- **Equalizar / Ver original:** alterna entre a imagem equalizada e a imagem original em escala de cinza;
- **Resolução original / 1024x768:** alterna a resolução de exibição da imagem e atualiza o tamanho da janela principal;
- **Tecla S:** salva a imagem atualmente exibida em `output_image.png`, sobrescrevendo o arquivo caso ele já exista.

## Processo de desenvolvimento

O desenvolvimento utilizou os vídeos das aulas do professor e o código-fonte apresentado na disciplina como material de estudo e referência. O código-base também foi consultado no repositório:

[CompVis262 — repositório-base da disciplina](https://github.com/profkishimoto/CompVis262)

Inicialmente, os requisitos estavam concentrados em um único arquivo `main.c`. Essa organização dificultava a compreensão do projeto, pois funções de carregamento, processamento, histograma, interface e salvamento estavam misturadas no mesmo arquivo. Por isso, o código foi refatorado e organizado em módulos com responsabilidades específicas.

Durante o desenvolvimento, foi utilizada a tecnologia de inteligência artificial **Claude** para auxiliar na detecção de bugs, identificação de erros, revisão da organização e melhoria do código. As sugestões foram analisadas pelo grupo e utilizadas como apoio ao desenvolvimento, sem substituir a compreensão dos requisitos e da implementação.

## Dificuldades encontradas

Uma das principais dificuldades ocorreu ao utilizar somente os arquivos `.json` de configuração do Visual Studio Code para compilar o projeto em máquinas diferentes. Esses arquivos continham caminhos específicos da máquina em que foram configurados. Como consequência, a compilação podia funcionar em um computador e falhar em outro quando as bibliotecas SDL estavam instaladas em diretórios diferentes.

Para resolver esse problema, foi criado o `Makefile`, que padroniza as regras de compilação, e o `Makefile.local`, que permite a cada integrante informar os caminhos locais das bibliotecas SDL3, SDL3_image e SDL3_ttf.

Outra dificuldade foi compreender e manter um código que inicialmente concentrava todos os requisitos em `main.c`. A refatoração foi necessária para tornar o projeto mais legível, facilitar a identificação de cada funcionalidade do PDF e permitir a manutenção independente de cada módulo.

## Funcionalidades contempladas

A versão atual do projeto contempla as seguintes funcionalidades:

- carregamento de imagem com SDL_image;
- tratamento de erros de carregamento;
- identificação de imagens coloridas e em escala de cinza;
- conversão para escala de cinza usando a fórmula do enunciado;
- cálculo e exibição do histograma;
- cálculo da média de intensidade;
- cálculo do desvio padrão;
- classificação da luminosidade e do contraste;
- interface com janela principal e janela secundária;
- equalização e restauração do histograma;
- alternância entre resolução original e 1024×768;
- botões desenhados e atualizados conforme o estado da interface;
- salvamento da imagem exibida em `output_image.png` usando a tecla `S`;
- carregamento de fonte própria com SDL3_ttf;
- exibição de textos informativos na interface.

## Documentação adicional

O relatório da análise final e implementação está disponível em:

```text
docs/Projeto 1_ Etapa 2 - Análise final e implementação.docx
```

## Referências

[1]: https://github.com/profkishimoto/CompVis262 "Repositório-base CompVis262"
[2]: https://wiki.libsdl.org/SDL3/FrontPage "Documentação oficial da SDL3"
[3]: https://wiki.libsdl.org/SDL3_image/FrontPage "Documentação oficial da SDL3_image"
[4]: https://wiki.libsdl.org/SDL3_ttf/FrontPage "Documentação oficial da SDL3_ttf"
