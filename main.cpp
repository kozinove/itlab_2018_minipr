#include <iostream>
#include <mpi.h>
#include <queue>

using namespace std;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int n = 100000, m = 500;
    double t1, t2, t3;
    int ** a, *b, *c, *cmpi;
    b = new int[m];
    int False = -1, True = 1;
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    int req;
    if(rank == 0)
    {
        c = new int[n];
        cmpi = new int[n];
        a = new int*[n];
        a[0] = new int[n*m];
        for(int i = 1; i < n; i++)
            a[i] = a[0] + m*i;
        int t = 1;
        for(int i = 0; i < n; i++)
        {
            for(int j = 0; j < m; j++)
                a[i][j] = t++;
            c[i] = cmpi[i] = 0;
        }
        t = m;
        for(int i = 0; i < m; i++)
            b[i] = t--;
        //NON PARALLEL PROGRAM
        t1 = MPI_Wtime();
        for(int i = 0; i < n; i++)
            for(int j = 0; j < m; j++)
                c[i]+=a[i][j]*b[j];
        t2 = MPI_Wtime();
        //END OF NON PARALLEL PROGRAM
    }
    if(size == 1)
    {
        for(int i = 0; i < n; i++)
            for(int j = 0; j < m; j++)
                cmpi[i]+=a[i][j]*b[j];
        for(int i = 0; i < n; i++)
        {
            //cout<<c[i]<<" "<<cmpi[i]<<"\n";
            if(c[i] != cmpi[i])
            {
                cout<<"ANSWERS ARE NOT EQUAL\n";
                break;
            }
        }
        MPI_Finalize();
        return 0;
    }
    MPI_Bcast(b, m, MPI_INT, 0, MPI_COMM_WORLD);
    if(!rank)
    {
        int *id;
        id = new int[size];
        int all = 0;
        for(int i = 0; i < size-1; i++)
        {
            if(i<n)
            {
                MPI_Send(&True, 1, MPI_INT, i+1, 0, MPI_COMM_WORLD);
                MPI_Send(a[i], m, MPI_INT, i+1, 0, MPI_COMM_WORLD);
                id[i+1] = i;
            }
            else
            {
                MPI_Send(&False, 1, MPI_INT, i+1, 0, MPI_COMM_WORLD);
                all++;
            }
        }
        queue<int>q;
        for(int i = size-1; i < n; i++)
            q.push(i);
        while(true)
        {
            MPI_Recv(&req, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            cmpi[id[status.MPI_SOURCE]] = req;
            if(q.size())
            {
                int x = q.front();
                MPI_Send(&True, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                MPI_Send(a[x], m, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                id[status.MPI_SOURCE] = x;
                q.pop();
            }
            else
            {

                MPI_Send(&False, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
                all++;
            }
            if(all == size-1)
                break;
        }
        t3 = MPI_Wtime();
        cout<<t2-t1<<" "<<t3-t2<<"\n";
        for(int i = 0; i < n; i++)
        {
            //cout<<c[i]<<" "<<cmpi[i]<<"\n";
            if(c[i] != cmpi[i])
            {
                cout<<"ANSWERS ARE NOT EQUAL\n";
                break;
            }
        }
        delete[] id;
        delete[] c;
        delete[] cmpi;
        delete[] a[0];
        delete[] a;
    }
    else
    {

        int* aa = new int[m];
        while(true)
        {
            MPI_Recv(&req, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(req == -1)
                break;
            MPI_Recv(aa, m, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int ans = 0;
            for(int i = 0; i < m; i++)
            {
                ans += aa[i]*b[i];
            }
            MPI_Send(&ans, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
        delete [] aa;
    }
    delete[] b;
    MPI_Finalize();
    return 0;
}
