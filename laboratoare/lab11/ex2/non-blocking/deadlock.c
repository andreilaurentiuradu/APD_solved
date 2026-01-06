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

    // Avem nevoie de un vector de request-uri (unul pt send, unul pt recv)
    MPI_Request requests[2];
    MPI_Status statuses[2];

 
    if (rank == 0) {
        for (int i = 0; i < SIZE; i++) {
            num1[i] = 100;
        }
        // MPI_Send(&num1, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD);
        // MPI_Recv(&num2, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Initiem trimiterea (nu blocheaza)
        MPI_Isend(num1, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, &requests[0]);
        // Initiem receptia (nu blocheaza)
        MPI_Irecv(num2, SIZE, MPI_INT, 1, 0, MPI_COMM_WORLD, &requests[1]);
        
        // Asteptam sa se termine ambele (index 0 si 1)
        MPI_Waitall(2, requests, statuses);
        printf("Rank 0 a terminat.\n");

    } else {
        for (int i = 0; i < SIZE; i++) {
            num2[i] = 200;
        }
        // MPI_Send(&num2, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD);
        // MPI_Recv(&num1, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // Initiem trimiterea
        MPI_Isend(num2, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, &requests[0]);
        // Initiem receptia
        MPI_Irecv(num1, SIZE, MPI_INT, 0, 0, MPI_COMM_WORLD, &requests[1]);
        
        // Asteptam sa se termine ambele
        MPI_Waitall(2, requests, statuses);
        printf("Rank 1 a terminat.\n");
    }

    MPI_Finalize();
 
}