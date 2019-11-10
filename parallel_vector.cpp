#include <iostream>
#include <vector>
#include <mpi.h>
#include "parallel_vector.h"

parallel_vector::parallel_vector() {
    MPI_Comm_size(MPI_COMM_WORLD, &sizeproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankproc);
    portion = 1;
    allsize = sizeproc;
    v.resize(portion);
}

parallel_vector::parallel_vector(const int& size) {
    MPI_Comm_size(MPI_COMM_WORLD, &sizeproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankproc);
    portion = size/sizeproc;
    if(rankproc < size%sizeproc)
        portion++;
    allsize = size;
    v.resize(portion);
}

parallel_vector::parallel_vector(const parallel_vector& pv) {
    portion = pv.portion;
    rankproc = pv.rankproc;
    sizeproc = pv.sizeproc;
    allsize = pv.allsize;
    v.resize(pv.v.size());
    for(int i = 0; i < portion; i++) {
        v[i] = pv.v[i];
    }
}

parallel_vector& parallel_vector::operator=(const parallel_vector& pv) {
    if(this != &pv) {
        portion = pv.portion;
        rankproc = pv.rankproc;
        sizeproc = pv.sizeproc;
        allsize = pv.allsize;
        v.resize(pv.v.size());
        for(int i = 0; i < portion; i++) {
            v[i] = pv.v[i];
        }
    }
    return *this;
}

int parallel_vector::get_elem(const int& index) const {
    if(index < 0 || index >= allsize)
        throw -1;
    int number_of_proccess = get_index_of_proccess(index), numberelem = get_index_of_element(index);
    int ans;
    if(rankproc == number_of_proccess)
        ans = v[numberelem];
    MPI_Bcast(&ans, 1, MPI_INT, number_of_proccess, MPI_COMM_WORLD);
    return ans;
}

void parallel_vector::set_elem(const int& index, const int& value) {
    if(index < 0 || index >= allsize)
        throw -1;
    int number_proc = get_index_of_proccess(index), number_elem = get_index_of_element(index);
    if(rankproc == number_proc) {
        v[number_elem] = value;
    }
}

int parallel_vector::get_elem_proc(const int& index) const { // WARNING!
    if(index < 0 || index >= portion)
         throw -1;
    return v[index];
}

void parallel_vector::set_elem_proc(const int& index, const int& value) { // WARNING!
    if(index < 0 || index >= portion)
         throw -1;
    v[index] = value;
}

int parallel_vector::get_portion() const {
    return portion;
}

int parallel_vector::get_index_of_proccess(const int& index) const {
    int number_proc;
    if(index < (allsize%sizeproc)*(allsize/sizeproc+1)) {
        number_proc = index/(allsize/sizeproc+1);
    } else {
        int tmp = index - (allsize%sizeproc)*(allsize/sizeproc+1);
        number_proc = tmp/(allsize/sizeproc) + (allsize%sizeproc);
    }
    return number_proc;
}

int parallel_vector::get_index_of_element(const int& index) const {
    int number_elem;
    if(index < (allsize%sizeproc)*(allsize/sizeproc+1)) {
        number_elem = index%(allsize/sizeproc+1);
    } else {
        int tmp = index - (allsize%sizeproc)*(allsize/sizeproc+1);
        number_elem = tmp%(allsize/sizeproc);
    }
    return number_elem;
}
