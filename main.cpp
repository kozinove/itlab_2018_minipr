#include <queue>
#include <algorithm>
#include <iostream>
#include "mpi.h"

using namespace std;

int main(int argc,char **argv)
{
    MPI_Init(&argc,&argv);
    MPI_Status stat;
    int prock,cpr;

    MPI_Comm_rank(MPI_COMM_WORLD,&cpr);
    MPI_Comm_size(MPI_COMM_WORLD,&prock);

    if (cpr!=0)
    {
        unsigned width;
        MPI_Bcast(&width,1,MPI_UNSIGNED,0,MPI_COMM_WORLD);
        double *b=new double[width];
        double *a=new double[width];
        MPI_Bcast(b,width,MPI_DOUBLE,0,MPI_COMM_WORLD);
        while (1)
        {
            char ch;
            MPI_Recv(&ch,1,MPI_CHAR,0,1,MPI_COMM_WORLD,&stat);
            if (ch==0)
                break;
            MPI_Recv(a,width,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&stat);
            double t=0.0;
            for (int i=0;i<width;++i)
                t+=a[i]*b[i];
            MPI_Send(&t,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
        }
    }
    else
    {
        if (prock==1)
        {
            cout <<"not parallel run";
            MPI_Finalize();
            return 0;
        }
        //initialization
        unsigned high=10000;
        unsigned width=10000;

        double *b=new double[width];
        double *rez1=new double[high];
        double *rez2=new double[high];
        double **a=new double*[high];
        a[0]=new double[high*width];

        for(int i=0;i<high;++i)
        {
            a[i]=a[0]+width*i;
            rez1[i]=rez2[i]=0.0;
        }

        for (int i=0;i<high;++i)
            for (int j=0;j<width;++j)
                    a[i][j]=double(i*width+j);

        for (int i=0;i<width;++i)
                b[i]=double(width-i-1);

        //simple
        double t1=MPI_Wtime();
        for (int i=0;i<high;++i)
            for (int j=0;j<width;++j)
                rez1[i]+=a[i][j]*b[j];
        double t2=MPI_Wtime();

        //parallel
        double t3=MPI_Wtime();
        int *cur=new int[prock];
        unsigned prin=0;
        queue<int> q;
        for (int i=0;i<high;++i)
            q.push(i);

        MPI_Bcast(&width,1,MPI_UNSIGNED,0,MPI_COMM_WORLD);
        MPI_Bcast(b,width,MPI_DOUBLE,0,MPI_COMM_WORLD);

        int stop=min((int)q.size(),prock-1);
        for (int i=0;i<stop;++i)
        {
            int tmp=q.front();
            q.pop();
            cur[i+1]=tmp;
            char ch=1;
            MPI_Send(&ch,1,MPI_CHAR,i+1,1,MPI_COMM_WORLD);
            MPI_Send(a[tmp],width,MPI_DOUBLE,i+1,0,MPI_COMM_WORLD);
        }

        while (q.size())
        {
            double t;
            MPI_Recv(&t,1,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&stat);
            ++prin;
            rez2[cur[stat.MPI_SOURCE]]=t;
            int tmp=q.front();
            q.pop();
            cur[stat.MPI_SOURCE]=tmp;
            char ch=1;
            MPI_Send(&ch,1,MPI_CHAR,stat.MPI_SOURCE,1,MPI_COMM_WORLD);
            MPI_Send(a[tmp],width,MPI_DOUBLE,stat.MPI_SOURCE,0,MPI_COMM_WORLD);
        }

        while (prin<high)
        {
            double t;
            MPI_Recv(&t,1,MPI_DOUBLE,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&stat);
            ++prin;
            rez2[cur[stat.MPI_SOURCE]]=t;
            char ch=0;
            MPI_Send(&ch,1,MPI_CHAR,stat.MPI_SOURCE,1,MPI_COMM_WORLD);
        }
        double t4=MPI_Wtime();

        //check
        bool fl=0;
        for (int i=0;i<high;++i)
            if (rez1[i]!=rez2[i])
                fl=1;
        if (fl)
            cout << "wrong\n";
        else
            cout << "correct\n";
        cout << "time simple: " << (t2-t1) << "\ntime mpi:    " << (t4-t3);
    }
    MPI_Finalize();
    return 0;
}
