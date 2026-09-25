
# CompVis

Projeto 1 — Processamento de Imagens da disciplina de **Computação Visual** da Faculdade de Computação e Informática da Universidade Presbiteriana Mackenzie.

O projeto consiste no desenvolvimento de um software em linguagem C para carregamento, processamento e análise de imagens utilizando SDL3, SDL3_image e SDL3_ttf. A aplicação recebe o caminho de uma imagem pela linha de comando, converte imagens coloridas para escala de cinza, calcula e exibe o histograma e disponibiliza operações de processamento por meio de uma interface com duas janelas.



## Objetivos e funcionamento

**1. Carregamento.** O programa recebe o caminho da imagem como argumento de linha de comando e a carrega com SDL_image. Erros de arquivo inexistente, inacessível ou em formato inválido/não suportado são tratados e reportados no terminal, sem derrubar o programa.

**2. Análise e conversão para escala de cinza.** Após o carregamento, o programa verifica se a imagem já está em escala de cinza ou é colorida — essa checagem é feita **pixel a pixel** (comparando os canais R, G e B), e não pelo formato/metadado do arquivo, já que o SDL_image pode decodificar um PNG em escala de cinza expandindo para um formato com múltiplos canais. O resultado é exibido no terminal. Quando a imagem é colorida, ela é convertida usando a fórmula do enunciado:

```text
Y = 0.2125 × R + 0.7154 × G + 0.0721 × B
```

A imagem em escala de cinza resultante é usada como base para todas as operações seguintes.

**3. Interface com duas janelas.** A janela principal exibe a imagem sendo processada, começa em 1024×768 pixels e é centralizada no monitor primário. A janela secundária é filha da principal, tem tamanho fixo (400×620), fica posicionada no canto `(0, 0)` da tela e concentra o histograma, as informações da análise e os dois botões de controle. Fechar qualquer uma das duas janelas encerra o programa, já que elas funcionam em conjunto.

**4. Histograma.** O programa calcula um histograma com 256 níveis de intensidade (0–255) e o exibe na janela secundária. A partir dele calcula a média de intensidade — classificando a imagem como **escura**, **média** ou **clara** — e o desvio padrão — classificando o contraste como **baixo**, **médio** ou **alto**.

**5. Equalização do histograma.** Um botão abaixo do histograma, desenhado com primitivas da SDL, equaliza o histograma pela função de distribuição acumulada (CDF) ao ser clicado, atualizando a imagem exibida e o histograma. A versão equalizada é calculada uma única vez e reaproveitada nos cliques seguintes. Um novo clique reverte para a imagem original em escala de cinza, sem recarregar o arquivo. O texto do botão alterna entre `"Equalizar"` e `"Ver original"`, e sua cor reflete o estado (azul neutro, azul claro no hover, azul escuro pressionado).

**6. Exibição da imagem.** Um segundo botão, abaixo do de equalizar, alterna entre exibir a imagem em sua resolução original e exibi-la esticada para 1024×768. A janela principal é redimensionada de acordo, permanecendo centralizada no monitor primário — exceto se o novo tamanho exceder a resolução do sistema, caso em que seu canto superior esquerdo é posicionado em `(0, 0)`. O texto do botão alterna entre `"Resolução original"` e `"1024x768"`, com os mesmos três estados visuais do botão de equalização.

**7. Salvar imagem.** Ao pressionar a tecla `S`, o programa salva exatamente o que está sendo exibido no momento na janela principal (imagem original ou equalizada, na resolução ativa) em `output_image.png`, sobrescrevendo o arquivo se ele já existir. O terminal informa se o arquivo foi criado ou sobrescrito.

**8. Textos.** Todos os textos da interface (informações do histograma e rótulos dos botões) são renderizados com uma fonte própria via SDL3_ttf, carregada de um arquivo incluído no repositório — ver [Fonte utilizada](#fonte-utilizada).

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
| `src/grayscale.c` | Verifica, pixel a pixel, se uma imagem está em escala de cinza e realiza a conversão de imagens coloridas usando a fórmula do enunciado. |
| `src/gui.c` | Gerencia as duas janelas, os renderizadores, o ciclo de eventos, os botões (estados e cliques) e a tecla de salvar. |
| `src/histogram.c` | Calcula o histograma, a média, o desvio padrão, as classificações de luminosidade e contraste e a renderização do histograma. |
| `src/equalization.c` | Implementa a equalização do histograma (função de distribuição acumulada) e a restauração da imagem original em escala de cinza. |
| `src/display_image.c` | Controla a exibição na resolução original ou em 1024×768 e o redimensionamento/posicionamento da janela principal. |
| `src/save_image.c` | Salva a imagem atualmente exibida em `output_image.png`, detectando se o arquivo já existia para informar a mensagem correta. |
| `src/text.c` | Inicializa a SDL3_ttf, carrega a fonte e renderiza os textos da interface (histograma e botões). |
| `assets/imgs/` | Contém as imagens utilizadas nos testes. |
| `assets/fonts/` | Contém a fonte usada pela aplicação e o arquivo de licença correspondente. |
| `docs/` | Contém a documentação da análise final e implementação. |
| `Makefile` | Compila automaticamente todos os arquivos `.c` da pasta `src` e vincula as bibliotecas SDL. |
| `Makefile.local` | Informa os caminhos das bibliotecas instaladas localmente em cada máquina. |

## Contribuição dos integrantes

| Parte inicial do desenvolvimento foi realizado pelo grupo em conjunto em chamada;

| Bruna Gonçalves Corte David |Testes, elaboração do relatório final e implementação do salvamento de imagem (Item 7)
| Henrique Brainer Costa | Configuração do ambiente, modularização do código, implementação e teste das funcionalidades principais (carregamento de imagens, conversão em escala de cinza, GUI, equalizador e exibição da imagem), ajustes no Makefile e documentação inicial do README
| João Pedro Queiroz de Andrade|Implementação da funcionalidade de exibição e cálculo de histograma (Parte 4)
| Júlia Andrade|Revisão geral, testes, e elaboração do relatório


## Bibliotecas e versões

| Biblioteca | Versão | Download |
|---|---:|---|
| Linguagem C | C23 | Padrão utilizado no desenvolvimento |
| GCC | 16.1.0 | Compilador utilizado |
| SDL3 | 3.4.0 | [github.com/libsdl-org/SDL/releases](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.0) |
| SDL3_image | 3.4.0 | [github.com/libsdl-org/SDL_image/releases](https://github.com/libsdl-org/SDL_image/releases/tag/release-3.4.0) |
| SDL3_ttf | 3.2.2 | [github.com/libsdl-org/SDL_ttf/releases](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-3.2.2) |

O projeto foi desenvolvido, compilado e testado nos sistemas operacionais Windows 10 e Windows 11
A SDL3 é utilizada para janelas, renderização, superfícies, texturas e eventos. A SDL3_image é utilizada para carregar imagens. A SDL3_ttf é utilizada para carregar a fonte e exibir textos na interface.

## Fonte utilizada

Os textos da aplicação utilizam a família **DejaVu Sans**, armazenada no próprio projeto em:

```text
assets/fonts/DejaVuSans.ttf
```

A fonte é carregada pelo programa por meio da SDL3_ttf. Como o arquivo está dentro do repositório, a aplicação não depende de uma fonte previamente instalada no sistema operacional — o mesmo `.ttf` é usado independentemente de o programa rodar no Windows ou no Linux/WSL. Caso a fonte não seja carregada por algum motivo, o programa não trava: continua funcionando normalmente (histograma e botões incluídos), apenas sem desenhar texto, e o motivo do erro é impresso no terminal.

A licença da fonte está disponível em:

```text
assets/fonts/DejaVuSans-LICENSE.txt
```

## Configuração do Makefile.local

O `Makefile.local` deve conter os caminhos das bibliotecas SDL instaladas na máquina. Como esses caminhos podem ser diferentes entre os integrantes do grupo, o arquivo é mantido separado do `Makefile` principal (e listado no `.gitignore`), para que cada pessoa use sua própria configuração sem sobrescrever a dos colegas.

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

- **Equalizar / Ver original:** alterna entre a imagem equalizada e a imagem original em escala de cinza. A versão equalizada é calculada uma vez e reaproveitada nos cliques seguintes.
- **Resolução original / 1024x768:** alterna a resolução de exibição da imagem, redimensionando e reposicionando a janela principal (centralizada no monitor primário, ou no canto `(0, 0)` se o tamanho exceder a resolução do sistema).
- **Tecla S:** salva a imagem atualmente exibida em `output_image.png`, sobrescrevendo o arquivo caso ele já exista.

Os dois botões refletem o estado da interação do usuário: azul no estado neutro, azul claro quando o mouse está sobre o botão e azul escuro quando pressionado.

## Processo de desenvolvimento

O desenvolvimento utilizou os vídeos das aulas do professor e o código-fonte apresentado na disciplina como material de estudo e referência. O código-base também foi consultado no repositório:

[CompVis262 — repositório-base da disciplina](https://github.com/profkishimoto/CompVis262)

Inicialmente, os requisitos estavam concentrados em um único arquivo `main.c`. Essa organização dificultava a compreensão do projeto, pois funções de carregamento, processamento, histograma, interface e salvamento estavam misturadas no mesmo arquivo. Por isso, o código foi refatorado e organizado em módulos com responsabilidades específicas, cada um correspondendo a um ou mais itens do enunciado do projeto.

Durante o desenvolvimento, foi utilizada a tecnologia de inteligência artificial **Claude** para auxiliar na detecção de bugs, identificação de erros, revisão da organização e melhoria do código. As sugestões foram analisadas pelo grupo e utilizadas como apoio ao desenvolvimento, sem substituir a compreensão dos requisitos e da implementação.

## Dificuldades encontradas

Uma das principais dificuldades ocorreu ao utilizar somente os arquivos `.json` de configuração do Visual Studio Code para compilar o projeto em máquinas diferentes. Esses arquivos continham caminhos específicos da máquina em que foram configurados. Como consequência, a compilação podia funcionar em um computador e falhar em outro quando as bibliotecas SDL estavam instaladas em diretórios diferentes.

Para resolver esse problema, foi criado o `Makefile`, que padroniza as regras de compilação, e o `Makefile.local`, que permite a cada integrante informar os caminhos locais das bibliotecas SDL3, SDL3_image e SDL3_ttf sem afetar a configuração dos colegas.

Outra dificuldade foi compreender e manter um código que inicialmente concentrava todos os requisitos em `main.c`. A refatoração foi necessária para tornar o projeto mais legível, facilitar a identificação de cada funcionalidade do PDF e permitir a manutenção independente de cada módulo.

Também foi necessário atenção a diferenças entre versões da SDL3: em algumas versões mais recentes, constantes como a tecla `S` do teclado (`SDLK_s`/`SDLK_S`) tiveram seu nome alterado, exigindo ajustes pontuais no código conforme a versão instalada em cada máquina.

## Funcionalidades contempladas

A versão atual do projeto contempla as seguintes funcionalidades:

- carregamento de imagem com SDL_image;
- tratamento de erros de carregamento (arquivo inexistente, formato inválido);
- identificação de imagens coloridas e em escala de cinza, pixel a pixel;
- conversão para escala de cinza usando a fórmula do enunciado;
- interface com janela principal e janela secundária (filha, tamanho fixo, posição `(0,0)`);
- cálculo e exibição do histograma;
- cálculo da média de intensidade e classificação da luminosidade;
- cálculo do desvio padrão e classificação do contraste;
- equalização e restauração do histograma, com botão de dois estados de texto;
- alternância entre resolução original e 1024×768, com reposicionamento da janela principal;
- botões desenhados com primitivas da SDL e atualizados conforme o estado da interação (neutro/hover/pressionado);
- salvamento da imagem exibida em `output_image.png` usando a tecla `S`, com detecção de sobrescrita;
- carregamento de fonte própria com SDL3_ttf, independente do sistema operacional;
- exibição de textos informativos na interface (histograma e botões).

## Documentação adicional

O relatório da análise final e implementação está disponível em:

[Relatório da Etapa 2](<docs/Projeto 1_ Etapa 2 - Análise final e implementação.pdf>)

## Referências

- [Repositório-base CompVis262](https://github.com/profkishimoto/CompVis262)
- [SDL3 3.4.0](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.0)
- [SDL3_image 3.4.0](https://github.com/libsdl-org/SDL_image/releases/tag/release-3.4.0)
- [SDL3_ttf 3.2.2](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-3.2.2)
- [Documentação oficial da SDL3](https://wiki.libsdl.org/SDL3/FrontPage)
- [Documentação oficial da SDL3_image](https://wiki.libsdl.org/SDL3_image/FrontPage)
- [Documentação oficial da SDL3_ttf](https://wiki.libsdl.org/SDL3_ttf/FrontPage)
