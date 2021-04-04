#include "mpi.h"
#include <iostream>
#include <queue>
#include <vector>
#include <array>
#include <chrono>
#include <algorithm>
#include <random>
// mpirun -n 4 -H localhost,slave01,slave02 -oversubscribe ./file
using namespace std;

const int MAX_ITERATIONS = 12;
const int SLEEP_TIME_US = 1000000;

int main(int argc, char *argv[])
{
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // My MPI process number
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Total number of MPI processes

    if (rank == 0)
    {
        int resources_data[2] = {0, 1};
        bool is_locked[2] = {false, false};
        int recv_buffer[3];
        vector<queue<int>> queues(2);

        for (int i = 0; i < MAX_ITERATIONS; i++)
        {
            MPI_Recv(recv_buffer, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (recv_buffer[0] == 1)
            { // lock request
                cout << "rank " << recv_buffer[1] << " requested resource " << recv_buffer[2] << endl;
                if (!is_locked[recv_buffer[2]])
                { // if it is not locked
                    is_locked[recv_buffer[2]] = true;
                    int data = resources_data[recv_buffer[2]];
                    MPI_Send(&data, 1, MPI_INT, recv_buffer[1], 0, MPI_COMM_WORLD);
                    cout << "resource " << recv_buffer[2] << " sent to " << recv_buffer[1] << endl;
                }
                else
                { // enqueue the process
                    queues[recv_buffer[2]].push(recv_buffer[1]);
                    cout << "rank " << recv_buffer[1] << " has to wait to get the resource " << recv_buffer[2] << endl;
                }
            }
            else if (recv_buffer[0] == 0)
            { // unlock request
                if (!queues[recv_buffer[2]].empty())
                { // if there are processes waiting (queue is not empty)
                    int process_waiting = queues[recv_buffer[2]].front();
                    queues[recv_buffer[2]].pop();

                    int data = resources_data[recv_buffer[2]];

                    MPI_Send(&data, 1, MPI_INT, process_waiting, 0, MPI_COMM_WORLD);
                    cout << "resource " << recv_buffer[2] << " sent to " << recv_buffer[1] << " after waiting for it" << endl;
                }
                else
                { // the resource is now available
                    is_locked[recv_buffer[2]] = false;
                    cout << "rank " << recv_buffer[1] << " has released the resource " << recv_buffer[2] << endl;
                }
            }
        }
    }

    else
    {
        array<int, MAX_ITERATIONS / 2> resources_numbers{0, 1, 0, 1, 0, 1};
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(resources_numbers.begin(), resources_numbers.end(), default_random_engine(seed));

        queue<int> requested_resources;

        for (int i = 0; i < MAX_ITERATIONS / 2; i++)
        {
            int lock_flag = rand() % 2 == 0 ? 0 : 1;

            if (lock_flag)
            { // requesting resource, if odd
                int request[3] = {lock_flag, rank, resources_numbers[i]};

                int resource_received;

                MPI_Send(request, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);

                MPI_Recv(&resource_received, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                requested_resources.push(resource_received);
            }

            else if (!requested_resources.empty())
            { // releasing resource, if even
                int resource = requested_resources.front();
                requested_resources.pop();

                int request[3] = {lock_flag, rank, resource};

                MPI_Send(request, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Finalize();

    return 0;
}