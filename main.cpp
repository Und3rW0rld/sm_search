#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 6 // Número de procesadores

typedef struct {
    int position;
    int thread_id;
} Match;

int sequential_search_all(int* S, int start, int end, int x, Match* local_matches, int tid) {
    int count = 0;
    for (int i = start; i < end; i++) {
        printf("Soy el hilo %d buscando a %d en la posición %d\n", tid, x, i);
        if (S[i] == x) {
            local_matches[count].position = i;
            local_matches[count].thread_id = tid;
            count++;
        }
    }
    return count; // cuántos encontró este hilo
}

void sm_search(int* S, int n, int x, Match* myks, int* match_count_out) {
    int chunk = (int) ceil((double)n / N);
    int match_count = 0;

    omp_set_num_threads(N);

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        int start = tid * chunk;
        int end = (tid == N - 1) ? n : start + chunk;

        printf("Hilo %d buscando en posiciones [%d - %d): \n", tid, start, end);
        for (int i = start; i < end; i++) {
            printf("%d ", S[i]);
        }
        printf("\n");

        // Cada hilo hace búsqueda secuencial en su segmento
        Match local_matches[100]; // máx 100 coincidencias por hilo (ajustable)
        int local_count = sequential_search_all(S, start, end, x, local_matches, tid);

        // Guardar resultados en arreglo compartido
        #pragma omp critical
        {
            for (int j = 0; j < local_count; j++) {
                myks[match_count] = local_matches[j];
                match_count++;
            }
        }
    }

    *match_count_out = match_count;
}

int main() {
    int S[12] = {6, 11, 15, 13, 9, 4, 10, 18, 1, 6, 14,6}; // Secuencia de números
    int x = 6; // Dato a buscar
    int n = sizeof(S) / sizeof(S[0]); // Tamaño de S

    Match* myks = (Match*) malloc(n * sizeof(Match)); 
    int match_count = 0;

    double start_time = omp_get_wtime();
    sm_search(S, n, x, myks, &match_count);
    double end_time = omp_get_wtime();

    printf("\nPosiciones encontradas:\n");
    for (int i = 0; i < match_count; i++) {
        printf("x encontrado en S[%d] por hilo %d\n", myks[i].position, myks[i].thread_id);
    }

    printf("\nTiempo total (OpenMP): %f segundos\n", end_time - start_time);

    free(myks);
    return 0;
}

