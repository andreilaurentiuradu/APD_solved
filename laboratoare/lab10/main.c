#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define CONVERGENCE_COEF 100
#define TAG_SONDA 0
#define TAG_ECOU 1

/**
 * Run: mpirun --oversubscribe -np 12 ./a.out
 */

static int num_neigh;
static int *neigh;

void read_neighbours(int rank) {
    FILE *fp;
    char file_name[15];
    sprintf(file_name, "./files/%d.in", rank);

    fp = fopen(file_name, "r");
    if (!fp) return;
    fscanf(fp, "%d", &num_neigh);

    neigh = malloc(sizeof(int) * num_neigh);

    for (size_t i = 0; i < num_neigh; i++)
        fscanf(fp, "%d", &neigh[i]);
    fclose(fp);
}

int* get_dst(int rank, int numProcs, int leader) {
    MPI_Status status;
    
    /* Vectori de parinti */
    int *v = malloc(sizeof(int) * numProcs);
    int *vRecv = malloc(sizeof(int) * numProcs);
    
    int sonda = 42;
    int parent = -1;

    // Initializare
    for(int i=0; i<numProcs; i++) { v[i] = -1; vRecv[i] = -1; }
    
    if (rank == leader)
        v[rank] = -1;
    else {
        /* Daca procesul curent nu este liderul, inseamna ca va astepta un mesaj de la un parinte */
        MPI_Recv(&sonda, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        v[rank] = status.MPI_SOURCE;
        parent = status.MPI_SOURCE;
    }

    /*
    * TODO2: Pentru fiecare proces vecin care nu este parintele procesului curent,
    * voi trimite o sonda. 
    */
    for (int i = 0; i < num_neigh; i++) {
        if (neigh[i] != parent) {
            MPI_Send(&sonda, 1, MPI_INT, neigh[i], TAG_SONDA, MPI_COMM_WORLD);
        }
    }

    /*
    * TODO2: Vom astepta de la fiecare proces vecin care nu este parintele procesului curent vectorul de parinti sau o sonda.
            Daca primim un ecou (vector de parinti), actualizam vectorul propriu de parinti daca exista informatii aditionale.
        HINT: Pentru simplitate, puteti face mereu recv ca pentru vectorul de parinti si sa verificati size-ul receptiei sau tag-ul
            pentru a determina daca este sonda sau ecou.
    */
    for (int i = 0; i < num_neigh; i++) {
        if (neigh[i] != parent) {
            MPI_Recv(vRecv, numProcs, MPI_INT, neigh[i], MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            if (status.MPI_TAG == TAG_ECOU) {
                // Am primit un ecou (vector de parinti)
                for (int k = 0; k < numProcs; k++) {
                    if (vRecv[k] != -1) v[k] = vRecv[k];
                }
            }
        }
    }

    /*
    * TODO2: Orice proces ce nu este lider va propaga vectorul de vecini parintelui lui si va astepta topologia completa de la acesta
    */
    if (rank != leader) {
        MPI_Send(v, numProcs, MPI_INT, parent, TAG_ECOU, MPI_COMM_WORLD);
        MPI_Recv(v, numProcs, MPI_INT, parent, TAG_ECOU, MPI_COMM_WORLD, &status);
    }

    /*
    * TODO2: Procesul curent va trimite doar copiilor lui topologia completa
    */
    for (int i = 0; i < numProcs; i++) {
        if (v[i] == rank && i != rank) {
            MPI_Send(v, numProcs, MPI_INT, i, TAG_ECOU, MPI_COMM_WORLD);
        }
    }

    for (int i = 0; i < numProcs && rank == leader; i++) {
        printf("The node %d has the parent %d\n", i, v[i]);
    }

    free(vRecv);
    return v;
}

int leader_chosing(int rank, int nProcesses) {
    int leader = -1;
    int q;
    leader = rank;
    
    /* Executam acest pas pana ajungem la convergenta */
    for (int k = 0; k < CONVERGENCE_COEF; k++) {
        /* TODO1: Pentru fiecare vecin, vom trimite liderul pe care il cunosc 
        * si voi astepta un mesaj de la orice vecin
        * Daca liderul e mai mare decat al meu, il actualizez pe al meu
        */
        for (int i = 0; i < num_neigh; ++i) {
            MPI_Send(&leader, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
        }

        for (int i = 0; i < num_neigh; ++i) {
            int received_leader;
            MPI_Recv(&received_leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (received_leader > leader) {
                leader = received_leader;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    printf("%i/%i: leader is %i\n", rank, nProcesses, leader);

    return leader;
}

int get_number_of_nodes(int rank, int leader) {
    
    double val;
    if (leader == rank) {
        val = 1.0;
    } else {
        val = 0.0;
    }

    double recvd = 0;
    /* Executam acest pas pana ajungem la convergenta */
    for (int k = 0; k < CONVERGENCE_COEF; k++) {
        /* TODO3: Pentru fiecare vecin, vom trimite valoarea pe care o cunosc
        * si voi astepta un mesaj de la el
        * Cu valoarea primita, actualizam valoarea cunoscuta ca fiind
        * media dintre cele 2
        */
        for (int i = 0; i < num_neigh; i++) {
            MPI_Send(&val, 1, MPI_DOUBLE, neigh[i], 0, MPI_COMM_WORLD);
        }

        double sum = val;
        for (int i = 0; i < num_neigh; i++) {
            double temp_rec;
            MPI_Recv(&temp_rec, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += temp_rec;
        }
        val = sum / (num_neigh + 1);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    return (int)(1.0 / val + 0.5);
}

int ** get_topology(int rank, int nProcesses, int * parents, int leader) {
    int ** topology = malloc(sizeof(int*) * nProcesses);
    
    // -- CORECTIE CRITICA PENTRU TODO4 --
    // Alocam o singura zona de memorie continua pentru a putea trimite prin MPI
    int *flat_topology = calloc(nProcesses * nProcesses, sizeof(int));
    int *recv_buffer = calloc(nProcesses * nProcesses, sizeof(int));
    
    // Mapam vectorul de pointeri la zona continua
    for (size_t i = 0; i < nProcesses; i++) {
        topology[i] = flat_topology + i * nProcesses;
    }
    // ------------------------------------

    for (size_t i = 0; i < num_neigh; i++) {
        topology[rank][neigh[i]] = 1;
    }

    /* TODO4: Primim informatii de la toti copii si actualizam matricea de topologie */
    for (int i = 0; i < nProcesses; i++) {
        if (parents[i] == rank && i != rank) {
            MPI_Recv(recv_buffer, nProcesses * nProcesses, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Reuniune (OR logic)
            for (int k = 0; k < nProcesses * nProcesses; k++) {
                if (recv_buffer[k] == 1) flat_topology[k] = 1;
            }
        }
    }

    /* TODO4: Propagam matricea proprie catre parinte */
    if (rank != leader) {
        MPI_Send(flat_topology, nProcesses * nProcesses, MPI_INT, parents[rank], 0, MPI_COMM_WORLD);
        
        /* TODO4: Daca nu suntem liderul, asteptam topologia completa de la parinte  */
        MPI_Recv(flat_topology, nProcesses * nProcesses, MPI_INT, parents[rank], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    /* TODO4: Trimitem topologia completa copiilor */
    for (int i = 0; i < nProcesses; i++) {
        if (parents[i] == rank && i != rank) {
            MPI_Send(flat_topology, nProcesses * nProcesses, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }

    free(recv_buffer);
    // free(flat_topology); // topology depinde de el
    return topology;
}

int main(int argc, char * argv[]) {
    int rank, nProcesses, num_procs, leader;
    int *parents, **topology;

    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    if (nProcesses != 12) {
        if (rank==0) printf("Warning: Run with 12 processes for full correctness.\n");
    }
     
    read_neighbours(rank);
    leader = leader_chosing(rank, nProcesses);
    
    MPI_Barrier(MPI_COMM_WORLD);

    parents = get_dst(rank, nProcesses, leader);

    MPI_Barrier(MPI_COMM_WORLD);

    num_procs = get_number_of_nodes(rank, leader);
    
    if (rank == leader) printf("%d/%d There are %d processes\n", rank, nProcesses,num_procs);

    topology = get_topology(rank, nProcesses, parents, leader);

    // Afiseaza doar liderul pentru claritate
    if (rank == leader) {
        for (size_t i = 0; i < nProcesses; i++)
        {
            for (size_t j = 0; j < nProcesses; j++)
            {
                printf("%2d ", topology[i][j]); 
            }
            printf("\n");
        }
    }
    
    MPI_Finalize();
    return 0;
}