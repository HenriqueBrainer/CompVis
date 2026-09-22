# CompVis
Projeto 1 — Computação Visual (Mackenzie)

## Bibliotecas e versões
- SDL3: 3.40
- SDL3_image: 3.40

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

## Sobre a pasta `1-Testes`
Testando a funcionalidade 1:

morango.jpg funcionou
morango.png funcionou
morango.avif erro (imagem não suportada) 

Analisar o porque essa extensão pode de dado erro
