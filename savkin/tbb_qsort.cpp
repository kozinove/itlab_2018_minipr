#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <queue>
#include <random>
#include <time.h>
#include "tbb.h"
#include "omp.h"
using namespace std;
using namespace tbb;

class qsort_task : public task
{
private:
	int* a;
	int first;
	int last;
public:
	qsort_task(int* _a, int _f, int _l):task(), a(_a), first(_f), last(_l)
	{}
	task* execute()
	{
		int bel;
		int mi = min(min(a[first], a[last]), a[(first + last) / 2]);
		int ma = min(min(a[first], a[last]), a[(first + last) / 2]);
		if ((a[first] > mi) && (a[first] < ma))
			bel = a[first];
		else if ((a[last] > mi) && (a[last] < ma))
			bel = a[last];
		else
			bel = a[(first + last) / 2];

		int l = first, r = last;
		while (l <= r)
		{
			while (a[l] < bel)
				++l;
			while (a[r] > bel)
				--r;
			if (l >= r)
				break;
			swap(a[l++], a[r--]);
		}

		int re = 1;
		if (first < r)
			++re;
		if (r + 1 < last)
			++re;
		set_ref_count(re);
		if (first < r)
		{
			qsort_task& left = *new(allocate_child()) qsort_task(a, first, r);
			spawn(left);
		}
		if (r + 1 < last)
		{
			qsort_task& right = *new(allocate_child()) qsort_task(a, r + 1, last);
			spawn(right);
		}
		wait_for_all();
		return NULL;
	}
};

void tbb_qsort(int* a, int first, int last)
{
	task_scheduler_init tsi(task_scheduler_init::automatic);
	qsort_task& b = *new(task::allocate_root()) qsort_task(a, first, last);
	task::spawn_root_and_wait(b);
}

void quicksort(int* a, int first, int last)
{
	struct sort_com
	{
		int l, r;
	};

	queue<sort_com> q;
	q.push({ first, last });
	while (q.size())
	{
		sort_com t = q.front();
		q.pop();
		
		int bel;
		int mi = min(min(a[t.l], a[t.r]), a[(t.l + t.r) / 2]);
		int ma = min(min(a[t.l], a[t.r]), a[(t.l + t.r) / 2]);
		if ((a[t.l] > mi) && (a[t.l] < ma))
			bel = a[t.l];
		else if ((a[t.r] > mi) && (a[t.r] < ma))
			bel = a[t.r];
		else
			bel = a[(t.l + t.r) / 2];

		int l = t.l, r = t.r;
		while (l <= r)
		{
			while (a[l] < bel)
				++l;
			while (a[r] > bel)
				--r;
			if (l >= r)
				break;
			swap(a[l++], a[r--]);
		}
		if (t.l < r)
			q.push({ t.l,r });
		if (r + 1 < t.r)
			q.push({ r + 1,t.r });
	}
}

int main()
{
	int size = 100000000;
	int *a1 = new int[size];
	int *a2 = new int[size];
	int *a3 = new int[size];
	random_device ra;
	mt19937 rand(time(0));
	uniform_int_distribution<int> random(INT_MIN, INT_MAX);
	for (int i = 0; i < size; ++i)
		a1[i] = a2[i] = a3[i] = random(rand);

	double t1 = omp_get_wtime();
	sort(a1, a1 + size);
	double t2 = omp_get_wtime();
	quicksort(a2, 0, size - 1);
	double t3 = omp_get_wtime();
	tbb_qsort(a3, 0, size - 1);
	double t4 = omp_get_wtime();

	bool fl = 0;
	for (int i = 0; i < size; ++i)
		fl |= (a1[i] != a2[i]) | (a1[i] != a3[i]);
	if (fl)
		cout << "wrong\n";
	else
		cout << "correct\n";
	cout << "std:   " << (t2 - t1) << '\n';
	cout << "simple:   " << (t3 - t2) << '\n';
	cout << "tbb: " << (t4 - t3) << '\n';
}
