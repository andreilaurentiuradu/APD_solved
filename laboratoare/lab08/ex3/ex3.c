#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MULTI 5 // chunk dimension
#define ROOT 0

int main (int argc, char *argv[])
{
    int  numtasks, rank, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Get_processor_name(hostname, &len);

    int num_elements = MULTI * numtasks; // total elements
    int *v_send = NULL; // full vector
    int *v_recv = (int *)malloc(MULTI * sizeof(int)); // partial vector

    // ROOT process generates the values for the full vector.
    // Scatter the vector to all processes.
    if (rank == ROOT) {
        v_send = malloc(num_elements * sizeof(int));
        for (int i = 0; i < num_elements; ++i) {
            v_send[i] = i;
        }
    }
    // imparte vectorul in chunkuri si le trimite la toate procesele
    MPI_Scatter(v_send, MULTI, MPI_INT, v_recv, MULTI, MPI_INT, ROOT, MPI_COMM_WORLD);

    /*
     * Prints the values received after scatter.
     * NOTE: If MULTI changed, also change this line.
     */
    // afiseaza ce a primit
    printf("Process [%d]: have elements %d %d %d %d %d.\n", rank, v_recv[0],
            v_recv[1], v_recv[2], v_recv[3], v_recv[4]);

    // Incrementeaza valorile primite
    for (int i = 0; i < MULTI; ++i) {
        v_recv[i] += rank;
    }
    // Le trimite inapoi incrementate
    MPI_Gather(v_recv, MULTI, MPI_INT, v_send, MULTI, MPI_INT, ROOT, MPI_COMM_WORLD);
    // Gathers the values from all the processes.
    // The ROOT process prints the elements received.

    if (rank == ROOT) {
        printf("Elmentele finale: ");
        for (int i = 0; i < num_elements; ++i) {
            printf("%d ", v_send[i]);
        }
        printf("\n");
        free(v_send);
    }

    // eliberam memoria
    free(v_recv);
    MPI_Finalize();

}

