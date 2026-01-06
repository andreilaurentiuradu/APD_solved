#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

int main (int argc, char *argv[])
{
    int procs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int value = rank, tmp_value;
    MPI_Status status;

    // merge doar pe puteri ale lui 2
    for (int i = 2; i <= procs; i *= 2) {
        // TODO
        if (rank % i == 0) {
            int source = rank + i / 2;
            if (source < procs) {
                // primeste de la procesul cu rankul (rank + i / 2)
                MPI_Recv(&tmp_value, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
                // aduna
                value += tmp_value;
            }
        } else if (rank % (i / 2) == 0) {
            int dest = rank - i / 2;
            // trimite la procesul cu rankul (rank - i / 2)
            MPI_Send(&value, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == MASTER) {
        printf("Result = %d\n", value);
    }

    MPI_Finalize();

}

