#ifndef __TASK_GRAPH_H__
#define __TASK_GRAPH_H__

#include "basic_task.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>

namespace auto_parallel
{

    class task_graph
    {
    protected:
        // при добавлении нового объекта присваивают ему уникальный id обеспечива€ корректность данных на всех процессах
        int base_task_id;
        int base_data_id;
        // описывает данные внутри графа
        struct d_id
        {
            const int id;
            int ref_count;
            d_id(const int nid = 0): id(nid)
            {
                ref_count = 0;
            }
        };
        // описывает задачи внутри графа
        struct t_id
        {
            const int id;
            std::set<task*> childs;
            std::set<task*> parents;
            t_id(const int nid = 0): id(nid)
            {

            }
        };
        // здесь хранитс€ граф задач и данных
        std::map<task*, t_id> t_map;
        std::map<message*, d_id> d_map;

    public:
        task_graph();
        task_graph(const task_graph& _tg);
        task_graph& operator =(const task_graph& _tg);
        // добавл€ет в граф только уникальные экземпл€ры объектов
        void add_task(task* t); // добавл€ет также все данные
        void add_task(task& t); // добавл€ет также все данные
        void add_data(message* m); // лучше не пользоватьс€
        void add_data(message& m); // лучше не пользоватьс€
        void add_dependence(task* parent, task* child);
        void add_dependence(task& parent, task& child);
        // удал€ет из графа + стирает все зависимости
        void del_task(task* t); // может удалить не нужные данные
        void del_task(task& t); // может удалить не нужные данные
        void del_data(message* m); // лучше не пользоватьс€
        void del_data(message& m); // лучше не пользоватьс€
        void del_dependence(task* parent, task* child);
        void del_dependence(task& parent, task& child);
        // мен€ет 1 задачу на другую сохран€€ зависимости от старой
        void change_task(task* old_t, task* new_t);
        void change_task(task& old_t, task& new_t);
        // проверка на наличие
        bool contain_task(task* t);
        bool contain_task(task& t);
        bool contain_data(message* m);
        bool contain_data(message& m);
        bool contain_dependence(task* parent, task* child);
        bool contain_dependence(task& parent, task& child);

        void clear();

        friend class parallelizer;
    };

}

#endif // __TASK_GRAPH_H__
