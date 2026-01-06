#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[])
{
    int numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int recv_num;
    // Avem nevoie de request si status pentru functiile non-blocante
    MPI_Request request;
    MPI_Status status;

    // First process starts the circle.
    if (rank == 0) {
        // First process starts the circle.
        // Generate a random number.
        srand(time(NULL)); // Initializam seed-ul random
        recv_num = rand() % 100;
        printf("Rank 0 a generat numarul: %d\n", recv_num);
        // Send the number to the next process.
        // Pornim trimiterea
        MPI_Isend(&recv_num, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
        // Asteptam sa se termine trimiterea (sa plece datele din buffer)
        MPI_Wait(&request, &status);

        // --- Procesul 0 trebuie sa si primeasca la final ---
        // Primeste de la ultimul proces
        MPI_Irecv(&recv_num, 1, MPI_INT, numtasks - 1, 0, MPI_COMM_WORLD, &request);
        // Asteptam sa ajunga datele efectiv
        MPI_Wait(&request, &status);
        
        printf("Rank 0 a primit inapoi valoarea finala: %d\n", recv_num);

    } else if (rank == numtasks - 1) {
        // Last process close the circle.
        // Receives the number from the previous process.
        // 1. Initializam receptia
        MPI_Irecv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &request);
        // 2. Asteptam finalizarea receptiei pentru a putea folosi recv_num
        MPI_Wait(&request, &status);
        printf("Rank %d a primit %d\n", rank, recv_num);
        // Increments the number.
        recv_num += 2;
        // Sends the number to the first process.
        MPI_Isend(&recv_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);

    } else {
        // Middle process.
        // Receives the number from the previous process.
        MPI_Irecv(&recv_num, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status); // Blocaj pana avem datele

        printf("Rank %d a primit %d\n", rank, recv_num);
        // Increments the number.
        recv_num += 2;
        // Sends the number to the next process.
        MPI_Isend(&recv_num, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);

    }

    MPI_Finalize();

}

