#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv)
{
    int i;
    int iters;
    int tag;
    int rank;
    int numranks;
    int number;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);


    iters = 100000;
    double start, end, t;
    double* trecv = (double*) malloc(iters*sizeof(double));
    double* tsend = (double*) malloc(iters*sizeof(double));
    double total_trecv = 0;
    double total_tsend = 0;
    for (i = 0; i < iters; i++) {
	if (rank != 0) {
	    start = MPI_Wtime();
	    MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	    end = MPI_Wtime();
	    t = end - start;
	    trecv[number] = t;
	    //printf("[%d]: number = %d\n", rank, number);
	    //printf("[%d]: %d. t = %.10e\n", rank, i, t);
	}
	if (rank  == 0) {
	    number = i;
	    start = MPI_Wtime();
	    MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	    end = MPI_Wtime();
	    t = end - start;
	    tsend[number] = t;
	    //printf("[%d]: %d. t = %.10e\n", rank, i, t);
	}
    }
    
    if (rank == 0) {
	MPI_Recv(trecv, iters, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &status);
	for (i = 0; i < iters; i++) {
	    total_trecv += trecv[i];
	    total_tsend += tsend[i];
	    //printf("[%d]: %d. t = %.10e\n", rank, i, trecv[i]);
	}
	printf("[%d]: total_tsend = %.10e\n", rank, total_tsend);
	printf("[%d]: tsend/iter = %.10e\n", rank, total_tsend/iters);
	printf("[%d]: iters = %d\n", rank, iters);

	
    }
    if (rank != 0) {
	MPI_Send(trecv, iters, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	for (i = 0; i < iters; i++) {
	    total_trecv += trecv[i];
	}
	printf("[%d]: total_trecv = %.10e\n", rank, total_trecv);
	printf("[%d]: trecv/iter = %.10e\n", rank, total_trecv/iters);
    }
    free(trecv);
    free(tsend);
    MPI_Finalize();

    return 0;
}
