//para geração de numeros aleatorios
#include <time.h>
//rest
#include <stdio.h>
#include "functions.c"

int main() {
    srand((unsigned int) time(NULL));
    
    Bloco *memoria = inicializar_memoria();

    int estrategia = 1;
    int opcao;
    int proximo_id = 1;

    printf("===============================================\n");
    printf("   SIMULADOR DE ALOCACAO DINAMICA DE MEMORIA\n");
    printf("===============================================\n");

    printf("\nMemoria simulada: %d bytes (1 MiB)\n",
           MEMORIA_TOTAL);

    printf("Estrategia inicial: First Fit\n");

    do {

        printf("\n");
        printf("=============== MENU ===============\n");

        mostrar_estrategia(estrategia);

        printf("\n");
        printf("1 - Alocar memoria\n");
        printf("2 - Liberar memoria\n");
        printf("3 - Mostrar memoria\n");
        printf("4 - Alterar estrategia\n");
        printf("5 - Gerar 10 alocacoes aleatorias\n");
        printf("0 - Sair\n");

        printf("\nOpcao: ");
        scanf("%d", &opcao);


        switch (opcao) {

            /* -----------------------------------------
               ALLOC
               ----------------------------------------- */

            case 1:{
                size_t tamanho;

                printf("\nTamanho da memoria a alocar (bytes): ");
                scanf("%zu", &tamanho);

                if (alocar(
                        memoria,
                        tamanho,
                        estrategia,
                        proximo_id)) {

                    proximo_id++;
                }

                break;
            }


            /* -----------------------------------------
               FREE
               ----------------------------------------- */

            case 2:{
                int id;

                printf("\nID da alocacao a liberar: ");
                scanf("%d", &id);

                liberar(memoria, id);

                break;
            }


            /* -----------------------------------------
               MOSTRAR MEMORIA
               ----------------------------------------- */

            case 3:{
                mostrar_memoria(memoria);
                break;
            }

            /* -----------------------------------------
               ALTERAR ESTRATEGIA
               ----------------------------------------- */

            case 4:{
                int nova_estrategia = escolher_estrategia();

                if (nova_estrategia != 0) {
                    estrategia = nova_estrategia;

                    printf("\nEstrategia alterada com sucesso.\n");
                }

                break;
            }
            case 5:{

                preencher_aleatorio(
                    memoria,
                    estrategia,
                    &proximo_id
                );

                break;
            }

            /* -----------------------------------------
               SAIR
               ----------------------------------------- */

            case 0:{
                printf("\nEncerrando simulador...\n");
                break;
            }

            default:
                printf("\nOpcao invalida.\n");
        }

    } while (opcao != 0);

    destruir_memoria(memoria);
    return 0;
}