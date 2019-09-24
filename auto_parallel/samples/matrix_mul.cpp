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

class mymessage: public message {
public:
    int size;
    int* arr;
    mymessage(int _size, int* _arr): message(), size(_size), arr(_arr) {}
    void send(sender& se) {
        se.isend(arr, size, MPI_INT);
    }
    void recv(receiver& re) {
        if (arr == nullptr)
            arr = new int[size];
        re.irecv(arr, size, MPI_INT);
    }
};


class onemessage: public message {
public:
    int a;
    onemessage(int _a): message(), a(_a) {}
    void send(sender& se) {
        se.isend(&a, 1, MPI_INT);
    }
    void recv(receiver& re) {
        re.irecv(&a, 1, MPI_INT);
    }
};


class mytask: public task {
public:
    mytask(std::vector<message*>& mes_v, std::vector<bool>& mode_v): task(mes_v, mode_v){}
    void perform() {
        int*& a = ((mymessage*)data_v[0])->arr;
        int*& b = ((mymessage*)data_v[1])->arr;
        int& c = ((onemessage*)data_v[2])->a;
        int size = ((mymessage*)data_v[0])->size;
        for(int i = 0; i < size; i++)
        {
            c += a[i]*b[i];
        }
    }
};

class out_task: public task {
public:
    out_task(std::vector<message*>& mes_v, std::vector<bool>& mode_v): task(mes_v, mode_v){}
    void perform() {
        int*& a = ((mymessage*)data_v[0])->arr;
        int size = ((mymessage*)data_v[0])->size;
        for(int i = 1; i <= size; i++)
        {
            a[i - 1] = ((onemessage*)data_v[i])->a;
            //printf("%d ", a[i - 1]);
        }
        //printf("\n");
    }
};

class init_task : public task
{
    public:
    init_task(std::vector<message*>& mes_v, std::vector<bool>& mode_v) : task(mes_v, mode_v) {}
    void perform()
    {
        int*& a = ((mymessage*)data_v[0])->arr;
        a = new int[size_t(n) * m];
        int tn = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
                a[i * m + j] = tn++;
            ((mymessage*)data_v[i])->arr = a + size_t(i) * size_t(m);
        }
    }
};

int main(int argc, char** argv) {
    if (argc > 1)
    {
        n = atoi(argv[1]);
        if (argc > 2)
            m = atoi(argv[2]);
    }
    int* b, *c;
    //a = new int*[n];
    //a[0] = new int[n*m];
    b = new int[m];
    c = new int[n];
    /*for(int i = 0; i < n; i++)
    {
        a[i] = a[0] + m*i;
    }*/
    int tn = 0;
    /*for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j++)
        {
            a[i][j] = tn++;
        }
    }*/
    tn = m;
    for(int i = 0; i < m; i++)
    {
        b[i] = tn--;
    }
    parallelizer pz(&argc, &argv);
    task_graph gr;
    mymessage* w = new mymessage(m, b);
    mymessage* cw = new mymessage(n, c);
    mytask** t = new mytask*[n];
    vector<message*> ve;
    vector<bool> be;
    vector<message*> vi;
    vector<bool> bi;
    ve.push_back(cw);
    be.push_back(message::read_write);
    for (int i = 0; i < n; ++i)
    {
        mymessage* p = new mymessage(m, nullptr);
        onemessage* q = new onemessage(0);
        vector<message*> v;
        vector<bool> e;
        v.push_back(p);
        v.push_back(w);
        v.push_back(q);
        e.push_back(message::read_only);
        e.push_back(message::read_only);
        e.push_back(message::read_write);
        ve.push_back(q);
        be.push_back(message::read_only);
        t[i] = new mytask(v,e);
        vi.push_back(p);
        bi.push_back(message::read_write);
    }
    out_task* te = new out_task(ve,be);
    init_task * ti = new init_task(vi, bi);
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
        //printf("work time: %f\n", time - parallelizer::get_start_time());
    }
}
