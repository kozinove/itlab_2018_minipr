#include "parallelizer.h"

namespace auto_parallel
{

    double parallelizer::start_time = 0.0;
    const int parallelizer::main_proc = 0;
    std::map<std::pair<message*,int>, std::vector<MPI_Request>> parallelizer::requests;

    parallelizer::parallelizer(int* argc, char*** argv)
    {
        int flag;
        MPI_Initialized(&flag);
        if (!flag)
        {
            MPI_Init(argc, argv);
            start_time = MPI_Wtime();
        }
        MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
        MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    }

    parallelizer::parallelizer(const task_graph& _tg, int* argc, char*** argv)
    {
        int flag;
        MPI_Initialized(&flag);
        if (!flag)
        {
            MPI_Init(argc, argv);
            start_time = MPI_Wtime();
        }
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
                ready_tasks.push({i, -1});
        }
        tmpr.clear();

        for (i = 0; i < task_v.size(); ++i)
        {
            const std::set<task*>& tp = (*_tg.t_map.find(task_v[i].t)).second.childs;
            task_v[i].childs.resize(tp.size());
            unsigned j = 0;
            for (auto it = tp.begin(); it != tp.end(); ++it, ++j)
                task_v[i].childs[j] = tmp[*it];
            for (j = 0; j < task_v[i].t->data_v.size(); ++j)
                if (task_v[i].t->mods[j] == message::read_write)
                    task_v[i].data_id.push_back(dmp[task_v[i].t->data_v[j]]);
                else
                    task_v[i].const_data_id.push_back(dmp[task_v[i].t->data_v[j]]);
        }
        top_versions.resize(data_v.size());
        top_versions.assign(data_v.size(), 0);
    }

    void parallelizer::execution()
    {
        if (proc_size < 2)
            sequential_execution();
        else if (proc_id == main_proc)
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
            for (int j = 0; j < proc_size; ++j)
                i.insert(j);

        std::queue<std::pair<int,int>> working_procs;

        std::set<int> ready_procs;
        for (int i = 0; i < proc_size; ++i)
            if (i != main_proc)
                ready_procs.insert(i);

        int start_task = 0;

        while (ready_tasks.size())
        {
            int cur_proc = 1;
            // data sending
            for (int i = start_task; i < ready_tasks.size(); ++i)
            {
                if (ready_tasks[i].second == -1)
                {
                    send_instruction(1, cur_proc, ready_tasks[i].first);
                    send_task_data(ready_tasks[i].first, cur_proc, versions);
                    ready_tasks[i].second = cur_proc;
                    next_proc(cur_proc);
                }
                ++start_task;
            }
            start_task -= (int)ready_procs.size();
            // task sending
            while (ready_tasks.size())
            {
                std::pair<int,int> cur_t = ready_tasks.front();
                if (ready_procs.find(cur_t.second) == ready_procs.end())
                    break;
                ready_procs.erase(cur_t.second);
                ready_tasks.pop();
                send_instruction(0, cur_t.second, cur_t.first);
                working_procs.push({cur_t.second, cur_t.first});
            }
            start_task += (int)ready_procs.size();
            // task waiting
            while (working_procs.size())
            {
                std::pair<int,int> p = working_procs.front();
                working_procs.pop();
                wait_proc(p.second, p.first, versions);
                ready_procs.insert(p.first);
            }
        }

        for (int i = 0; i < proc_size; ++i)
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
        int* low_versions = new int[d.size() + cd.size()];
        int size = 0;

        for (int i = 0; i < d.size(); ++i)
        {
            if (ver[d[i]].find(proc) == ver[d[i]].end())
            {
                ver[d[i]].insert(proc);
                low_versions[size++] = d[i];
            }
        }
        for (int i = 0; i < cd.size(); ++i)
        {
            if (ver[cd[i]].find(proc) == ver[cd[i]].end())
            {
                ver[cd[i]].insert(proc);
                low_versions[size++] = cd[i];
            }
        }

        MPI_Send(low_versions, size, MPI_INT, proc, 3, MPI_COMM_WORLD);
        for (int i = 0; i < size; ++i)
            data_v[low_versions[i]].d->send(proc);

        delete[] low_versions;
    }

    void parallelizer::wait_proc(int task_id, int proc, std::vector<std::set<int>>& v)
    {
        std::vector<int>& d = task_v[task_id].data_id;
        std::vector<int>& cd = task_v[task_id].const_data_id;
        MPI_Status status;

        for (int i = 0; i < d.size(); ++i)
        {
            std::pair<message*, int> p(data_v[d[i]].d, proc);
            if (requests.find(p) == requests.end())
                continue;
            std::vector<MPI_Request>& vec = requests[p];
            for (int j = 0; j < vec.size(); ++j)
                MPI_Wait(&vec[j], &status);
            requests.erase(p);
        }

        for (int i = 0; i < cd.size(); ++i)
        {
            std::pair<message*, int> p(data_v[cd[i]].d, proc);
            if (requests.find(p) == requests.end())
                continue;
            std::vector<MPI_Request>& vec = requests[p];
            for (int j = 0; j < vec.size(); ++j)
                MPI_Wait(&vec[j],&status);
            requests.erase(p);
        }

        for (int i = 0; i < d.size(); ++i)
        {
            data_v[d[i]].d->recv(proc);
            top_versions[d[i]]++;
            v[d[i]].clear();
            v[d[i]].insert(proc);
            v[d[i]].insert(main_proc);
        }

        for (int i = 0; i < d.size(); ++i)
        {
            std::pair<message*, int> p(data_v[d[i]].d, proc);
            if (requests.find(p) == requests.end())
                continue;
            std::vector<MPI_Request>& vec = requests[p];
            for (int j = 0; j < vec.size(); ++j)
                MPI_Wait(&vec[j],&status);
            requests.erase(p);
        }

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

        MPI_Probe(proc, 3, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &size);
        int* recv_d = new int[size];
        MPI_Recv(recv_d, size, MPI_INT, proc, 3, MPI_COMM_WORLD, &status);

        for (int i = 0; i < size; ++i)
            data_v[recv_d[i]].d->recv(proc);

        delete[] recv_d;
    }

    void parallelizer::execute_task(int task_id)
    {
        std::vector<int>& d = task_v[task_id].data_id;
        std::vector<int>& cd = task_v[task_id].const_data_id;
        MPI_Status status;

        for (int i = 0; i < d.size(); ++i)
        {
            std::pair<message*, int> p(data_v[d[i]].d, main_proc);
            if (requests.find(p) == requests.end())
                continue;
            std::vector<MPI_Request>& vec = requests[p];
            for (int j = 0; j < vec.size(); ++j)
                MPI_Wait(&vec[j],&status);
            requests.erase(p);
        }

        for (int i = 0; i < cd.size(); ++i)
        {
            std::pair<message*, int> p(data_v[cd[i]].d, main_proc);
            if (requests.find(p) == requests.end())
                continue;
            std::vector<MPI_Request>& vec = requests[p];
            for (int j = 0; j < vec.size(); ++j)
                MPI_Wait(&vec[j],&status);
            requests.erase(p);
        }

        task_v[task_id].t->perform();

        for (size_t i = 0; i < d.size(); ++i)
            data_v[d[i]].d->send(main_proc);

        for (int i = 0; i < d.size(); ++i)
        {
            std::pair<message*, int> p(data_v[d[i]].d, main_proc);
            if (requests.find(p) == requests.end())
                continue;
            std::vector<MPI_Request>& vec = requests[p];
            for (int j = 0; j < vec.size(); ++j)
                MPI_Wait(&vec[j], &status);
            requests.erase(p);
        }

    }

    void parallelizer::send_instruction(int type, int proc, int info)
    {
        instruction i;
        i.n[0] = type;
        i.n[1] = info;
        MPI_Send(i.n, 2, MPI_INT, proc, 1, MPI_COMM_WORLD);
    }

    parallelizer::instruction parallelizer::recv_instruction(int proc)
    {
        MPI_Status status;
        instruction i;
        MPI_Recv(i.n, 2, MPI_INT, proc, 1, MPI_COMM_WORLD, &status);
        return i;
    }

    void parallelizer::next_proc(int& proc)
    {
        proc = 1 + proc % (proc_size - 1);
    }

    MPI_Request* parallelizer::new_request(message* mes, int proc)
    {
        std::pair<message*, int> p(mes, proc);
        MPI_Request r;
        requests[p].push_back(r);
        return &(requests[p][requests[p].size()-1u]);
    }

    int parallelizer::get_current_proc()
    {
        return proc_id;
    }

    double parallelizer::get_start_time()
    {
        return start_time;
    }

}
