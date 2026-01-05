#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define GROUP_SIZE 4

int main (int argc, char *argv[])
{
    int old_size, new_size;
    int old_rank, new_rank;
    int recv_rank;
    MPI_Comm custom_group;
	MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &old_size); // Total number of processes.
    MPI_Comm_rank(MPI_COMM_WORLD, &old_rank); // The current process ID / Rank.

	if (old_size % GROUP_SIZE != 0) {
        if (old_rank == 0) printf("Eroare: Numarul de procese trebuie sa fie divizibil cu %d.\n", GROUP_SIZE);
        MPI_Finalize();
        return 0;
    }

    // Split the MPI_COMM_WORLD in small groups.
	// facem grupe de cate GROUP_SIZE
	int color = old_rank / GROUP_SIZE;
	int key = old_rank;

	// spargem comunicatorul in color grupe
	MPI_Comm_split(MPI_COMM_WORLD, color, key, &custom_group);

	// aflam noul rank si noua dimensiune
	MPI_Comm_size(custom_group, &new_size);
	MPI_Comm_rank(custom_group, &new_rank);

    printf("Rank [%d] / size [%d] in MPI_COMM_WORLD and rank [%d] / size [%d] in custom group.\n",
            old_rank, old_size, new_rank, new_size);

    // Send the rank to the next process.
	if (new_rank == 0) {
		MPI_Send(&new_rank, 1, MPI_INT, new_rank + 1, 0, custom_group);
		MPI_Recv(&recv_rank, 1, MPI_INT, new_size - 1, 0, custom_group, &status);
		printf("Process [%d] from group [%d] received [%d]\n", 
               new_rank, color, recv_rank);
	} else {
		MPI_Recv(&recv_rank, 1, MPI_INT, new_rank - 1, 0, custom_group, &status);
		printf("Process [%d] from group [%d] received [%d].\n", new_rank,
            old_rank / GROUP_SIZE, recv_rank);
		if (new_rank == new_size - 1) {
			MPI_Send(&new_rank, 1, MPI_INT, 0, 0, custom_group);
		} else {
			MPI_Send(&new_rank, 1, MPI_INT, new_rank + 1, 0, custom_group);
		}	
	}
    
	// curatenie
	MPI_Comm_free(&custom_group);
    MPI_Finalize();
}

