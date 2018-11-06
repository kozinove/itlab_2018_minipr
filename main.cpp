#include <iostream>
#include <mpi.h>
#include <ctime>
using namespace std;

int main(int argc, char* argv[])
{
    int ** a, * b, *c, *cmpi;
    int n = 4, m = 50;
    a = new int*[n];
    a[0] = new int[n*m];
    b = new int[m];
    c = new int[n];
    cmpi = new int[n];
    int t = 1;
    for(int i = 0; i < n; i++)
    {
        if(i)
            a[i] = a[0] + m*i;
        for(int j = 0; j < m; j++)
            a[i][j] = t++;
        c[i] = cmpi[i] = 0;
    }
    t = m;
    for(int i = 0; i < m; i++)
        b[i] =  t--;
    //NON-PARALLEL PROGRAMM
    int t1 = clock();
    for(int i = 0; i < n; i++)
        for(int j = 0; j < m; j++)
            c[i]+=a[i][j]*b[j];
    int t2 = clock();
    //PARALLEL PROGRAMM
    MPI_Init(&argc, &argv);
    double Start = MPI_Wtime();
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;
    int message[2];
    //message[0] - сколько элементов
    //message[1] - индекс начала
    if(size == 1)
    {
        for(int i = 0; i < n; i++)
            for(int j = 0; j < m; j++)
                cmpi[i]+=a[i][j]*b[j];
        for(int i = 0; i < n; i++)
            cout<<c[i]<<" "<<cmpi[i]<<"\n";
    }
    else
    {
        if(rank == 0)
        {
            int del = n/(size-1);
            if(del == 0)
                del = 1;
            for(int i = 1; i < min(size, n+1); i++)
            {
                if(i == size-1)
                    message[0] = n-del*(i-1);
                else
                    message[0] = del;
                message[1] = del*(i-1);
                MPI_Send(&message, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Recv(cmpi+message[1], message[0], MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
            double Finish = MPI_Wtime();
            for(int i = 0; i < n; i++)
            {
                cout<<c[i]<<" "<<cmpi[i]<<"\n";
                if(cmpi[i] != c[i])
                {
                    cout<<"NON_EQUAL ANSWERS\n";
                    break;
                }
            }
            cout<<double(t2-t1)/CLOCKS_PER_SEC<<" "<<Finish-Start<<"\n";
        }
        else
        {
            if(rank<=n)
            {
                MPI_Recv(&message, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                for(int i = message[1]; i<message[1]+message[0]; i++)
                    for(int j = 0; j < m; j++)
                        cmpi[i] += a[i][j]*b[j];
                MPI_Send(cmpi+message[1], message[0], MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        }
    }
    MPI_Finalize();
    return 0;
}
