# Simulador de Alocação Dinâmica de Memória

Simulador em C, com menu interativo via linha de comando, que reproduz o comportamento de um alocador de memória dinâmica implementando três estratégias clássicas de busca de espaço livre: **First Fit**, **Best Fit** e **Worst Fit**. A memória é simulada como uma região de 1 MiB, organizada em blocos que podem ser divididos (*split*) e reunidos (*coalescência*) conforme alocações e liberações acontecem — permitindo observar, na prática, como e por que a fragmentação de memória surge.

## Conceitos

Em um sistema com alocação dinâmica, o gerenciador de memória mantém o controle de quais regiões estão livres e quais estão ocupadas, respondendo a pedidos de alocação e liberação em qualquer ordem. Para isso, ele organiza a memória livre em **blocos**, e a cada pedido precisa escolher qual bloco livre usar.

Duas formas de desperdício podem ocorrer:

- **Fragmentação interna**: o bloco alocado é maior que o necessário (aqui, quando a sobra de um *split* é pequena demais para valer a pena virar um bloco separado, ela fica "presa" dentro do bloco alocado).
- **Fragmentação externa**: existe memória livre suficiente no total, mas espalhada em pedaços pequenos e não contíguos, nenhum deles grande o bastante para um novo pedido.

As três estratégias implementadas escolhem o bloco livre de formas diferentes, o que muda como a fragmentação externa se acumula ao longo do tempo:

- **First Fit** — aloca no primeiro bloco livre grande o suficiente encontrado ao percorrer a lista. Simples e rápido (não precisa varrer a lista inteira), mas tende a espalhar fragmentos perto do início da memória.
- **Best Fit** — percorre toda a lista e aloca no menor bloco livre que ainda comporta o pedido, minimizando a sobra imediata. Como efeito colateral, tende a deixar muitos fragmentos pequenos ("micro-buracos") que dificilmente serão reaproveitados.
- **Worst Fit** — percorre toda a lista e aloca sempre no maior bloco disponível, na expectativa de que a sobra continue grande e útil. Na prática, tende a destruir os blocos grandes rapidamente, deixando o sistema sem "reserva" para pedidos maiores.

## Estrutura do projeto

```
functions.c   -> struct Bloco e toda a lógica do simulador
                 (criação/inicialização, first_fit/best_fit/worst_fit,
                 split, coalescência, alocar/liberar, exibição, geração
                 de carga aleatória)
main.c        -> ponto de entrada: inicializa a semente aleatória,
                 cria a memória simulada e roda o menu interativo
                 (inclui functions.c diretamente)
```

## Como compilar e rodar

```bash
gcc -O2 -Wall -Wextra -o simulador main.c
./simulador
```

(`functions.c` não precisa ser compilado separadamente — `main.c` o inclui via `#include "functions.c"`.)

## Uso / menu

Ao rodar o programa, o menu principal aparece assim:

```
=============== MENU ===============
Estrategia atual: FIRST FIT

1 - Alocar memoria
2 - Liberar memoria
3 - Mostrar memoria
4 - Alterar estrategia
5 - Gerar 10 alocacoes aleatorias
0 - Sair
```

- **1 — Alocar memória**: pede um tamanho em bytes e tenta alocar usando a estratégia atual.
- **2 — Liberar memória**: pede o ID de uma alocação ativa e a libera, disparando a coalescência com blocos vizinhos livres.
- **3 — Mostrar memória**: imprime o estado atual da memória simulada, bloco a bloco, com faixa de endereços, tamanho e status (`FREE` ou `ID <n>`).
- **4 — Alterar estratégia**: troca entre First Fit, Best Fit e Worst Fit a qualquer momento — inclusive no meio de uma sessão, o que permite comparar como cada uma se sairia a partir do mesmo estado de memória.
- **5 — Gerar 10 alocações aleatórias**: dispara 10 pedidos de alocação com tamanhos aleatórios entre 32 e 8.192 bytes, útil para criar rapidamente uma memória fragmentada e observar o comportamento da estratégia atual sob carga.
- **0 — Sair**: libera toda a lista de blocos e encerra o programa.

## Como funciona por dentro

### A struct `Bloco`

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

A memória simulada é uma lista duplamente encadeada de blocos, na ordem em que aparecem fisicamente no espaço de endereços. Cada bloco guarda seu deslocamento (`inicio`), tamanho, se está livre e, quando ocupado, o `id` da alocação — isso é o que permite `mostrar_memoria()` imprimir faixas de endereço reais como `[200000 - 319999] ID 4`.

### Busca do bloco (as três estratégias)

Cada estratégia é uma função separada que percorre a lista e escolhe um bloco livre segundo seu próprio critério; `buscar_bloco()` apenas direciona para a função certa:

```c
Bloco *first_fit(Bloco *memoria, size_t tamanho){
    Bloco *atual = memoria;
    while (atual != NULL) {
        if (atual->livre && atual->tamanho >= tamanho) {
            return atual;              // primeiro que serve
        }
        atual = atual->proximo;
    }
    return NULL;
}
```

`best_fit()` e `worst_fit()` têm a mesma estrutura de laço, mas em vez de retornar assim que encontram um candidato, continuam percorrendo toda a lista guardando o menor (Best Fit) ou o maior (Worst Fit) bloco livre encontrado até então.

### Split (divisão de bloco)

Quando o bloco escolhido é maior que o pedido, `dividir_bloco()` o corta em dois: um do tamanho exato pedido (que vira ocupado) e outro com a sobra (que continua livre). Se a sobra for menor que `MIN_BLOCO` (32 bytes), ela não vira um bloco novo — fica "presa" dentro do bloco alocado como fragmentação interna, evitando criar blocos livres inutilmente pequenos.

### Coalescência (fusão de blocos livres)

Quando um bloco é liberado, `coalescer()` verifica se o vizinho anterior e o vizinho seguinte, na lista, também estão livres, fundindo-os em um único bloco maior quando for o caso. É essa fusão que evita que a memória fique permanentemente picada em pedaços cada vez menores conforme alocações e liberações se acumulam.

## Exemplo de sessão (fragmentação em ação)

A sequência abaixo foi rodada de verdade com o simulador, usando First Fit, e mostra a fragmentação externa surgindo a partir de operações comuns:

1. Aloca 200.000 bytes → **ID 1**, início 0
2. Aloca 150.000 bytes → **ID 2**, início 200.000
3. Aloca 100.000 bytes → **ID 3**, início 350.000
4. Libera o **ID 2** → o espaço `[200.000–349.999]` (150.000 bytes) volta a ficar livre; como os blocos vizinhos (ID 1 e ID 3) continuam ocupados, não há coalescência possível
5. Aloca 120.000 bytes → cai exatamente no buraco deixado pelo ID 2 (First Fit encontra esse bloco primeiro), virando **ID 4**, e o *split* deixa uma sobra livre de 30.000 bytes

Estado final da memória (opção 3 do menu):

```
============================================================
                    MEMORIA SIMULADA
============================================================
[0 - 199999] ID 1 | 200000 bytes
[200000 - 319999] ID 4 | 120000 bytes
[320000 - 349999] FREE | 30000 bytes
[350000 - 449999] ID 3 | 100000 bytes
[450000 - 1048575] FREE | 598576 bytes
============================================================
```

Esse resultado ilustra bem o problema central do trabalho: mesmo havendo **628.576 bytes livres no total** (30.000 + 598.576), um pedido de, digamos, 500.000 bytes seria atendido tranquilamente pelo bloco do final — mas um cenário com mais alocações e liberações intercaladas facilmente deixaria vários buracos pequenos como o de 30.000 bytes, nenhum deles grande o bastante para pedidos maiores, mesmo que a soma total ainda fosse suficiente. É exatamente esse fenômeno — fragmentação externa — que muda de comportamento conforme a estratégia de busca escolhida (First, Best ou Worst Fit).

## Limitações e possíveis melhorias

- Busca de bloco é O(n) nas três estratégias (lista simples, sem índice por tamanho); para uma memória com muitos blocos, uma árvore balanceada ou *free lists* segregadas por faixa de tamanho seriam mais eficientes.
- `scanf` não verifica o retorno — uma entrada não numérica no menu trava a leitura em vez de pedir novamente.
- Sem suporte a *realloc* (crescer/encolher uma alocação existente).
