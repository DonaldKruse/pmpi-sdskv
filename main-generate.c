#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int main(int argc, char** argv) {
    int world_rank;
    int world_size;
    MPI_Request request;
    double start, end, elapsed;

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
        for (i = 0; i < 25; i++) {
            x = 2*i + 1;
	    //printf("main-generate: Rank 0 sending %d to Rank 1\n", x);
	    start = MPI_Wtime();
            MPI_Isend(&x, 1, MPI_INT,
                      1, 0,
                      MPI_COMM_WORLD, &request);
	    end = MPI_Wtime();
	    elapsed = end - start;
	    printf("main-generate [%d]: Isend #%d took %lf seconds\n", world_rank, i, elapsed);
	     
        }
        x =-1; // signal other rank to break out of loop
        MPI_Isend(&x, 1, MPI_INT,
                  1, 0,
                  MPI_COMM_WORLD, &request);
    }

    MPI_Finalize();
    return 0;
}
