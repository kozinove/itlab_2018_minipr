#include <iostream>
#include <ctime>
#include <omp.h>
#include <queue>
using namespace std;

void HoaraSort(int *a, int first, int last)
{
    queue<pair<int,int>>q;
    q.push({first, last});
    while(!q.empty())
    {
        pair<int, int> cur = q.front();
        q.pop();
        int i = cur.first, j = cur.second, x = a[(cur.first+cur.second)/2];
        if(i-j >= 0 && i-j < 50)
        {
            for(int k = i; k < j; k++)
            {
                for(int l = k+1; l < j; l++)
                {
                    if(a[k]>a[l])
                        swap(a[k], a[l]);
                }
            }
            continue;
        }
        while(i <= j)
        {
            while(a[i] < x)
                i++;
            while(a[j] > x)
                j--;
            if(i <= j)
            {
                if(i < j)
                    swap(a[i], a[j]);
                i++;
                j--;
            }
        }
        if(i < cur.second)
            q.push({i, cur.second});
        if(j > cur.first)
            q.push({cur.first, j});
    }
}

void OmpHoaraSort(int *a, int first, int last)
{
    queue<pair<int,int>>q;
    q.push({first, last});
    while(!q.empty() && q.size() < omp_get_max_threads())
    {
        pair<int, int> cur = q.front();
        q.pop();
        int i = cur.first, j = cur.second, x = a[(cur.first+cur.second)/2];
        if(i-j >= 0 && i-j < 50)
        {
            for(int k = i; k < j; k++)
            {
                for(int l = k+1; l < j; l++)
                {
                    if(a[k]>a[l])
                        swap(a[k], a[l]);
                }
            }
            continue;
        }
        while(i <= j)
        {
            while(a[i] < x)
                i++;
            while(a[j] > x)
                j--;
            if(i <= j)
            {
                if(i < j)
                    swap(a[i], a[j]);
                i++;
                j--;
            }
        }
        if(i < cur.second)
            q.push({i, cur.second});
        if(j > cur.first)
            q.push({cur.first, j});
    }
    #pragma omp parallel
    {
        queue<pair<int,int>>q2;
        #pragma omp critical
        {
            q2.push(q.front());
            q.pop();
        }
        while(!q2.empty())
        {
            pair<int, int> cur = q2.front();
            q2.pop();
            int i = cur.first, j = cur.second, x = a[(cur.first+cur.second)/2];
            if(i-j >= 0 && i-j < 10)
            {
                for(int k = i; k < j; k++)
                {
                    for(int l = k+1; l < j; l++)
                    {
                        if(a[k]>a[l])
                            swap(a[k], a[l]);
                    }
                }
                continue;
            }
            while(i <= j)
            {
                while(a[i] < x)
                    i++;
                while(a[j] > x)
                    j--;
                if(i <= j)
                {
                    if(i < j)
                        swap(a[i], a[j]);
                    i++;
                    j--;
                }
            }
            if(i < cur.second)
                q2.push({i, cur.second});
            if(j > cur.first)
                q2.push({cur.first, j});
        }
    }
}

void OmpTasksHoaraSort(int* a, int l, int r)
{
    int i = l, j = r, x = a[(l+r)/2];
    if(i-j >= 0 && i-j < 50)
    {
        for(int k = i; k < j; k++)
        {
            for(int l = k+1; l < j; l++)
            {
                if(a[k]>a[l])
                    swap(a[k], a[l]);
            }
        }
        return;
    }
    while(i <= j)
    {
        while(a[i] < x)
            i++;
        while(a[j] > x)
            j--;
        if(i <= j)
        {
            if(i < j)
                swap(a[i], a[j]);
            i++;
            j--;
        }
    }
    #pragma omp task shared(a)
    if(j > l)
        OmpTasksHoaraSort(a, l, j);
    #pragma omp task shared(a)
    if(i < r)
        OmpTasksHoaraSort(a, i, r);
}

int main()
{
    int n = 50000000;
    int *a1, *a2, *a3;
    a1 = new int[n];
    a2 = new int[n];
    a3 = new int[n];
    srand(time(0));
    for(int i = 0; i < n; i++)
    {
        a1[i] = 1 + rand()%10000;
        a3[i] = a2[i] = a1[i];
    }
    double t1 = omp_get_wtime();
    HoaraSort(a1, 0, n-1);
    double t2 = omp_get_wtime();
    cout<<t2-t1<<" <- Single Hoara Sort\n";
    OmpHoaraSort(a2, 0, n-1);
    double t3 = omp_get_wtime();
    cout<<t3-t2<<" <- Multiple Hoara Sort\n";
    #pragma omp parallel shared(a3)
    {
        #pragma omp single nowait
        {
            OmpTasksHoaraSort(a3, 0, n-1);
        }
    }
    double t4 = omp_get_wtime();
    cout<<t4-t3<<" <- Multiple Tasks Hoara Sort\n";
//    for(int i = 0; i < n; i++)
//    {
//        if(a1[i] != a2[i])
//            cout<<"multiple Hoara isn't correct\n";
//        if(a3[i] != a2[i])
//        {
//            if(i && a3[i-1] < a3[i])
//                cout<<"multiple tasks Hoara isn't correct\n";
//        }
//    }
//    for(int i = 0; i < n; i++)
//        cout<<a1[i]<<" "<<a2[i]<<"\n";
//    cout<<"\n";
    delete[] a1;
    delete[] a2;
    delete[] a3;
    return 0;
}
