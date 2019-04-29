#include "parallel.h"
#include <bits/stdc++.h>
#include <iostream>
#include <vector>
using namespace std;
using namespace auto_parallel;

int n = 100, m = 50;

class mymessage: public message {
public:
    int size;
    int* arr;
    mymessage(int _size, int* _arr): message(nullptr), size(_size), arr(_arr) {}
    void send(int proc) {
        MPI_Send(arr, size, MPI_INT, proc, 0, MPI_COMM_WORLD);
    }
    void recv(int proc) {
        MPI_Status status;
        MPI_Recv(arr, size, MPI_INT, proc, 0, MPI_COMM_WORLD, &status);
    }
};


class onemessage: public message {
public:
    int a;
    onemessage(int _a): message(nullptr), a(_a) {}
    void send(int proc) {
        MPI_Send(&a, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
    }
    void recv(int proc) {
        MPI_Status status;
        MPI_Recv(&a, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, &status);
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
            a[i] = ((onemessage*)data_v[i])->a;
            printf("%d ", a[i]);
        }
    }
};

int main(int argc, char** argv) {
    int** a, *b, *c;
    a = new int*[n];
    a[0] = new int[n*m];
    b = new int[m];
    c = new int[n];
    for(int i = 0; i < n; i++)
    {
        a[i] = a[0] + m*i;
    }
    int tn = 0;
    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j++)
        {
            a[i][j] = tn++;
        }
    }
    tn = m;
    for(int i = 0; i < m; i++)
    {
        b[i] = tn--;
    }
    task_graph gr;
    mymessage* w = new mymessage(m, b);
    mymessage* cw = new mymessage(n, c);
    mytask** t = new mytask*[n];
    vector<message*> ve;
    vector<bool> be;
    ve.push_back(cw);
    be.push_back(message::read_write);
    for (int i = 0; i < n; ++i)
    {
        mymessage* p = new mymessage(m, a[i]);
        onemessage* q = new onemessage(0);
        vector<message*> v;
        vector<bool> e;
        v.push_back(p);
        v.push_back(w);
        v.push_back(q);
        e.push_back(message::read_write);
        e.push_back(message::read_only);
        e.push_back(message::read_write);
        ve.push_back(q);
        be.push_back(message::read_write);
        t[i] = new mytask(v,e);
    }
    out_task* te = new out_task(ve,be);
    for (int i = 0; i < n; ++i)
        gr.add_dependence(t[i], te);
    parallelizer pz(gr, &argc, &argv);
    pz.execution();
    //for (int i = 0; i < n; ++i)
        //printf("%d ", c[i]);
}
