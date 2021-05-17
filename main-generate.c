#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>


int main(int argc, char** argv) {
    int world_rank;
    int world_size;
    MPI_Request request;


    double start, end, elapsed;
    unsigned ITERS = 3200;
    unsigned i;
    
    double *isendtimes = (double*) malloc(ITERS*sizeof(double));
    double isendtotaltime = 0.0;
    double isendavg = 0.0;
    double isendstddev = 0.0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    
    // Check to make sure we have enough ranks
    if (world_size != 2) {
        printf("main-generate: World size must be 2. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    } else {
        printf("main-generate: World size is 2!\n");
    }

    int x;
    // This is a weird way to do things for this example....
    if (world_rank != 0) {
        // we are the recieving rank: we get data, and
        // sdskv puts data in the DB.
        while (1) {
            MPI_Recv(&x, 1, MPI_INT,
                      0, 0,
                      MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("main-generate: Rank 1 recieved number %d from process 0\n", x);

            if (x == -1){
                //printf("main-generate: Rank 1 recieved -1. Breaking...\n");
                break;
            }
        }

    } else {
        // we are the first rank
        int i;
        for (i = 0; i < ITERS; i++) {
            x = 2*i + 1;
	    //printf("main-generate: Rank 0 sending %d to Rank 1\n", x);
	    start = MPI_Wtime();
            MPI_Isend(&x, 1, MPI_INT,
                      1, 0,
                      MPI_COMM_WORLD, &request);
	    end = MPI_Wtime();
	    elapsed = end - start;
	    isendtimes[i] = elapsed;
	    isendtotaltime += elapsed;
	    //printf("main-generate [%d]: Isend #%d took %.10e seconds\n", world_rank, i, elapsed);
	     
        }
        x =-1; // signal other rank to break out of loop
        MPI_Isend(&x, 1, MPI_INT,
                  1, 0,
                  MPI_COMM_WORLD, &request);
    }
    if (world_rank == 0) {
	/* compute standard deviation */
	isendavg = isendtotaltime/((double) ITERS);
	double sum = 0.0;
	for (i = 0; i < ITERS; i++) 
	    sum += (isendtimes[i] - isendavg)*(isendtimes[i] - isendavg);
	
	sum = sum / ((double) ITERS);
	isendstddev = sqrt(sum);
	printf("isend N: %d\n", ITERS);
	printf("isend totaltime: %.10e\n", isendtotaltime);
	printf("isend avg: %.10e\n", isendavg);
	printf("isend std-dev: %.10e\n", isendstddev);


	
	/* write times to file */
	char* filename = "main-generate-isend-times.txt";
	FILE* fp = fopen(argv[argc-1], "w");
	if (!fp) {
	    printf("Could not open '%s'. Will not write to file\n", argv[argc-1]);
	} else {
	    fprintf(fp, "isend N: %d\n", ITERS);
	    fprintf(fp, "isend totaltime: %.10e\n", isendtotaltime);
	    fprintf(fp, "isend avg: %.10e\n", isendavg);
	    fprintf(fp, "isend std-dev: %.10e\n", isendstddev);
	    fprintf(fp, "----\n");
	    for (i = 0; i < ITERS; i++) {
		fprintf(fp, "%.10e\n", isendtimes[i]);
	    }
	    fclose(fp);
	}
    }
    free(isendtimes);
    MPI_Finalize();
    return 0;
}
