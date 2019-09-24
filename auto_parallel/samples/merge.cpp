#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <algorithm>
#include "parallel.h"

using namespace std;
using namespace auto_parallel;

class m_array: public message
{
    private:
    int* p;
    int size;
    bool res;
    public:
    m_array(int sz, int* pt = nullptr): size(sz), p(pt)
    {
        if (p == nullptr)
        {
            p = new int[size];
            res = true;
        }
        else
            res = false;
    }
    ~m_array()
    {
        if (res)
            delete[] p;
    }
    void send(sender& se)
    {
        se.isend(p, size, MPI_INT);
    }
    void recv(receiver& re)
    {
        re.irecv(p, size, MPI_INT);
    }
    int* get_p()
    {
        return p;
    }
    int get_size()
    {
        return size;
    }
};

class merge_t: public task
{
    public:
    merge_t(vector<message*> vm, vector<bool> vb): task(vm, vb)
    {

    }
    void perform()
    {
        m_array* s1, *s2, *out;
        s1 = (m_array*)data_v[0];
        s2 = (m_array*)data_v[1];
        out = (m_array*)data_v[2];

        int first = 0, second = 0;
        int* p_out = out->get_p();
        for (int i = 0; i < out->get_size(); ++i)
        {
            if ((first >= s1->get_size()))
                p_out[i] = s2->get_p()[second++];
            else if ((second < s2->get_size()) && (s2->get_p()[second] < s1->get_p()[first]))
                p_out[i] = s2->get_p()[second++];
            else
                p_out[i] = s1->get_p()[first++];
        }
    }
    m_array* get_out()
    {
        return (m_array*)data_v[2];
    }
    m_array* get_first()
    {
        return (m_array*)data_v[0];
    }
    m_array* get_second()
    {
        return (m_array*)data_v[1];
    }
};

class merge_t_all: public task
{
    public:
    merge_t_all(vector<message*> vm, vector<bool> vb): task(vm, vb)
    {

    }
    void perform()
    {
        m_array* s1, *s2;
        s1 = (m_array*)data_v[0];
        s2 = (m_array*)data_v[1];
        for (int i = 0; i < s1->get_size(); ++i)
            s2->get_p()[i] = s1->get_p()[i];
        merge_it(s1->get_p(), s2->get_p(), s1->get_size()/2, s1->get_size());
    }
    void merge_it(int* s, int* out, int size1, int size2)
    {
        if (size2 < 2)
        {
            for (int i = 0; i < size2; ++i)
                out[i] = s[i];
            return;
        }
        merge_it(out, s, size1 / 2, size1);
        merge_it(out + size1, s + size1, (size2 - size1) / 2, size2 - size1);
        int first = 0;
        int second = size1;
        for (int i = 0; i < size2; ++i)
        {
            if ((first == size1))
                out[i] = s[second++];
            else if ((second < size2) && (s[second] < s[first]))
                out[i] = s[second++];
            else
                out[i] = s[first++];
        }
    }
};

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    int layers = 2;
    int size = 1000;
    if (argc > 1)
    {
        layers = atoi(argv[1]);
        if (argc > 2)
            size = atoi(argv[2]);
    }

    int* p1 = new int[size];
    int* p2 = new int[size];
    int* p3 = new int[size];
    mt19937 mt(time(0));
    uniform_int_distribution<int> uid(0, 10000);
    for (int i = 0; i < size; ++i)
        p1[i] = p3[i] = uid(mt);

    parallelizer pz;
    task_graph tg;
    vector<task*> v1, v2;
    int g = 1 << layers;
    if (layers != 0)
    {
        if (layers % 2 == 0)
            swap(p1,p2);
        vector<message*> w(3);
        vector<bool> e(3);
        w[0] = new m_array(size / 2, p2);
        w[1] = new m_array(size - size / 2, p2 + size / 2);
        w[2] = new m_array(size, p1);
        e[0] = message::read_only;
        e[1] = message::read_only;
        e[2] = message::read_write;
        v2.push_back(new merge_t(w, e));
        for (int i = 1; i < layers; ++i)
        {
            int q = 1 << i;
            v1.resize(q);
            for (int j = 0; j < q; ++j)
            {
                m_array* me;
                int* ptr;
                if (j%2)
                {
                    me = ((merge_t*)v2[j/2])->get_second();
                    ptr = ((merge_t*)v2[j/2])->get_out()->get_p() + ((merge_t*)v2[j/2])->get_first()->get_size();
                }
                else
                {
                    me = ((merge_t*)v2[j/2])->get_first();
                    ptr = ((merge_t*)v2[j/2])->get_out()->get_p();
                }
                w[0] = new m_array(me->get_size() / 2, ptr);
                w[1] = new m_array(me->get_size() - me->get_size() / 2, ptr + me->get_size() / 2);
                w[2] = me;
                e[0] = message::read_only;
                e[1] = message::read_only;
                e[2] = message::read_write;
                v1[j] = new merge_t(w, e);
                tg.add_dependence(v1[j], v2[j/2]);
            }
            swap(v1, v2);
        }
        w.clear();
        e.clear();
        w.resize(2);
        e.resize(2);
        e[0] = e[1] = message::read_write;
        for (int i = 0; i < v2.size(); ++i)
        {
            w[0] = new m_array(((merge_t*)v2[i])->get_first()->get_size(), ((merge_t*)v2[i])->get_out()->get_p());
            w[1] = ((merge_t*)v2[i])->get_first();
            tg.add_dependence(new merge_t_all(w, e), v2[i]);
            w[0] = new m_array(((merge_t*)v2[i])->get_second()->get_size(), ((merge_t*)v2[i])->get_out()->get_p()
                + ((merge_t*)v2[i])->get_first()->get_size());
            w[1] = ((merge_t*)v2[i])->get_second();
            tg.add_dependence(new merge_t_all(w, e), v2[i]);
        }
    }
    else
    {
        vector<message*> w(2);
        vector<bool> e(2);
        w[0] = new m_array(size, p1);
        w[1] = new m_array(size, p2);
        e[0] = message::read_write;
        e[1] = message::read_write;
        tg.add_task(new merge_t_all(w, e));
        swap(p1, p2);
    }

    pz.init(tg);
    pz.execution();

    if (pz.get_current_proc() == parallelizer::main_proc)
    {
        double dt = MPI_Wtime();
        sort(p3, p3 + size);
        double pt = MPI_Wtime();
        bool fl = false;
        for (int i = 0; i < size; ++i)
            if (p1[i] != p3[i])
                fl = true;
        if (fl)
            cout << "wrong\n";
        else
            cout << "correct\n";
        cout << dt - pz.get_start_time() << '\n' << pt - dt;
        cout.flush();
    }
    pz.~parallelizer();
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
