# CompVis
Projeto 1 — Computação Visual (Mackenzie)

## Bibliotecas e versões
- SDL3: 3.4.0
- SDL3_image: 3.4.6

## Compilação

### Opção recomendada: Makefile
O projeto tem um `Makefile` (Windows / MSYS2-MinGW).

Como cada integrante do grupo pode ter o SDL3/SDL3_image em uma pasta
diferente, **não edite os caminhos direto no `Makefile`**. Em vez disso, crie
um arquivo `Makefile.local` (não é versionado, já está no `.gitignore`) ao
lado do `Makefile` com:

```make
SDL3_DIR       := C:/Users/SEU_USUARIO/../SDL3
SDL3_IMAGE_DIR := C:/Users/SEU_USUARIO/../SDL3_image
```

Depois é só rodar (no terminal do MSYS2/UCRT64, ou qualquer terminal com o
`gcc`/`make` do MSYS2 no PATH):

```
make            # compila e gera main.exe
make run        # compila e roda com 1-Testes/morango.png
make run IMG=caminho/da/imagem.png   # compila e roda com outro arquivo
make clean      # remove o executável gerado
```

### Opção alternativa: VS Code (tasks.json)
Também é possível usar a task `C/C++: gcc.exe build active file` do VS Code
(`.vscode/tasks.json`), que roda o mesmo comando `gcc` com os caminhos fixos
configurados nela. Ajuste os caminhos `-I`/`-L` no `tasks.json` conforme a
pasta onde o SDL3/SDL3_image estão na sua máquina.

Em qualquer uma das opções, copie `SDL3.dll` e `SDL3_image.dll` para a mesma
pasta do `main.exe` antes de executar (Windows).

## Execução
O programa exige o caminho da imagem como argumento de linha de comando:

```
main.exe caminho_da_imagem.ext
```

## Histograma e análise da imagem

Depois de converter a imagem para escala de cinza, o programa calcula as 256
frequências de intensidade (0 a 255) e exibe o histograma normalizado na janela
secundária. A normalização usa a maior frequência como referência, para que o
gráfico permaneça legível independentemente da resolução da imagem.

A mesma janela mostra a média de intensidade e o desvio padrão populacional:

- média menor que 85: imagem escura;
- média de 85 a 170: imagem média;
- média maior que 170: imagem clara;
- desvio padrão menor que 42,5: contraste baixo;
- desvio padrão de 42,5 até abaixo de 85: contraste médio;
- desvio padrão a partir de 85: contraste alto.

Os limites de luminosidade dividem a faixa de intensidade de 8 bits em três
partes. Os limites de contraste dividem em três partes a faixa teórica do
desvio padrão, que vai de 0 a 127,5. As informações textuais são desenhadas
com a fonte de depuração 8x8 incorporada à SDL3, sem depender de fontes
instaladas no sistema operacional.

## Sobre a pasta `1-Testes`
Testando a funcionalidade 1:

morango.jpg funcionou
morango.png funcionou
morango.avif erro (imagem não suportada) 

Analisar o porque essa extensão pode de dado erro
