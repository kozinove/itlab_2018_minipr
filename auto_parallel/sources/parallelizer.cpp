#include "parallelizer.h"

namespace auto_parallel
{

    parallelizer::parallelizer(int mode, int* argc, char*** argv)
    {
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
        MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    }

    parallelizer::parallelizer(int mode, const task_graph& _tg, int* argc, char*** argv)
    {
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
        MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
        init(_tg);
    }

    parallelizer::~parallelizer()
    {
        MPI_Finalize();
    }

    void parallelizer::init(const task_graph& _tg)
    {
        while (ready_tasks.size())
            ready_tasks.pop();

        task_v.clear();
        data_v.clear();
        task_v.resize(_tg.t_map.size());
        data_v.resize(_tg.d_map.size());

        std::map<task*, int> tmp;
        std::map<message*, int> dmp;
        std::map<int, task*> tmpr;
        std::map<int, message*> dmpr;
        unsigned i = 0;

        for (auto it = _tg.d_map.begin(); it != _tg.d_map.end(); ++it)
        {
            dmpr[(*it).second.id] = (*it).first;
        }

        for (auto it = dmpr.begin(); it != dmpr.end(); ++it, ++i)
        {
            dmp[(*it).second] = i;
            data_v[i].d = (*it).second;
            data_v[i].version = 0;
        }
        dmpr.clear();

        for (auto it = _tg.t_map.begin(); it != _tg.t_map.end(); ++it)
        {
            tmpr[(*it).second.id] = (*it).first;
        }

        i = 0;
        for (auto it = tmpr.begin(); it != tmpr.end(); ++it, ++i)
        {
            tmp[(*it).second] = i;
            task_v[i].t = (*it).second;
            task_v[i].parents = (*_tg.t_map.find((*it).second)).second.parents.size();
            if (task_v[i].parents == 0)
                ready_tasks.push(i);
        }
        tmpr.clear();

        for (i = 0; i < task_v.size(); ++i)
        {
            const std::set<task*>& tp = (*_tg.t_map.find(task_v[i].t)).second.childs;
            task_v[i].childs.resize(tp.size());
            unsigned j = 0;
            for (auto it = tp.begin(); it != tp.end(); ++it, ++j)
                task_v[i].childs[j] = tmp[*it];
            task_v[i].data_id.resize(task_v[i].t->data_v.size());
            for (j = 0; j < task_v[i].t->data_v.size(); ++j)
                task_v[i].data_id[j] = dmp[task_v[i].t->data_v[j]];
        }
    }

}
