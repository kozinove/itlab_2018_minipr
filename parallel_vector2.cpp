#include <iostream>
#include <vector>
#include <mpi.h>

using namespace std;

class coarray_vector {
    vector<int>v;
    int sizeproc, rankproc;
public:
    coarray_vector(int size) {
        MPI_Comm_size(MPI_COMM_WORLD, &sizeproc);
        MPI_Comm_rank(MPI_COMM_WORLD, &rankproc);
        v.resize(size);
    }
    int get_elem(int index, int number) {
        int ans = (rankproc == index?v[number]:0);
        MPI_Bcast(&ans, 1, MPI_INT, index, MPI_COMM_WORLD);
    }
    void set_elem(int index, int number, int value) {
        if(index == rankproc)
            v[number] = value;
    }
    void push_back(int index, int value) {
        if(index == rankproc)
            v.push_back(value);
    }
    void pop_back(int index) {
        if(index == rankproc)
            v.pop_back();
    }
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    return 0;
}