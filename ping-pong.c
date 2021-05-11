#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int my_rank;
    int world_size;
    MPI_Request request;

    const unsigned PING_PONG_LIMIT = 100000;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int ping_pong_count = 0;
    int partner_rank = (my_rank + 1) % 2;
    while (ping_pong_count < PING_PONG_LIMIT) {
	if (my_rank == ping_pong_count % 2) {
	    // Increment the ping pong count before you send it
	    ping_pong_count++;
	    MPI_Send(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);
	    //if ((ping_pong_count %100) == 0)
	    //	printf("[%d]: Send to [%d]: ping_pong_count: %d -> %d\n",
	    //	       my_rank, partner_rank, ping_pong_count, partner_rank);
	} else {
	    MPI_Recv(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD,
		     MPI_STATUS_IGNORE);
	    //if ((ping_pong_count %101) == 0)
	    //	printf("[%d]: Recv from [%d]: ping_pong_count: %d\n",
	    //	       my_rank, partner_rank, ping_pong_count);
	}
    }
    MPI_Finalize();
}
