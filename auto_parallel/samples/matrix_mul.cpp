#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <algorithm>
#include <iostream>
#include "parallel.h"
using namespace std;
using namespace auto_parallel;

int n = 100, m = 50;

class mymessage: public message
{
public:
    int size;
    int* arr;
    mymessage(int _size, int* _arr): message(), size(_size), arr(_arr)
    { }
    void send(sender& se)
    { se.isend(arr, size, MPI_INT); }
    void recv(receiver& re)
    {
        if (arr == nullptr)
            arr = new int[size];
        re.irecv(arr, size, MPI_INT);
    }
};


class onemessage: public message
{
public:
    int a;
    onemessage(int _a): message(), a(_a)
    {   }
    void send(sender& se)
    { se.isend(&a, 1, MPI_INT); }
    void recv(receiver& re)
    { re.irecv(&a, 1, MPI_INT); }
};


class mytask: public task
{
public:
    mytask(std::vector<message*>& mes_v, std::vector<const message*>& cmes_v): task(mes_v, cmes_v)
    { }
    void perform()
    {
        int*& a = ((mymessage*)c_data[0])->arr;
        int*& b = ((mymessage*)c_data[1])->arr;
        int& c = ((onemessage*)data[0])->a;
        int size = ((mymessage*)data[0])->size;
        for(int i = 0; i < size; i++)
            c += a[i]*b[i];
    }
};

class out_task: public task
{
public:
    out_task(std::vector<message*>& mes_v, std::vector<const message*>& cmes_v): task(mes_v, cmes_v)
    { }
    void perform()
    {
        int*& a = ((mymessage*)data[0])->arr;
        int size = ((mymessage*)data[0])->size;
        for(int i = 0; i < size; i++)
            a[i] = ((onemessage*)c_data[i])->a;
    }
};

class init_task : public task
{
    public:
    init_task(std::vector<message*>& mes_v, std::vector<const message*>& cmes_v) : task(mes_v, cmes_v)
    { }
    void perform()
    {
        int*& a = ((mymessage*)data[0])->arr;
        a = new int[size_t(n) * m];
        int tn = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
                a[i * m + j] = tn++;
            ((mymessage*)data[i])->arr = a + size_t(i) * size_t(m);
        }
    }
};

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        n = atoi(argv[1]);
        if (argc > 2)
            m = atoi(argv[2]);
    }
    int* b, *c;
    b = new int[m];
    c = new int[n];
    int tn = 0;
    tn = m;
    for(int i = 0; i < m; i++)
        b[i] = tn--;
    parallelizer pz(&argc, &argv);
    task_graph gr;
    mymessage* w = new mymessage(m, b);
    mymessage* cw = new mymessage(n, c);
    mytask** t = new mytask*[n];
    vector<message*> ve;
    vector<const message*> cve;
    vector<message*> vi;
    vector<const message*> cvi;
    ve.push_back(cw);
    for (int i = 0; i < n; ++i)
    {
        mymessage* p = new mymessage(m, nullptr);
        onemessage* q = new onemessage(0);
        vector<message*> v;
        vector<const message*> cv;
        cv.push_back(p);
        cv.push_back(w);
        v.push_back(q);
        cve.push_back(q);
        t[i] = new mytask(v,cv);
        vi.push_back(p);
    }
    out_task* te = new out_task(ve,cve);
    init_task * ti = new init_task(vi, cvi);
    for (int i = 0; i < n; ++i)
    {
        gr.add_dependence(t[i], te);
        gr.add_dependence(ti, t[i]);
    }
    pz.init(gr);
    pz.execution();
    if (pz.get_current_proc() == 0)
    {
        double time = MPI_Wtime();
        cout << time - pz.get_start_time();
    }
}
