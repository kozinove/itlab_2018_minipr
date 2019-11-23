#include <iostream>
#include <mpi.h>
#include "parallel_vector.h"
#include "parallel_reduce.h"
#include "parallel_for.h"


class Func {
public:
    parallel_vector* a;
    std::vector<int>* b;
    Func(parallel_vector& pv, std::vector<int>&v):a(&pv), b(&v) {}
    int operator()(int l, int r, int identity)  const {
        int ans = identity;
        for(int i = l; i < r; i++) {
            int rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            ans+=a->get_elem_proc(i)*(*b)[a->get_reverse_index_of_element(i, rank)%b->size()];
        }
        return ans;
    }  
};

class Reduction {
    int a, b;
public:
    Reduction(): a(0), b(0) {}
    Reduction(int aa, int bb): a(aa), b(bb) {}
    int operator()(int a, int b) const {
        return a+b;
    }
};


int main(int argc, char** argv) { // b*a
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int n = 10, m = 10;
    parallel_vector a(n*m);
    //parallel_vector b(n);
    std::vector<int>b(n);
    parallel_vector ans(m);
    for(int i = 0; i < n*m; i++) {
        a.set_elem(i, i);
    }
    for(int i = 0; i < n; i++) {
        // b.set_elem(i, i);
        b[i] = i;
    }
    for(int i = 0; i < m; i++) {
        int anss = parallel_reduce(i*n, (i+1)*n, a, 0, Func(a, b), Reduction());
        int proccess = ans.get_index_of_proccess(i);
        if(proccess > 0) {
            if(rank == 0) {
                MPI_Request request;
                MPI_Isend(&anss, 1, MPI_INT, proccess, 0, MPI_COMM_WORLD, &request);
            }
            else if(rank == proccess) {
                MPI_Status status;
                MPI_Recv(&anss, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
        }
        ans.set_elem(i, anss);
    }
    // for(int i = 0; i < n*m; i++) {
    //     int anss = a.get_elem(i);
    //     if(rank == 0) 
    //         std::cout<<a.get_index_of_proccess(i)<<": "<<i<<" ("<<a.get_index_of_element(i)<<") - "<<anss<<"\n";
    // }
    for(int i = 0; i < m; i++) {
        int anss = ans.get_elem(i);
        if(rank == 0) {
            std::cout<<anss<<" ";
        }
    }
    if(rank == 0)
        std::cout<<"\n";
    // if(rank == 0) {
    //     std::vector<int>a(n*m);
    //     std::vector<int>b(n);
    //     std::vector<int>ans(m);
    //     for(int i = 0; i < n*m; i++)
    //         a[i] = i;
    //     for(int i = 0; i < n; i++)
    //         b[i] = i;
    //     for(int j = 0; j < m; j++) {
    //         ans[j] = 0;
    //         for(int i = 0; i < n; i++) {
    //             ans[j] += a[j*n+i]*b[i]; 
    //         }
    //     }
    //     for(int i = 0; i < m; i++) {
    //         std::cout<<ans[i]<<" ";
    //     }
    //     std::cout<<"\n";
    // }
    MPI_Finalize();
    return 0;
}