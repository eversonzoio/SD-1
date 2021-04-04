#include "mpi.h"
#include <iostream>
#include <random>
#include <algorithm>

using namespace std;

void initiate_ranks(int online_ranks[], int size)
{
    for (int i = 0; i < size; i++)
        online_ranks[i] = -1;
}

void print_ranks(int online_ranks[], int size)
{
    for (int i = 0; i < size; i++)
        cout << online_ranks[i] << " ";
    cout << endl;
}

int main(int argc, char *argv[])
{
    int size, rank;
    int random_rank = 2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // My MPI process number
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Total number of MPI processes

    //cout << random_rank << " and rank " << rank << endl;

    if (rank == size - 1)
    { // if I am the leader (greatest id)
        int status_msg[2];
        // receiving a msg asking if I am online
        MPI_Recv(status_msg, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cout << "I am the leader " << rank << " and I am offline " << endl;

        int my_status = 0; // offline
        // send a msg telling I am offline
        MPI_Send(&my_status, 1, MPI_INT, status_msg[1], 1, MPI_COMM_WORLD);
    }

    else if (rank == random_rank)
    {
        int status_msg[2] = {1, rank};
        int curr_leader = size - 1;

        // asking the leader if it is online
        MPI_Send(&status_msg, 2, MPI_INT, curr_leader, 0, MPI_COMM_WORLD);

        int leader_status;

        // it is offline
        MPI_Recv(&leader_status, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (leader_status == 0)
            cout << "leader " << curr_leader << " has status 0, then it is offline" << endl;

        // starting the election
        int next_rank = 0; // next rank is 0

        int online_ranks[size - 1];
        initiate_ranks(online_ranks, size - 1);
        online_ranks[rank] = rank;

        cout << "rank " << rank << " is starting the election" << endl;
        // send my rank to the next rank
        MPI_Send(online_ranks, size - 1, MPI_INT, next_rank, 2, MPI_COMM_WORLD);

        // wait for the msg to get back
        MPI_Recv(online_ranks, size - 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // election is over

        cout << "all ranks online: " << endl;
        print_ranks(online_ranks, size - 1);

        int new_leader = *max_element(online_ranks, online_ranks + size - 1);

        cout << "election winner: " << new_leader << endl;

        for (int i = 0; i < rank; i++)
            MPI_Send(&new_leader, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
    }

    else
    {
        int online_ranks[size - 1];
        initiate_ranks(online_ranks, size - 1);
        // recv rank from previous rank
        MPI_Recv(online_ranks, size - 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        online_ranks[rank] = rank;

        int next_rank = rank + 1; //(rank + 1) % (size - 2);

        // send to the next rank
        MPI_Send(online_ranks, size - 1, MPI_INT, next_rank, 2, MPI_COMM_WORLD);

        int new_leader;
        // recv the leader
        MPI_Recv(&new_leader, 1, MPI_INT, random_rank, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        cout << "rank " << rank << " received new leader: " << new_leader << endl;
    }

    MPI_Finalize();

    return 0;
}