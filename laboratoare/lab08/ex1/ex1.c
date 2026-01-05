#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[])
{
    int  numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int recv_num;
    MPI_Status status;
    srand(time(NULL));

    if (numtasks < 2) {
        if (rank == 0) printf("Eroare: Ai nevoie de minim 2 procese. Ai rulat cu %d.\n", numtasks);
        MPI_Finalize();
        return 0;
    }

    // First process starts the circle.
    if (rank == 0) {
        // First process starts the circle.
        // Generate a random number.
        recv_num = rand() % 100;
        // Send the number to the next process.
        MPI_Send(&recv_num, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        // Primeste de la procesul final
        MPI_Recv(&recv_num, 1, MPI_INT, numtasks - 1, 0, MPI_COMM_WORLD, &status);
        printf("Back to process %d, received number %d (Close the ring)\n", rank, recv_num);

    } else if (rank == numtasks - 1) {
        // Last process close the circle.
        // Receives the number from the previous process.
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
        printf("Process %d received number %d (Final process)\n", rank, recv_num);
        // Increments the number.
        recv_num += 2;
        // Sends the number to the first process.
        MPI_Send(&recv_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    } else {
        // Middle process.
        // Receives the number from the previous process.
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
        printf("Process %d received number %d\n", rank, recv_num);
        // Increments the number.
        recv_num += 2;
        // Sends the number to the next process.
        MPI_Send(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);

    }

    MPI_Finalize();

}

