#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ping 1
#define pong 0

int main(int argc, char** argv) {
    int my_rank;
    int world_size;
    MPI_Request request;

    const unsigned PING_PONG_LIMIT = 40;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int ping_pong_count = 0;
    int partner_rank = (my_rank + 1) % world_size;

    int i,j;
    int p; // ping pong iter
    int buffer =0;
    printf("[%d]: before non-root ranks loops\n", my_rank);
    if (my_rank != 0) {
	MPI_Status status;
	for (p = 0; p < PING_PONG_LIMIT; p++) {
	    MPI_Recv(&buffer, 1, MPI_INT, 0, ping, MPI_COMM_WORLD, &status);
	    buffer++;
	    MPI_Send(&buffer, 1, MPI_INT, 0, pong, MPI_COMM_WORLD);
	    printf("[%d]: buffer is now %d\n", my_rank, buffer);
	}
    }

    printf("[%d]: before root rank loop\n", my_rank);
    if (my_rank == 0) {
	MPI_Status status;
	for (i = 1; i < world_size; i++) {
	    buffer++;
	    MPI_Send(&buffer, 1, MPI_INT, i, ping, MPI_COMM_WORLD);
	    MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, pong, MPI_COMM_WORLD, &status);
	    printf("[%d]: buffer is now %d\n", my_rank, buffer);
	}
    } 
    

    //while (ping_pong_count < PING_PONG_LIMIT) {
    //	if (my_rank == ping_pong_count % world_size) {
    //	    // Increment the ping pong count before you send it
    //	    ping_pong_count++;
    //	    MPI_Send(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);
    //	    	printf("[%d]: Send to [%d]: ping_pong_count: %d -> %d\n",
    //	    	       my_rank, partner_rank, ping_pong_count, partner_rank);
    //	} else {
    //	    MPI_Recv(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD,
    //		     MPI_STATUS_IGNORE);
    //	    	printf("[%d]: Recv from [%d]: ping_pong_count: %d\n",
    //	    	       my_rank, partner_rank, ping_pong_count);
    //	}
    //}




    MPI_Finalize();
    return 0;
}
