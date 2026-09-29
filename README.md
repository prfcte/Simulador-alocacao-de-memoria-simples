# Simulador de Alocação Dinâmica de Memória

Simulador desenvolvido em C que reproduz a alocação dinâmica de memória utilizando três estratégias clássicas: **First Fit**, **Best Fit** e **Worst Fit**.

A memória é simulada como uma região de **1 MiB**, organizada em blocos por meio de uma lista duplamente encadeada. O programa permite realizar alocações, liberações, divisão e fusão de blocos, além de gerar automaticamente solicitações aleatórias.

## Conceitos

O simulador representa uma memória dividida em blocos livres e ocupados. A cada solicitação, uma estratégia é utilizada para selecionar um bloco livre adequado.

* **First Fit:** seleciona o primeiro bloco livre que comporte a solicitação.
* **Best Fit:** seleciona o menor bloco livre que comporte a solicitação.
* **Worst Fit:** seleciona o maior bloco livre que comporte a solicitação.

Durante a utilização da memória podem ocorrer dois tipos de fragmentação:

* **Fragmentação interna:** ocorre quando existe espaço não utilizado dentro de um bloco alocado. No simulador, isso acontece quando a sobra de uma alocação é menor que `MIN_BLOCO` (32 bytes).
* **Fragmentação externa:** ocorre quando a memória livre está dividida em vários blocos separados, dificultando uma nova alocação mesmo que exista espaço livre suficiente no total.

## Estrutura do projeto

```text
functions.c  → Estrutura dos blocos e lógica do simulador
                - criação e inicialização
                - First Fit, Best Fit e Worst Fit
                - divisão de blocos (split)
                - coalescência
                - alocação e liberação
                - exibição da memória
                - geração de alocações aleatórias

menu.c       → Interface e fluxo de interação
                - inicialização da memória
                - menu principal
                - entrada de dados
                - seleção da estratégia
                - chamadas das operações

main.c       → Ponto de entrada do programa
                - chama a função menu()
```

> **Observação:** atualmente o projeto utiliza `#include "functions.c"` em `menu.c` e `#include "menu.c"` em `main.c`. Dessa forma, os arquivos são incluídos durante a compilação a partir do `main.c`.

## Como compilar e executar

No Linux, utilize:

```bash
gcc -O2 -Wall -Wextra -o simulador main.c
```

Depois execute:

```bash
./simulador
```

## Menu

Ao iniciar o programa, o menu apresenta:

```text
=============== MENU ===============
Estrategia atual: FIRST FIT

1 - Alocar memoria
2 - Liberar memoria
3 - Mostrar memoria
4 - Alterar estrategia
5 - Gerar 10 alocacoes aleatorias
0 - Sair
====================================
—> Opcao:
```

### 1 — Alocar memória

Solicita ao usuário o tamanho desejado em bytes e tenta realizar a alocação utilizando a estratégia atualmente selecionada.

### 2 — Liberar memória

Solicita o ID de uma alocação existente e libera o bloco correspondente. Após a liberação, o programa verifica se é possível realizar a coalescência com blocos livres adjacentes.

### 3 — Mostrar memória

Exibe todos os blocos existentes, mostrando:

* intervalo de endereços;
* estado (`LIVRE` ou `ID X`);
* tamanho do bloco.

### 4 — Alterar estratégia

Permite alternar entre:

1. First Fit;
2. Best Fit;
3. Worst Fit.

A alteração pode ser realizada durante a execução sem reiniciar a memória simulada.

### 5 — Gerar 10 alocações aleatórias

Realiza **10 solicitações de alocação** com tamanhos aleatórios entre **32 bytes e 8 KiB (8192 bytes)**, utilizando a estratégia atualmente selecionada.

Essa opção permite gerar rapidamente diferentes configurações de memória e observar a divisão dos blocos e a formação de espaços livres.

### 0 — Sair

Encerra o programa e libera os nós da lista utilizada para representar a memória.

## Como funciona

### Estrutura `Bloco`

A memória é representada por uma lista duplamente encadeada:

```c
typedef struct Bloco {
    size_t inicio;
    size_t tamanho;
    int livre;
    int id;

    struct Bloco *anterior;
    struct Bloco *proximo;
} Bloco;
```

Cada bloco possui:

* `inicio`: endereço inicial dentro da memória simulada;
* `tamanho`: tamanho do bloco em bytes;
* `livre`: indica se o bloco está livre;
* `id`: identifica uma alocação ocupada;
* `anterior`: ponteiro para o bloco anterior;
* `proximo`: ponteiro para o próximo bloco.

Os blocos são mantidos na ordem em que aparecem na memória.

### Busca dos blocos

As três estratégias possuem funções próprias:

```text
First Fit → primeiro bloco adequado
Best Fit  → menor bloco adequado
Worst Fit → maior bloco adequado
```

A função `buscar_bloco()` seleciona qual estratégia será utilizada.

### Split

Quando um bloco livre é maior que a solicitação, ele pode ser dividido:

```text
Antes:

[          LIVRE          ]

Depois:

[      OCUPADO      ][   LIVRE   ]
```

Se a sobra for maior ou igual a `MIN_BLOCO` (32 bytes), um novo bloco livre é criado.

Caso a sobra seja menor que 32 bytes, ela permanece dentro do bloco alocado, caracterizando fragmentação interna.

### Coalescência

Quando um bloco é liberado, o simulador verifica seus vizinhos.

Por exemplo:

```text
Antes:

[ LIVRE ][ LIBERADO ][ LIVRE ]

Depois:

[          LIVRE          ]
```

Os blocos livres adjacentes são unidos em um único bloco maior, reduzindo a fragmentação externa.

## Geração aleatória

A opção de geração aleatória utiliza `rand()` para produzir dez tamanhos entre 32 e 8192 bytes.

A semente do gerador é inicializada no início da execução:

```c
srand((unsigned int) time(NULL));
```

Cada valor gerado é enviado para a função de alocação utilizando a estratégia atualmente selecionada.

Exemplo:

```text
Alocacao 1: 452 bytes
Alocacao 2: 7190 bytes
Alocacao 3: 1832 bytes
...
Alocacao 10: 367 bytes
```

## Exemplo de funcionamento

Considere uma memória inicialmente livre:

```text
[---------------- 1 MiB ----------------]
```

Após algumas alocações, ela pode assumir uma configuração como:

```text
[ ID 1 ][ LIVRE ][ ID 2 ][ LIVRE ][ ID 3 ][ LIVRE ]
```

Se uma região ocupada for liberada:

```text
[ ID 1 ][ LIVRE ][ ID 2 ][ LIVRE ][ ID 3 ][ LIVRE ]
          ↑
       liberado
```

Caso blocos livres adjacentes sejam encontrados, a coalescência os transforma em uma região maior.

A opção **Mostrar memória** permite acompanhar essas alterações diretamente no terminal.

## Complexidade

Na implementação atual, baseada em lista duplamente encadeada:

| Estratégia | Busca             |
| ---------- | ----------------- |
| First Fit  | O(n) no pior caso |
| Best Fit   | O(n)              |
| Worst Fit  | O(n)              |

O First Fit pode encontrar um bloco adequado antes de percorrer toda a lista. Best Fit e Worst Fit precisam percorrer a lista para identificar, respectivamente, o menor ou o maior bloco adequado.

A divisão e a coalescência podem ser realizadas em **O(1)** após o bloco envolvido já ter sido localizado, pois dependem principalmente da atualização dos ponteiros da lista.


## Limitações e possíveis melhorias

- Busca de bloco é O(n) nas três estratégias (lista simples, sem índice por tamanho); para uma memória com muitos blocos, uma árvore balanceada ou *LIVRE lists* segregadas por faixa de tamanho seriam mais eficientes.
- `scanf` não verifica o retorno — uma entrada não numérica no menu trava a leitura em vez de pedir novamente.
- Sem suporte a *realloc* (crescer/encolher uma alocação existente).