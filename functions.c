#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define MEMORIA_TOTAL (1024 * 1024)
#define MIN_BLOCO 32
#define FIRST_FIT 1
#define BEST_FIT 2
#define WORST_FIT 3

typedef struct Bloco {
    size_t inicio;
    size_t tamanho;
    int livre;
    int id;

    struct Bloco *anterior;
    struct Bloco *proximo;
} Bloco;


// ---------- CRIAÇÃO E INICIALIZAÇÃO ---------- 

Bloco *criar_bloco(size_t inicio, size_t tamanho, int livre, int id){
    Bloco *novo = malloc(sizeof(Bloco));

    if (novo == NULL) {
        printf("Erro ao alocar memoria para um bloco.\n");
        exit(EXIT_FAILURE);
    }

    novo->inicio = inicio;
    novo->tamanho = tamanho;
    novo->livre = livre;
    novo->id = id;
    novo->anterior = NULL;
    novo->proximo = NULL;

    return novo;
}

Bloco *inicializar_memoria(){
    return criar_bloco(0, MEMORIA_TOTAL, 1, -1);
}

// ---------- FIRST FIT ---------- 

Bloco *first_fit(Bloco *memoria, size_t tamanho){
    Bloco *atual = memoria;

    while (atual != NULL) {
        if (atual->livre && atual->tamanho >= tamanho) {
            return atual;
        }
        atual = atual->proximo;
    }

    return NULL;
}

// ---------- BEST FIT ---------- 

Bloco *best_fit(Bloco *memoria, size_t tamanho){
    Bloco *atual = memoria;
    Bloco *melhor = NULL;

    while (atual != NULL) {
        if (atual->livre && atual->tamanho >= tamanho) {
            if (melhor == NULL ||
                atual->tamanho < melhor->tamanho) {
                melhor = atual;
            }
        }
        atual = atual->proximo;
    }

    return melhor;
}

// ---------- WORST FIT ---------- 

Bloco *worst_fit(Bloco *memoria, size_t tamanho){
    Bloco *atual = memoria;
    Bloco *pior = NULL;

    while (atual != NULL) {
        if (atual->livre && atual->tamanho >= tamanho) {
            if (pior == NULL ||
                atual->tamanho > pior->tamanho) {
                pior = atual;
            }
        }

        atual = atual->proximo;
    }

    return pior;
}

// ---------- BUSCA DA ESTRATEGIA ---------- 

Bloco *buscar_bloco(Bloco *memoria, size_t tamanho, int estrategia){
    switch (estrategia) {
        case FIRST_FIT:
            return first_fit(memoria, tamanho);

        case BEST_FIT:
            return best_fit(memoria, tamanho);

        case WORST_FIT:
            return worst_fit(memoria, tamanho);

        default:
            return NULL;
    }
}

// ---------- SPLIT ---------- 

void dividir_bloco(Bloco *bloco, size_t tamanho){
    size_t restante = bloco->tamanho - tamanho;

    /*
     * Se o espaço restante for muito pequeno,
     * ele permanece dentro do bloco alocado.
     */
    if (restante < MIN_BLOCO) {
        bloco->livre = 0;
        return;
    }

    Bloco *novo = criar_bloco(
        bloco->inicio + tamanho,
        restante,
        1,
        -1
    );

    novo->proximo = bloco->proximo;
    novo->anterior = bloco;

    if (bloco->proximo != NULL) {
        bloco->proximo->anterior = novo;
    }

    bloco->proximo = novo;

    bloco->tamanho = tamanho;
    bloco->livre = 0;
}

// ---------- COALESCING ---------- 

void coalescer(Bloco *bloco){
    /*
     * Junta com o bloco anterior, caso esteja livre.
     */
    if (bloco->anterior != NULL &&
        bloco->anterior->livre) {

        Bloco *anterior = bloco->anterior;

        anterior->tamanho += bloco->tamanho;
        anterior->proximo = bloco->proximo;

        if (bloco->proximo != NULL) {
            bloco->proximo->anterior = anterior;
        }

        free(bloco);

        bloco = anterior;
    }

    /*
     * Junta com o próximo bloco, caso esteja livre.
     */
    if (bloco->proximo != NULL &&
        bloco->proximo->livre) {

        Bloco *proximo = bloco->proximo;

        bloco->tamanho += proximo->tamanho;
        bloco->proximo = proximo->proximo;

        if (proximo->proximo != NULL) {
            proximo->proximo->anterior = bloco;
        }

        free(proximo);
    }
}

// ---------- ALLOC ---------- 

int alocar(Bloco *memoria, size_t tamanho, int estrategia, int id){
    if (tamanho == 0) {
        printf("O tamanho deve ser maior que zero.\n");
        return 0;
    }

    if (tamanho > MEMORIA_TOTAL) {
        printf("Tamanho maior que a memoria simulada.\n");
        return 0;
    }

    Bloco *bloco = buscar_bloco(
        memoria,
        tamanho,
        estrategia
    );

    if (bloco == NULL) {
        printf("\nNao foi possivel realizar a alocacao.\n");
        printf("Nao existe um bloco livre suficientemente grande.\n");
        return 0;
    }

    dividir_bloco(bloco, tamanho);

    bloco->id = id;

    printf("\nAlocacao realizada com sucesso!\n");
    printf("ID: %d\n", id);
    printf("Inicio: %zu\n", bloco->inicio);
    printf("Tamanho: %zu bytes\n", bloco->tamanho);

    return 1;
}

// ---------- LIBERACAO ---------- 

int liberar(Bloco *memoria, int id){
    Bloco *atual = memoria;

    while (atual != NULL) {

        if (!atual->livre && atual->id == id) {

            atual->livre = 1;
            atual->id = -1;

            coalescer(atual);

            printf("\nBloco %d liberado com sucesso.\n", id);

            return 1;
        }

        atual = atual->proximo;
    }

    printf("\nID %d nao encontrado.\n", id);

    return 0;
}

// ---------- EXIBICAO DA MEMORIA ---------- 

void mostrar_memoria(Bloco *memoria){
    Bloco *atual = memoria;

    printf("\n");
    printf("============================================================\n");
    printf("                    MEMORIA SIMULADA\n");
    printf("============================================================\n");

    while (atual != NULL) {

        printf("[%zu - %zu] ",
               atual->inicio,
               atual->inicio + atual->tamanho - 1);

        if (atual->livre) {
            printf("LIVRE");
        } else {
            printf("ID %d", atual->id);
        }

        printf(" | %zu bytes\n", atual->tamanho);

        atual = atual->proximo;
    }

    printf("============================================================\n");
}

// ---------- MOSTRA ESTRATEGIA ---------- 

void mostrar_estrategia(int estrategia){
    printf("Estrategia atual: ");

    switch (estrategia) {

        case FIRST_FIT:
            printf("FIRST FIT\n");
            break;

        case BEST_FIT:
            printf("BEST FIT\n");
            break;

        case WORST_FIT:
            printf("WORST FIT\n");
            break;

        default:
            printf("DESCONHECIDA\n");
    }
}

// ---------- ALTERA ESTRATEGIA ---------- 

int escolher_estrategia(){
    int estrategia;

    printf("\n");
    printf("========== ESTRATEGIA ==========\n");
    printf("1 - First Fit\n");
    printf("2 - Best Fit\n");
    printf("3 - Worst Fit\n");
    printf("Escolha: ");

    scanf("%d", &estrategia);

    if (estrategia < 1 ||
        estrategia > 3) {

        printf("Estrategia invalida.\n");
        return 0;
    }

    return estrategia;
}

// ----------  LIBERA LISTA ---------- 

void destruir_memoria(Bloco *memoria){
    Bloco *atual = memoria;

    while (atual != NULL) {

        Bloco *proximo = atual->proximo;

        free(atual);

        atual = proximo;
    }
}

// ----------  GERA 10 VALORES ALEATORIOS ---------- 

void preencher_aleatorio(Bloco *memoria, int estrategia, int *proximo_id){
    printf("\nGerando 10 alocacoes aleatorias...\n");

    for (int i = 0; i < 10; i++) {
        size_t tamanho = 32 + rand() % (8192 - 32 + 1);
        printf("\nAlocacao %d: %zu bytes\n", i + 1, tamanho);

        if (alocar(
                memoria,
                tamanho,
                estrategia,
                *proximo_id)) {
            (*proximo_id)++;
        }
    }

    printf("\n10 tentativas de alocacao realizadas.\n");
}