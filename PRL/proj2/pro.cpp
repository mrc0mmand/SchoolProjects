/**
 * @author Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 * @date 25.4.2018
 * @file pro.cpp
 */
#include <iostream>
#include <map>
#include <mpi.h>
#include <vector>

#define ROOT_ID 0

#define PRINT_EDGE(x) \
    cout << (char)(x)->from << " -> " << (char)(x)->to << " (" << (x)->reversed << ")\n";

typedef struct {
    int from;
    int to;
    int reversed;
} t_edge;

typedef struct t_adj {
    int from;
    int to;
    struct t_adj *next;
} t_adj;

using namespace std;

int main(int argc, char *argv[])
{
    int cpu_count;
    int my_id;
    int my_weight;
    int my_rank;
    int my_pos;
    int my_ssum;
    int next_edge_idx;
    t_edge *my_edge;
    map<int, t_adj*> adj_list;
    vector<t_edge*> edges;
    vector<int> etour;
    vector<int> ranks;
    vector<int> weights;
    vector<int> preorder;
    MPI::Status stat;

    MPI::Init();
    cpu_count = MPI::COMM_WORLD.Get_size();
    my_id = MPI::COMM_WORLD.Get_rank();

    if(argc < 2) {
        if(my_id == ROOT_ID) {
            cerr << "Missing argument: nodes" << endl;
        }

        exit(EXIT_FAILURE);
    }

    if(2 * strlen(argv[1]) - 2 != cpu_count) {
        if(my_id == ROOT_ID) {
            cerr << "Number of nodes != number of CPUs" << endl;
        }

        exit(EXIT_FAILURE);
    }

    // 1) Create adjacency list
    // 2) Euler tour
    // 3) Calculate weights (suffix sum)
    // 4) Preorder Euler tour
    char *nodes = argv[1];

    // Structure:
    // node = x (counted from 1)
    // left subtree = 2x
    // right subtree = 2x + 1
    unsigned int nlength = strlen(nodes);
    for(unsigned int i = 1; i <= nlength; i++) {
        //cout << nodes[i - 1] << endl;
        if(2 * i <= nlength) {
            // Left subtree
            t_edge *f_edge = new t_edge{nodes[i - 1], nodes[2 * i - 1], 0};
            t_edge *r_edge = new t_edge{nodes[2 * i - 1], nodes[i - 1], 1};
            edges.push_back(f_edge);
            edges.push_back(r_edge);
        }

        if((2 * i + 1) <= nlength) {
            // Right subtree
            t_edge *f_edge = new t_edge{nodes[i - 1], nodes[2 * i], 0};
            t_edge *r_edge = new t_edge{nodes[2 * i], nodes[i - 1], 1};
            edges.push_back(f_edge);
            edges.push_back(r_edge);
        }
    }

    // Adjacency list
    for(unsigned int i = 0; i < edges.size(); i++) {
        t_adj *adj;
        int nfrom  = edges[i]->from;

        if(adj_list.find(nfrom) == adj_list.end()) {
            adj_list[nfrom] = new t_adj;
            adj_list[nfrom]->from = i;
            adj_list[nfrom]->to = i + 1;
            adj_list[nfrom]->next = 0;
        } else {
            adj = adj_list[nfrom];
            while(adj->next != 0)
                adj = adj->next;

            adj->next = new t_adj;
            adj = adj->next;
            adj->from = i;
            adj->to = i + 1;
            adj->next = 0;
        }
    }

    // Assign edges
    my_edge = edges[my_id];
    MPI::COMM_WORLD.Barrier();

    // Euler tour
    t_adj *adj = adj_list[my_edge->to];
    while(adj != 0) {
        if(my_edge->to == edges[adj->from]->from && my_edge->from == edges[adj->from]->to) {
            if(adj->next != 0)
                next_edge_idx = adj->next->from;
            else
                next_edge_idx = adj_list[my_edge->to]->from;
        }

        adj = adj->next;
    }

    MPI::COMM_WORLD.Barrier();

    // Gather edges for Euler tour from all CPUs
    if(my_id == ROOT_ID) {
        int recvn;
        int idx;
        int idxpre;
        int rank = edges.size() - 1;
        etour.resize(cpu_count);
        ranks.resize(cpu_count);
        etour[0] = next_edge_idx;

        for(int i = 1; i < cpu_count; i++) {
            MPI::COMM_WORLD.Recv(&recvn, 1, MPI_INT, i, 0);
            // Assign Etour(e) = e for edge from node to root
            if(recvn == 0)
                etour[i] = i;
            else
                etour[i] = recvn;
        }

        // Non-parallel list ranking
        idx = 0;
        idxpre = -1;

        while(idx != idxpre) {
            ranks[idx] = rank--;
            idxpre = idx;
            idx = etour[idx];
        }

        for(int i = 1; i < cpu_count; i++)
            MPI::COMM_WORLD.Send(&ranks[i], 1, MPI_INT, i, 0);

        my_rank = ranks[0];
    } else {
        MPI::COMM_WORLD.Send(&next_edge_idx, 1, MPI_INT, ROOT_ID, 0);
        MPI::COMM_WORLD.Recv(&my_rank, 1, MPI_INT, ROOT_ID, 0);
    }

    if(my_edge->reversed == 0)
        my_weight = 1;
    else
        my_weight = 0;

    my_pos = edges.size() - my_rank;

    MPI::COMM_WORLD.Barrier();

    weights.resize(cpu_count);

    // Sort weights to calculate suffixsum
    if(my_id == ROOT_ID) {
        weights[my_pos - 1] = my_weight;

        // Accumulate weights and insert them to their right positions
        int args[2];
        for(int i = 1; i < cpu_count; i++) {
            MPI::COMM_WORLD.Recv(args, 2, MPI_INT, i, 0);
            weights[args[0] - 1] = args[1];
        }

        // Broadcast the 'sorted' weights array
        MPI::COMM_WORLD.Bcast(&weights[0], cpu_count, MPI_INT, ROOT_ID);


    } else {
        int args[2] = {my_pos, my_weight};
        MPI::COMM_WORLD.Send(args, 2, MPI_INT, ROOT_ID, 0);
        MPI::COMM_WORLD.Bcast(&weights[0], cpu_count, MPI_INT, ROOT_ID);
    }

    MPI::COMM_WORLD.Barrier();

    // Suffix sum
    my_ssum = 0;
    for(int i = my_pos - 1; i < cpu_count; i++)
        my_ssum += weights[i];

    // Preorder
    if(my_id == ROOT_ID) {
        preorder.resize(strlen(nodes));
        // Insert root
        preorder[0] = my_edge->from;
        preorder[1] = my_edge->to;

        int args[2];
        for(int i = 1; i < cpu_count; i++) {
            MPI::COMM_WORLD.Recv(args, 2, MPI_INT, i, 0);
            if(args[0] != -1)
                preorder[args[1] - 1] = args[0];
        }

        // Print the preorder euler tour
        for(auto x : preorder)
            cout << (char)x;

        cout << endl;

    } else {
        // If e is a forward edge, then preorder(v) = n - weight(e) + 1
        int args[2] = {-1, -1};
        if(my_edge->reversed == 0) {
            args[0] = my_edge->to;
            args[1] = strlen(nodes) - my_ssum + 1;
        }

        MPI::COMM_WORLD.Send(args, 2, MPI_INT, ROOT_ID, 0);
    }

    // Cleanup
    for(auto x : edges)
        delete x;

    for(auto x : adj_list) {
        t_adj *next = 0;
        t_adj *adj = x.second;

        while(adj != 0) {
            next = adj->next;
            delete adj;
            adj = next;
        }
    }

    MPI::Finalize();

    return 0;
}
