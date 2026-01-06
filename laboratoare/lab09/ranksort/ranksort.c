#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#define N 10
#define MASTER 0

void compareVectors(int * a, int * b) {
	// DO NOT MODIFY
	int i;
	for(i = 0; i < N; i++) {
		if(a[i]!=b[i]) {
			printf("Sorted incorrectly\n");
			return;
		}
	}
	printf("Sorted correctly\n");
}

void displayVector(int * v) {
	// DO NOT MODIFY
	int i;
	int displayWidth = 2 + log10(v[N-1]);
	for(i = 0; i < N; i++) {
		printf("%*i", displayWidth, v[i]);
	}
	printf("\n");
}

int cmp(const void *a, const void *b) {
	// DO NOT MODIFY
	int A = *(int*)a;
	int B = *(int*)b;
	return A-B;
}
 
int main(int argc, char * argv[]) {
	int rank, i, j;
	int nProcesses;
	MPI_Init(&argc, &argv);
	int pos[N];
	int sorted = 0;
	int *v = (int*)malloc(sizeof(int)*N);
	int *vQSort = (int*)malloc(sizeof(int)*N);

	for (i = 0; i < N; i++)
		pos[i] = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
	printf("Hello from %i/%i\n", rank, nProcesses);

    if (rank == MASTER) {
        // generate random vector
		srand(42);
        for(i = 0; i < N; i++) {
            v[i] = rand() % 5000;
        }
    }

    // send the vector to all processes
    // Toate procesele au nevoie de vectorul complet 'v' pentru a face comparatii
    MPI_Bcast(v, N, MPI_INT, MASTER, MPI_COMM_WORLD);

	// Calculam intervalele pentru fiecare proces (pentru a sti cat sa procesam)
    int start = rank * N / nProcesses;
    int end = (rank + 1) * N / nProcesses;
    if (rank == nProcesses - 1) end = N; // Ultimul ia restul

	if(rank == 0) {
		// DO NOT MODIFY
		displayVector(v);

		// make copy to check it against qsort
		// DO NOT MODIFY
		for(i = 0; i < N; i++)
			vQSort[i] = v[i];
		qsort(vQSort, N, sizeof(int), cmp);

		// sort the vector v
		// 1. Master isi calculeaza partea LUI de pozitii (de la start la end, unde rank e 0)
        for(i = start; i < end; i++) {
            pos[i] = 0;
            for(j = 0; j < N; j++) {
                // Algoritmul Rank Sort: numaram cate sunt mai mici
                // Partea (v[j] == v[i] && j < i) este pentru stabilitate la duplicate
                if (v[j] < v[i] || (v[j] == v[i] && j < i)) {
                    pos[i]++;
                }
            }
        }

        // recv the new pozitions
		// 2. Master primeste pozitiile calculate de ceilalti muncitori
        for(int r = 1; r < nProcesses; r++) {
            int r_start = r * N / nProcesses;
            int r_end = (r + 1) * N / nProcesses;
            if (r == nProcesses - 1) r_end = N;
            
            // Primim exact bucata calculata de procesul r
            MPI_Recv(&pos[r_start], r_end - r_start, MPI_INT, r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

		// 3. Reconstruim vectorul final folosind vectorul 'pos' completat
        int *tempV = (int*)malloc(sizeof(int)*N);
        for(i = 0; i < N; i++) {
            tempV[pos[i]] = v[i];
        }
        for(i = 0; i < N; i++) v[i] = tempV[i];
        free(tempV);

		displayVector(v);
		compareVectors(v, vQSort);
	} else {
		
        // compute the positions
		// Fiecare worker calculeaza rank-ul doar pentru bucata sa (start -> end)
        for(i = start; i < end; i++) {
            pos[i] = 0;
            for(j = 0; j < N; j++) {
                if (v[j] < v[i] || (v[j] == v[i] && j < i)) {
                    pos[i]++;
                }
            }
        }
        // send the new positions to process MASTER
		// Trimitem doar bucata din 'pos' pe care am calculat-o
        MPI_Send(&pos[start], end - start, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
	}

	free(v);
    free(vQSort);
	MPI_Finalize();
	return 0;
}
