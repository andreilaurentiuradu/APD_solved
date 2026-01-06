#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

int N;

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
	for(i = 0; i < N; i++) {
		printf("%d ", v[i]);
	}
	printf("\n");
}

int cmp(const void *a, const void *b) {
	// DO NOT MODIFY
	int A = *(int*)a;
	int B = *(int*)b;
	return A-B;
}

// Use 'mpirun -np 20 --oversubscribe ./pipeline_sort' to run the application with more processes
int main(int argc, char * argv[]) {
	int rank;
	int nProcesses;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
	printf("Hello from %i/%i\n", rank, nProcesses);

	if(rank==0) { // This code is run by a single process
		int intialValue = -1;
		int sorted = 0;
		int aux;
		int *v = (int*)malloc(sizeof(int) * (nProcesses - 1));
		int *vQSort = (int*)malloc(sizeof(int) * (nProcesses - 1));
		int i, val;

		// generate the vector v with random values
		// DO NOT MODIFY
		srandom(42);
		for(i = 0; i < nProcesses - 1; i++)
			v[i] = random() % 200;
		N = nProcesses - 1;
		displayVector(v);

		// make copy to check it against qsort
		// DO NOT MODIFY
		for(i = 0; i < nProcesses - 1; i++)
			vQSort[i] = v[i];
		qsort(vQSort, nProcesses - 1, sizeof(int), cmp);

		// TODO send the vector to rank == 1
		// trimitem element cu element pentru a simula fluxul
		for (int i = 0; i < nProcesses - 1; ++i) {
			MPI_Send(&v[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		}

		// colectam rezultatele sortate
		// fiecare proces worker (1, N) a retinut cate un numar
		// le primitm inapoi si reconstituim vectorul v

		for (int i = 1; i < nProcesses; ++i) {
			int received_val;
			MPI_Recv(&received_val, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			v[i-1] = received_val;
		}

		displayVector(v);
		compareVectors(v, vQSort);

		free(v);
		free(vQSort);
	} else {
		// TODO sort the vector v using N processes (N == nProcesses - 1)
		int current_val = -1;
		int received_val;

		// nr total de elemente care vor trece prin acest proces
		// rank 1 primeste n elemente, rank 2 n-1 si tot asa n = nProcesses - 1

		int elements_to_receive = (nProcesses - 1) - (rank - 1);

		// 1. Primim primul element si il pastram ca valoare curenta
        MPI_Recv(&current_val, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// 2. Procesam restul fluxului de date
        for (int i = 1; i < elements_to_receive; i++) {
            MPI_Recv(&received_val, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if (received_val < current_val) {
                // Daca am primit ceva mai mic decat ce am eu,
                // trimit mai departe ce aveam eu si pastrez noul numar mic
                MPI_Send(&current_val, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                current_val = received_val;
            } else {
                // Daca am primit ceva mai mare, il trimit mai departe
                MPI_Send(&received_val, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
            }
        }

        // 3. La final, trimitem valoarea retinuta inapoi la MASTER (Rank 0)
        // pentru a fi asamblata in vectorul final
        MPI_Send(&current_val, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
