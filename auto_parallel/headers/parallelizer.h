#ifndef __PARALLELIZER_H__
#define __PARALLELIZER_H__

#include "task_graph.h"
#include <vector>
#include <queue>
#include <map>
#include "mpi.h"

namespace auto_parallel
{

    class parallelizer
    {
    private:
        // для выборки инструкции на принимающей стороне
        struct instruction
        {
            int n[2];
        };
        // описатель данных
        struct d_info
        {
            message* d;
            int version;
        };
        // описатель задачи
        struct t_info
        {
            task* t;
            int parents; // кол-во предков, когда 0 - можно ставить в очередь
            std::vector<int> childs; // вектор id дочерних задач. после завершения задачи у всех из этого вектора уменьшается поле parent
            std::vector<int> data_id; // вектор с id используемых данных соответствует по размеру такому-же вектору из класса task
            std::vector<int> const_data_id; // вектор с id не изменяемых используемых данных
        };

        int proc_id;
        int proc_size;

        std::queue<int> ready_tasks; // очередь с id готовых к исполнению задач
        std::vector<t_info> task_v; // вектор задач id задачи - позиция в этом векторе
        std::vector<d_info> data_v; // вектор данных id данных - позиция в этом векторе

        void master(); // основной процесс всегда есть хотя-бы 1
        void worker(); // может и не быть
        // поставить на выполнению задачу по id из вектора
        void execute_task(int task_id);
        void control_task(int task_id, int proc);
        void wait_proc(int task_id, int proc);
        // посылки различных данных
        void send_instruction(int type, int proc, int info = 0);
        instruction recv_instruction(int proc);

        void send_ver_of_data(int did, int proc);
        int recv_ver_of_data(int did, int proc);

        void send_data(int did, int proc);
        void recv_data(int did, int proc);

    public:

        const static int main_proc; // просто 0

        parallelizer(int* argc = NULL, char*** argv = NULL);
        parallelizer(const task_graph& _tg, int* argc = NULL, char*** argv = NULL);
        ~parallelizer();

        void init(const task_graph& _tg); // если был вызван конструктор без графа то нужно вызвать эту функцию
        void execution(); // запуск всего выполнения
    };

}

#endif // __PARALLELIZER_H__
