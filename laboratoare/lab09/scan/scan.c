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

    int value = rank, tmp_val;

    for (int i = 1; i < procs; i *= 2) {
        // TODO
        if (rank + i < procs) {
            // trimite la procesul cu rankul rank + i
            MPI_Send(&value, 1, MPI_INT, rank + i, 0, MPI_COMM_WORLD);
        }
        if (rank - i >= 0) {
            // primeste de la procesul cu rankul rank - i
            MPI_Recv(&tmp_val, 1, MPI_INT, rank - i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // aduna
            value += tmp_val;
        }
    }

    printf("Process [%d] has result = %d\n", rank, value);

    MPI_Finalize();

}

