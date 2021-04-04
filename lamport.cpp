#include "mpi.h"
#include <iostream>
#include <random>
#include <vector>
#include <array>
#include <chrono>
#include <algorithm>
#include <unistd.h>

#define DATA rand() % 100

using namespace std;

const int MAX_ITERATIONS = 10;
const int SLEEP_TIME_US = 1000000;


void do_stuff(int &clock)
{
    if (rand() % 2 == 0)
        clock++;
}

array<int, MAX_ITERATIONS> create_and_fill_odd()
{
    array<int, MAX_ITERATIONS> dests{1, 3, 1, 3, 1, 3, 1, 3, 1, 3};
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(dests.begin(), dests.end(), default_random_engine(seed));
    return dests;
}

array<int, MAX_ITERATIONS> create_and_fill_even()
{
    array<int, MAX_ITERATIONS> dests{0, 2, 0, 2, 0, 2, 0, 2, 0, 2};
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(dests.begin(), dests.end(), default_random_engine(seed));
    return dests;
}

int main(int argc, char *argv[])
{
    int size, rank;
    int clock = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // My MPI process number
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Total number of MPI processes

    if (rank % 2 == 0)
    {
        // first seding data
        auto dests = create_and_fill_odd();
        for (int i = 0; i < MAX_ITERATIONS; i++)
        {
            do_stuff(clock);
            clock++; // increment clock anyway
            int data[2] = {DATA, clock};

            MPI_Send(&data, 2, MPI_INT, dests[i], 0, MPI_COMM_WORLD);

            cout << "rank " << rank << " sent " << data[0] << " and clock " << data[1] << " to " << dests[i] << endl;
        }

        usleep(SLEEP_TIME_US);

        // receiveing data
        for (int i = 0; i < MAX_ITERATIONS; i++)
        {
            clock++;
            int data[2];

            MPI_Recv(&data, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            cout << "rank " << rank << " received " << data[0] << " and clock " << data[1] << endl;

            cout << "rank " << rank << " had clock " << clock;

            clock = max(clock, data[1]) + 1;

            cout << " and now has clock " << clock << endl;
        }
    }

    else
    {
        // first receiveing data
        for (int i = 0; i < MAX_ITERATIONS; i++)
        {
            clock++;
            int data[2];

            MPI_Recv(&data, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            cout << "rank " << rank << " received " << data[0] << " and clock " << data[1] << endl;

            cout << "rank " << rank << " had clock " << clock;

            clock = max(clock, data[1]) + 1;

            cout << " and now has clock " << clock << endl;
        }

        usleep(SLEEP_TIME_US * 0.1);

        // seding data
        auto dests = create_and_fill_even();
        for (int i = 0; i < MAX_ITERATIONS; i++)
        {
            do_stuff(clock);
            clock++; // increment clock anyway
            int data[2] = {DATA, clock};

            MPI_Send(&data, 2, MPI_INT, dests[i], 0, MPI_COMM_WORLD);

            cout << "rank " << rank << " sent " << data << " to " << dests[i] << endl;
        }
    }

    MPI_Finalize();

    return 0;
}
