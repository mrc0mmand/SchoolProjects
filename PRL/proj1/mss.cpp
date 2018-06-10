/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @file mss.cpp
 * @date 7.4.2018
 */

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <vector>

#define PAD_NUMBER (-1)
#define NUM_FILE "numbers"
#define ROOT_ID 0
#define TAG 0

using namespace std;

void dump_vector(const std::vector<int> &vec, char sep=' ')
{
    for(const auto &x : vec)
        cout << x << sep;
}

int main(int argc, char *argv[])
{
    int cpu_count;
    int my_id;
    int max_num;
    vector<int> numbers;
    vector<int> my_numbers;
    vector<int> sorted;
    MPI::Status stat;

    MPI::Init();
    cpu_count = MPI::COMM_WORLD.Get_size();
    my_id = MPI::COMM_WORLD.Get_rank();

    // Root CPU (rank 0) - load numbers and distribute them to other CPUs
    if(my_id == ROOT_ID) {
        int num;
        int num_per_cpu;
        ifstream fin(NUM_FILE);

        while(!fin.eof()) {
            num = fin.get();
            // Ignore EOF
            if(fin.eof()) break;
            numbers.push_back(num);
        }

        fin.close();

        // Print loaded numbers
        dump_vector(numbers);
        cout << endl;

        // Pad the vector with PAD_NUMBER
        // This is a lazy workaround for the uneven distribution issue
        // where size of the numbers array is not divisible by CPU count
        for(int i = 0; i < numbers.size() % cpu_count; i++)
            numbers.insert(numbers.begin(), PAD_NUMBER);

        // Broadcast maximum number of numbers per CPU
        num_per_cpu = ceil(numbers.size() / float(cpu_count));
        MPI::COMM_WORLD.Bcast(&num_per_cpu, 1, MPI_INT, ROOT_ID);
        max_num = num_per_cpu;
    } else {
        // Receive broadcasted max. number of numbers per CPU
        MPI::COMM_WORLD.Bcast(&max_num, 1, MPI_INT, ROOT_ID);
    }

    // Distribute numbers across CPUs
    my_numbers.resize(max_num);
    MPI::COMM_WORLD.Scatter(&numbers[0], max_num, MPI_INT,
            &my_numbers[0], max_num, MPI_INT, ROOT_ID);

    // Each CPU sorts its sequence
    sort(my_numbers.begin(), my_numbers.end());

    MPI::COMM_WORLD.Barrier();

    for(int steps = 0; steps < ceil(cpu_count / 2.0); steps++) {
        for(int oddeven = 0; oddeven < 2; oddeven++) {
            if(my_id % 2 == oddeven % 2 && my_id + 1 < cpu_count) {
                vector<int> merge(my_numbers);
                vector<int> recv;
                recv.resize(max_num);

                // Get numbers from the neighbor (slave) CPU
                MPI::COMM_WORLD.Recv(&recv[0], max_num, MPI_INT,
                        my_id + 1, TAG, stat);

                // Merge and sort the sequence
                merge.insert(merge.end(), recv.begin(), recv.end());
                sort(merge.begin(), merge.end());

                // Split and distribute the sorted sequence
                my_numbers.assign(merge.begin(), merge.begin() + max_num);
                MPI::COMM_WORLD.Send(&merge[max_num], max_num, MPI_INT,
                        my_id + 1, TAG);
            } else if(my_id % 2 == (oddeven + 1) % 2 && my_id - 1 >= 0) {
                // Send numbers to the neighbor (master) CPU
                MPI::COMM_WORLD.Send(&my_numbers[0], my_numbers.size(),
                        MPI_INT, my_id - 1, TAG);
                // Get sorted and split part back
                MPI::COMM_WORLD.Recv(&my_numbers[0], max_num, MPI_INT,
                        my_id - 1, TAG);
            }

            MPI::COMM_WORLD.Barrier();
        }
    }

    if(my_id == ROOT_ID) {
        sorted.resize(numbers.size());
    }

    MPI::COMM_WORLD.Barrier();
    MPI::COMM_WORLD.Gather(&my_numbers[0], my_numbers.size(), MPI_INT,
            &sorted[0], max_num, MPI_INT, ROOT_ID);

    // 'Unpad' the number vector
    sorted.erase(remove(sorted.begin(), sorted.end(), PAD_NUMBER),
            sorted.end());
    dump_vector(sorted, '\n');

    MPI::Finalize();

    return 0;
}
