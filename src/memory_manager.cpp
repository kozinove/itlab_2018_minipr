#include "memory_manager.h"
#include <thread>
#include <mpi.h>
#include "parallel_vector.h"

#define GET_DATA_FROM_HELPER 123
// посылка: [номер структуры, откуда требуются данные; требуемый номер элемента, кому требуется переслать объект]
// если номер структуры = -1, то завершение функции helper_thread

memory_manager::memory_manager(int argc, char**argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    number_of_objects = 0;
    helper_thr = std::thread(this->helper_thread);
    // создание своего типа для пересылки посылок ???
}

void memory_manager::create_object(parallel_vector* object, int number_of_elements) {  // parallel_vector*???
    // разделение памяти по процессам?
    number_of_objects++;
    map_pointer_to_int[object] = number_of_objects;
    map_int_to_pointer[number_of_objects] = object;
}

void memory_manager::delete_object(parallel_vector* object) {  // parallel_vector*???
    auto tmp = map_pointer_to_int.find(object);
    if(tmp == map_pointer_to_int.end())
        throw -1;
    auto tmp2 = map_int_to_pointer.find(tmp->second);  // ???
    if(tmp2 == map_int_to_pointer.end())
        throw -2;
    map_int_to_pointer.erase(tmp2);
    map_pointer_to_int.erase(tmp);
}

void memory_manager::helper_thread() {
    int request[3];
    MPI_Status status;
    while(true) {
        // инициализация переменной для принятия-отправок посылок
        MPI_Recv(&request, 3, MPI_INT, 0, GET_DATA_FROM_HELPER, MPI_COMM_WORLD, &status);
        if(request[0] == -1)
            break;
        auto tmp = map_int_to_pointer.find(request[1]);
        if(tmp == map_int_to_pointer.end())
            throw -3;
        int data = (tmp->second)->get_elem(request[1]);
        MPI_Send(&data, 1, MPI_INT, request[2], 0, MPI_COMM_WORLD);
    }
}

int memory_manager::get_data(parallel_vector* object, int index_of_element, int rank, int index_of_proccess) {
    // send to 0, чтобы узнать, какой процесс затребовал элемент?
     if(rank == 0) {
         auto tmp = map_pointer_to_int.find(object);
        if(tmp == map_pointer_to_int.end())
            throw -1;
        int request[3] = {tmp->second, index_of_element, index_of_proccess};
        MPI_Send(request, 3, MPI_INT, rank, GET_DATA_FROM_HELPER, MPI_COMM_WORLD);
     }   
}

memory_manager::~memory_manager() {
    int request[3] = {-1, -1, -1};
    if(rank == 0) {
        for(int i = 0; i < size; i++) {
            MPI_Send(request, 3, MPI_INT, 0, GET_DATA_FROM_HELPER, MPI_COMM_WORLD);
        }
    }
}