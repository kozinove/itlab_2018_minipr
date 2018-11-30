#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <queue>
#include <random>
#include <time.h>
#include <omp.h>
using namespace std;

// simple quick sort
void quicksort(int* a,int size);
// slightly slower than simple
void parallel_quicksort(int* a,int size);
// fastest
void very_quicksort(int* a,int size);

int main(int argc,char** argv)
{
    ios_base::sync_with_stdio(0);
    int size=1000000;
    if (argc>1)
        size=atoi(argv[1]);
    if (argc>2)
        omp_set_num_threads(atoi(argv[2]));

    int *a1=new int[size];
    int *a2=new int[size];
    int *a3=new int[size];
    random_device ra;
    mt19937 rand(time(0));
    uniform_int_distribution<int> random(INT_MIN,INT_MAX);
    for (int i=0;i<size;++i)
        a1[i]=a2[i]=a3[i]=random(rand);

    double t1=omp_get_wtime();
    quicksort(a1,size);
    double t2=omp_get_wtime();
    parallel_quicksort(a2,size);
    double t3=omp_get_wtime();
    very_quicksort(a3,size);
    double t4=omp_get_wtime();

    bool fl=0;
    for (int i=0;i<size;++i)
        fl|=(a1[i]!=a2[i])|(a1[i]!=a3[i]);
    if (fl)
        cout << "wrong\n";
    else
        cout << "correct\n";
    cout << "simple:   " << (t2-t1) << '\n';
    cout << "parallel: " << (t3-t2) << '\n';
    cout << "fast:     " << (t4-t3) << '\n';
}

void parallel_quicksort(int* a,int size)
{
    struct sort_com
    {
        int l,r;
    };

    queue<sort_com> q;
    q.push({0,size-1});
    #pragma omp parallel shared(a,q)
    {
        while (q.size())
        {
            sort_com t;
            int bel,mi,ma,l,r;
            bool fl=0;
            #pragma omp critical (start)
            {
                if (q.size())
                {
                    t=q.front();
                    q.pop();
                }
                else
                    fl=1;
            }

            if (fl)
                goto barr;

            if (t.r-t.l<10)
            {
                for (int i=t.l;i<t.r;++i)
                    for (int j=i+1;j>t.l;--j)
                        if (a[j-1]>a[j])
                            swap(a[j-1],a[j]);
                        else
                            break;
                goto barr;
            }

            mi=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
            ma=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
            if ((a[t.l]>mi)&&(a[t.l]<ma))
                bel=a[t.l];
            else if ((a[t.r]>mi)&&(a[t.r]<ma))
                bel=a[t.r];
            else
                bel=a[(t.l+t.r)/2];

            l=t.l;
            r=t.r;
            while(l<=r)
            {
                while (a[l]<bel)
                    ++l;
                while (a[r]>bel)
                    --r;
                if (l<=r)
                    swap(a[l++],a[r--]);
            }

            #pragma omp critical (finish)
            {
                q.push({t.l,r});
                q.push({r+1,t.r});
            }
            barr:;
        }
    }
}

void very_quicksort(int* a,int size)
{
    struct sort_com
    {
        int l,r;
    };

    queue<sort_com> q;
    q.push({0,size-1});
    while ((q.size()>0)&&(q.size()<omp_get_max_threads()))
    {
        sort_com t=q.front();
        q.pop();

        if (t.r-t.l<10)
        {
            for (int i=t.l;i<t.r;++i)
                for (int j=i+1;j>t.l;--j)
                    if (a[j-1]>a[j])
                        swap(a[j-1],a[j]);
                    else
                        break;
            continue;
        }

        int bel;
        int mi=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
        int ma=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
        if ((a[t.l]>mi)&&(a[t.l]<ma))
            bel=a[t.l];
        else if ((a[t.r]>mi)&&(a[t.r]<ma))
            bel=a[t.r];
        else
            bel=a[(t.l+t.r)/2];

        int l=t.l,r=t.r;
        while(l<=r)
        {
            while (a[l]<bel)
                ++l;
            while (a[r]>bel)
                --r;
            if (l<=r)
                swap(a[l++],a[r--]);
        }
        q.push({t.l,r});
        q.push({r+1,t.r});
    }

    #pragma omp parallel shared(a,q)
    {
        queue<sort_com> u;
        #pragma omp critical
        {
            u.push(q.front());
            q.pop();
        }
        while (u.size())
        {
            sort_com t;
            int bel,mi,ma,l,r;
            t=u.front();
            u.pop();

            if (t.r-t.l<10)
            {
                for (int i=t.l;i<t.r;++i)
                    for (int j=i+1;j>t.l;--j)
                        if (a[j-1]>a[j])
                            swap(a[j-1],a[j]);
                        else
                            break;
                continue;
            }

            mi=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
            ma=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
            if ((a[t.l]>mi)&&(a[t.l]<ma))
                bel=a[t.l];
            else if ((a[t.r]>mi)&&(a[t.r]<ma))
                bel=a[t.r];
            else
                bel=a[(t.l+t.r)/2];

            l=t.l;
            r=t.r;
            while(l<=r)
            {
                while (a[l]<bel)
                    ++l;
                while (a[r]>bel)
                    --r;
                if (l<=r)
                    swap(a[l++],a[r--]);
            }
            u.push({t.l,r});
            u.push({r+1,t.r});
        }
    }
}

void quicksort(int* a,int size)
{
    struct sort_com
    {
        int l,r;
    };

    queue<sort_com> q;
    q.push({0,size-1});
    while (q.size())
    {
        sort_com t=q.front();
        q.pop();

        if (t.r-t.l<10)
        {
            for (int i=t.l;i<t.r;++i)
                for (int j=i+1;j>t.l;--j)
                    if (a[j-1]>a[j])
                        swap(a[j-1],a[j]);
                    else
                        break;
            continue;
        }

        int bel;
        int mi=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
        int ma=min(min(a[t.l],a[t.r]),a[(t.l+t.r)/2]);
        if ((a[t.l]>mi)&&(a[t.l]<ma))
            bel=a[t.l];
        else if ((a[t.r]>mi)&&(a[t.r]<ma))
            bel=a[t.r];
        else
            bel=a[(t.l+t.r)/2];

        int l=t.l,r=t.r;
        while(l<=r)
        {
            while (a[l]<bel)
                ++l;
            while (a[r]>bel)
                --r;
            if (l<=r)
                swap(a[l++],a[r--]);
        }
        q.push({t.l,r});
        q.push({r+1,t.r});
    }
}
