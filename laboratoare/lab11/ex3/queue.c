#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>

typedef struct {
    int size;
    int arr[1000];
} queue;

int main (int argc, char *argv[]) {
    int numtasks, rank;

    queue q;
    // TODO: declare the types and arrays for offsets and block counts
    MPI_Datatype custom_queue_type;
    int blockcounts[2];
    MPI_Aint offsets[2];
    MPI_Datatype types[2];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // TODO: create the MPI data type, using offsets and block counts, and commit the data type
    // 1. Definim blocul pentru 'size' (1 int)
    blockcounts[0] = 1;
    offsets[0] = offsetof(queue, size);
    types[0] = MPI_INT;

    // 2. Definim blocul pentru 'arr' (1000 int)
    blockcounts[1] = 1000;
    offsets[1] = offsetof(queue, arr);
    types[1] = MPI_INT;

    // Cream structura si o comitem
    MPI_Type_create_struct(2, blockcounts, offsets, types, &custom_queue_type);
    MPI_Type_commit(&custom_queue_type);

    srand(time(NULL) + rank);
    int val = rand() % 100; // Generam un numar intre 0 si 99

    // First process starts the circle.
    if (rank == 0) {
        // First process starts the circle.
        // Initializam coada doar la procesul 0
        q.size = 0;
        // Generate a random number and add it in queue.
        q.arr[q.size++] = val;
        printf("Procesul 0 a generat: %d si trimite catre 1.\n", val);
        // Sends the queue to the next process.
        MPI_Send(&q, 1, custom_queue_type, 1, 0, MPI_COMM_WORLD);

        // Procesul 0 trebuie sa primeasca coada inapoi de la ultimul proces pentru a afisa rezultatul
        MPI_Recv(&q, 1, custom_queue_type, numtasks - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    } else if (rank == numtasks - 1) {
        // Last process close the circle.
        // Receives the queue from the previous process.
        MPI_Recv(&q, 1, custom_queue_type, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Generate a random number and add it in queue.
        q.arr[q.size++] = val;
        printf("Procesul %d (ultimul) a primit coada, a generat: %d si trimite catre 0.\n", rank, val);

        // Sends the queue to the first process.
        MPI_Send(&q, 1, custom_queue_type, 0, 0, MPI_COMM_WORLD);

    } else {
        // Middle process.
        // Receives the queue from the previous process.
        MPI_Recv(&q, 1, custom_queue_type, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Generate a random number and add it in queue.
        q.arr[q.size++] = val;
        printf("Procesul %d a primit coada, a generat: %d si trimite catre %d.\n", rank, val, rank + 1);

        // Sends the queue to the next process.
        MPI_Send(&q, 1, custom_queue_type, rank + 1, 0, MPI_COMM_WORLD);
    }

    // TODO: Process 0 prints the elements from queue
    if (rank == 0) {
        printf("\n--- Rezultat Final (Colectat la Procesul 0) ---\n");
        printf("Dimensiune coada: %d\n", q.size);
        printf("Elemente: ");
        for (int i = 0; i < q.size; i++) {
            printf("%d ", q.arr[i]);
        }
        printf("\n");
    }

    // TODO: free the newly created MPI data type
    MPI_Type_free(&custom_queue_type);
    
    MPI_Finalize();
}