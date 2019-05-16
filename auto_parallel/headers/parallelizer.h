#ifndef __PARALLELIZER_H__
#define __PARALLELIZER_H__

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <iostream>
#include "mpi.h"
#include "task_graph.h"
#include "it_queue.h"

namespace auto_parallel
{

    class parallelizer
    {
    private:
        // ��� ������� ���������� �� ����������� �������
        struct instruction
        {
            int n[2];
        };
        // ��������� ������
        struct d_info
        {
            message* d;
            int version;
        };
        // ��������� ������
        struct t_info
        {
            task* t;
            int parents; // ���-�� �������, ����� 0 - ����� ������� � �������
            std::vector<int> childs; // ������ id �������� �����. ����� ���������� ������ � ���� �� ����� ������� ����������� ���� parent
            std::vector<int> data_id; // ������ � id ������������ ������ ������������� �� ������� ������-�� ������� �� ������ task
            std::vector<int> const_data_id; // ������ � id �� ���������� ������������ ������
        };

        int proc_id;
        int proc_size;

        it_queue<int> ready_tasks; // ������� � id ������� � ���������� �����
        std::vector<t_info> task_v; // ������ ����� id ������ - ������� � ���� �������
        std::vector<d_info> data_v; // ������ ������ id ������ - ������� � ���� �������
        std::vector<int> top_versions;

        void master(); // �������� ������� ������ ���� ����-�� 1
        void worker(); // ����� � �� ����
        // ��������� �� ���������� ������ �� id �� �������
        void execute_task(int task_id);
        // �������� ������ �������� ��������
        void control_task(int task_id, int proc, std::vector<std::set<int>>& v);
        // �������� ���������� ������ �������
        void wait_proc(int task_id, int proc, std::vector<std::set<int>>& v);
        // ������� ��������� ������
        void send_instruction(int type, int proc, int info = 0);
        instruction recv_instruction(int proc);

        void send_ver_of_data(int did, int proc);
        int recv_ver_of_data(int did, int proc);

        void send_data(int did, int proc);
        void recv_data(int did, int proc);

    public:

        const static int main_proc; // ������ 0

        parallelizer(int* argc = NULL, char*** argv = NULL);
        parallelizer(const task_graph& _tg, int* argc = NULL, char*** argv = NULL);
        ~parallelizer();

        void init(const task_graph& _tg); // ���� ��� ������ ����������� ��� ����� �� ����� ������� ��� �������
        void execution(); // ������ ����� ����������
    };

}

#endif // __PARALLELIZER_H__
