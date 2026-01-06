#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define SIZE 500000
 
int main (int argc, char *argv[])
{
    int  numtasks, rank, len;
 
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks); // Total number of processes.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // The current process ID / Rank.
 
    srand(42);
    int num1[SIZE], num2[SIZE];
    
    // 1. Calculam dimensiunea bufferului necesar
    // Dimensiunea datelor + overhead-ul specific MPI
    int buffer_size = SIZE * sizeof(int) + MPI_BSEND_OVERHEAD;
    void *buffer = malloc(buffer_size);

    // 2. Atasam bufferul
    MPI_Buffer_attach(buffer, buffer_size);

    if (rank == 0) {
        for (int i = 0; i < SIZE; i++) {
            num1[i] = 100;
        }
        // MPI_Send(&num1, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
        // MPI_Recv(&num2, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Bsend returneaza imediat dupa ce copiaza datele in buffer
        MPI_Bsend(num1, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(num2, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 0 a terminat.\n");
    } else {
        for (int i = 0; i < SIZE; i++) {
            num2[i] = 200;
        }
        // MPI_Send(&num2, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
        // MPI_Recv(&num1, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Bsend(num2, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(num1, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 1 a terminat.\n");
    }

    // 3. Detasam si eliberam bufferul
    MPI_Buffer_detach(&buffer, &buffer_size);
    free(buffer);
    
    MPI_Finalize();
 
}