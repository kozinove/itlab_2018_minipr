#include "parallelizer.h"

namespace auto_parallel
{

    const int parallelizer::main_proc = 0;

    parallelizer::parallelizer(int* argc, char*** argv): parallel_engine(argc, argv), comm(MPI_COMM_WORLD), instr_comm(comm)
    { }

    parallelizer::parallelizer(task_graph& _tg, int* argc, char*** argv): parallel_engine(argc, argv), comm(MPI_COMM_WORLD), instr_comm(comm)
    { init(_tg); }

    parallelizer::~parallelizer()
    { }

    void parallelizer::init(task_graph& _tg)
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
            dmpr[(*it).second.id] = (*it).first;

        for (auto it = dmpr.begin(); it != dmpr.end(); ++it, ++i)
        {
            dmp[(*it).second] = i;
            data_v[i].d = (*it).second;
            data_v[i].version = 0;
        }
        dmpr.clear();

        for (auto it = _tg.t_map.begin(); it != _tg.t_map.end(); ++it)
            tmpr[(*it).second.id] = (*it).first;

        i = 0;
        for (auto it = tmpr.begin(); it != tmpr.end(); ++it, ++i)
        {
            tmp[(*it).second] = i;
            task_v[i].t = (*it).second;
            task_v[i].parents = int((*_tg.t_map.find((*it).second)).second.parents.size());
            if (task_v[i].parents == 0)
                ready_tasks.push({i, -1});
        }
        tmpr.clear();

        for (i = 0; i < task_v.size(); ++i)
        {
            const std::set<task*>& tp = (*_tg.t_map.find(task_v[i].t)).second.childs;
            task_v[i].childs.resize(tp.size());
            size_t j = 0;
            for (auto it = tp.begin(); it != tp.end(); ++it, ++j)
                task_v[i].childs[j] = tmp[*it];
            for (j = 0; j < task_v[i].t->data.size(); ++j)
                task_v[i].data_id.push_back(dmp[task_v[i].t->data[j]]);
            for (j = 0; j < task_v[i].t->c_data.size(); ++j)
            {
                message* t = const_cast<message*>(task_v[i].t->c_data[j]);
                task_v[i].const_data_id.push_back(dmp[t]);
            }
        }
        _tg.clear();

        top_versions.resize(data_v.size());
        top_versions.assign(data_v.size(), 0);
    }

    void parallelizer::execution()
    {
        if (comm.get_size() < 2)
            sequential_execution();
        else if (comm.get_rank() == main_proc)
            master();
        else
            worker();
    }

    void parallelizer::sequential_execution()
    {
        while(ready_tasks.size())
        {
            std::pair<int, int> p = ready_tasks.front();
            ready_tasks.pop();
            task_v[p.first].t->perform();
            for (int i: task_v[p.first].childs)
            {
                --task_v[i].parents;
                if (task_v[i].parents <= 0)
                    ready_tasks.push({i, -1});
            }
        }
    }

    void parallelizer::master()
    {
        std::vector<std::set<int>> versions(data_v.size());
        for (std::set<int>& i: versions)
            for (int j = 0; j < comm.get_size(); ++j)
                i.insert(j);

        std::queue<std::pair<int,int>> working_procs;

        std::set<int> ready_procs;
        for (int i = 0; i < comm.get_size(); ++i)
            if (i != main_proc)
                ready_procs.insert(i);

        int start_task = 0;
        int my_task = -1;
        int cur_proc = 0;

        while (ready_tasks.size())
        {

            for (int i = start_task; i < ready_tasks.size(); ++i)
            {
                if (ready_tasks[i].second == -1)
                {
                    if (cur_proc != main_proc)
                    {
                        send_instruction(1, cur_proc, ready_tasks[i].first);
                        send_task_data(ready_tasks[i].first, cur_proc, versions);
                    }
                    ready_tasks[i].second = cur_proc;
                    next_proc(cur_proc);
                }
                ++start_task;
            }
            start_task -= (int)ready_procs.size() + 1;

            while (ready_tasks.size())
            {
                std::pair<int,int> cur_t = ready_tasks.front();
                if (cur_t.second == main_proc)
                {
                    if (my_task != -1)
                        break;
                    ready_tasks.pop();
                    my_task = cur_t.first;
                    continue;
                }
                if (ready_procs.find(cur_t.second) == ready_procs.end())
                    break;
                ready_procs.erase(cur_t.second);
                ready_tasks.pop();
                send_instruction(0, cur_t.second, cur_t.first);
                working_procs.push({cur_t.second, cur_t.first});
            }
            start_task += (int)ready_procs.size();

            if (my_task != -1)
            {
                task_v[my_task].t->perform();
                std::vector<int>& d = task_v[my_task].data_id;
                for (int i = 0; i < d.size(); ++i)
                {
                    top_versions[d[i]]++;
                    versions[d[i]].clear();
                    versions[d[i]].insert(main_proc);
                }
                for (int i: task_v[my_task].childs)
                {
                    --task_v[i].parents;
                    if (task_v[i].parents <= 0)
                        ready_tasks.push({i, -1});
                }
                my_task = -1;
            }
            else
                start_task += 1;

            while (working_procs.size())
            {
                std::pair<int,int> p = working_procs.front();
                working_procs.pop();
                wait_proc(p.second, p.first, versions);
                ready_procs.insert(p.first);
            }
        }

        for (int i = 0; i < comm.get_size(); ++i)
            if (i != main_proc)
                send_instruction(-1, i, 0);
    }

    void parallelizer::worker()
    {
        instruction cur_inst;
        bool exe = true;
        while(exe)
        {
            cur_inst = recv_instruction(main_proc);
            switch (cur_inst.n[0])
            {
            case 0:
                execute_task(cur_inst.n[1]);
                break;
            case 1:
                recv_task_data(cur_inst.n[1], main_proc);
                break;
            default:
                exe = false;
            }
        }
    }

    void parallelizer::send_task_data(int tid, int proc, std::vector<std::set<int>>& ver)
    {
        std::vector<int>& d = task_v[tid].data_id;
        std::vector<int>& cd = task_v[tid].const_data_id;
        std::vector<int> low_versions;

        for (int i = 0; i < d.size(); ++i)
        {
            if (ver[d[i]].find(proc) == ver[d[i]].end())
            {
                ver[d[i]].insert(proc);
                low_versions.push_back(d[i]);
            }
        }
        for (int i = 0; i < cd.size(); ++i)
        {
            if (ver[cd[i]].find(proc) == ver[cd[i]].end())
            {
                ver[cd[i]].insert(proc);
                low_versions.push_back(cd[i]);
            }
        }

        MPI_Send(low_versions.data(), low_versions.size(), MPI_INT, proc, 3, instr_comm.get_comm());

        for (int i = 0; i < low_versions.size(); ++i)
            comm.send(data_v[low_versions[i]].d, proc);

    }

    void parallelizer::wait_proc(int task_id, int proc, std::vector<std::set<int>>& v)
    {
        std::vector<int>& d = task_v[task_id].data_id;
        std::vector<int>& cd = task_v[task_id].const_data_id;

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();

        for (int i = 0; i < cd.size(); ++i)
            data_v[cd[i]].d->wait_requests();

        for (int i = 0; i < d.size(); ++i)
        {
            comm.recv(data_v[d[i]].d, proc);
            top_versions[d[i]]++;
            v[d[i]].clear();
            v[d[i]].insert(proc);
            v[d[i]].insert(main_proc);
        }

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();

        for (int i: task_v[task_id].childs)
        {
            --task_v[i].parents;
            if (task_v[i].parents <= 0)
                ready_tasks.push({i, -1});
        }
    }

    void parallelizer::recv_task_data(int tid, int proc)
    {
        MPI_Status status;
        int size;

        MPI_Probe(proc, 3, instr_comm.get_comm(), &status);
        MPI_Get_count(&status, MPI_INT, &size);
        int* recv_d = new int[size];
        MPI_Recv(recv_d, size, MPI_INT, proc, 3, instr_comm.get_comm(), &status);

        for (int i = 0; i < size; ++i)
            comm.recv(data_v[recv_d[i]].d, proc);

        delete[] recv_d;
    }

    void parallelizer::execute_task(int task_id)
    {
        std::vector<int>& d = task_v[task_id].data_id;
        std::vector<int>& cd = task_v[task_id].const_data_id;

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();

        for (int i = 0; i < cd.size(); ++i)
            data_v[cd[i]].d->wait_requests();

        task_v[task_id].t->perform();

        for (size_t i = 0; i < d.size(); ++i)
            comm.send(data_v[d[i]].d, main_proc);

        for (int i = 0; i < d.size(); ++i)
            data_v[d[i]].d->wait_requests();
    }

    void parallelizer::send_instruction(int type, int proc, int info)
    {
        instruction i;
        i.n[0] = type;
        i.n[1] = info;
        MPI_Send(i.n, 2, MPI_INT, proc, 1, instr_comm.get_comm());
    }

    parallelizer::instruction parallelizer::recv_instruction(int proc)
    {
        MPI_Status status;
        instruction i;
        MPI_Recv(i.n, 2, MPI_INT, proc, 1, instr_comm.get_comm(), &status);
        return i;
    }

    void parallelizer::next_proc(int& proc)
    { proc = (1 + proc) % comm.get_size(); }

    int parallelizer::get_current_proc()
    { return comm.get_rank(); }

}
