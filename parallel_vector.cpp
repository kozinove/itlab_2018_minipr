#include <iostream>
#include <vector>
#include <mpi.h>

using namespace std;

class parallel_vector {
    vector<int>v;
    int sizeproc, rankproc;
    int portion;
    int allsize;
public:
    parallel_vector(int size) {
        MPI_Comm_size(MPI_COMM_WORLD, &sizeproc);
        MPI_Comm_rank(MPI_COMM_WORLD, &rankproc);
        portion = size/sizeproc;
        if(rankproc < size%sizeproc)
            portion++;
        allsize = size;
        v.resize(portion);
    }
    int get_elem(int index) {
        if(index < 0 || index >= allsize)
            throw -1;
        int numberproc = get_number_of_proccess(index), numberelem = get_number_of_element(index);
        int ans;
        if(rankproc == numberproc)
            ans = v[numberelem];
        MPI_Bcast(&ans, 1, MPI_INT, numberproc, MPI_COMM_WORLD);
        return ans;
    }

    void set_elem(int index, int value) {
        if(index < 0 || index >= allsize)
            throw -1;
        int number_proc = get_number_of_proccess(index), number_elem = get_number_of_element(index);
        if(rankproc == number_proc) {
            v[number_elem] = value;
        }
    }

private:
    int get_number_of_proccess(int index) {
        int number_proc;
        if(index < (allsize%sizeproc)*(allsize/sizeproc+1)) {
            number_proc = index/(allsize/sizeproc+1);
        } else {
            int tmp = index - (allsize%sizeproc)*(allsize/sizeproc+1);
            number_proc = tmp/(allsize/sizeproc) + (allsize%sizeproc);
        }
        return number_proc;
    }
    int get_number_of_element(int index) {
        int number_elem;
        if(index < (allsize%sizeproc)*(allsize/sizeproc+1)) {
            number_elem = index%(allsize/sizeproc+1);
        } else {
            int tmp = index - (allsize%sizeproc)*(allsize/sizeproc+1);
            number_elem = tmp%(allsize/sizeproc);
        }
        return number_elem;
    }
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int n = 100;
    parallel_vector pv(n);
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    for(int i = 0; i < n; i++) {
        pv.set_elem(i, i*i);
    }
    for(int i = 0; i < n; i++) {
        int tmp = pv.get_elem(i);
        if(rank == 0)
            cout<<tmp<<"\n";
    }
    return 0;
}