#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__

#include <map>
#include <thread>
#include "parallel_vector.h"

class memory_manager {
    std::map<int, parallel_vector*> map_int_to_pointer;
    std::map<parallel_vector*, int> map_pointer_to_int;
    int number_of_objects;
    std::thread helper_thr;
    int rank, size;
public:
    memory_manager(int argc, char** argv);
    int get_data(parallel_vector* object, int index_of_element, int rank, int index_of_proccess); //int*???
    void create_object(parallel_vector* object, int number_of_elements);
    void delete_object(parallel_vector* object);
    // int get_number_by_pointer(parallel_vector* pointer);
    // parallel_vector* get_pointer_by_number(int number);
    ~memory_manager();
    void helper_thread();
};

void init(int argc, char** argv);

#endif  // __MEMORY_MANAGER_H__