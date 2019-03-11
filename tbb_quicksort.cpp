#include <iostream>
#include <queue>
#include <utility>
#include <cmath>
#include <ctime>
#include <random>
#include <queue>
#include <utility>
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_sort.h"
#include "tbb/tick_count.h"

const int N = 100000000;



class quicksort :public tbb::task
{
	int l, r;
	int* a;
public:
	quicksort(int *_a, int _l, int _r): a(_a), l(_l), r(_r) {}
	task* execute()
	{
		int i = l, j = r, x = a[(l + r) / 2];
		if (j - i >= 0 && j - i < 50)
		{
			for (int k = i; k <= j; k++)
				for (int l = k + 1; l <= j; l++)
					if (a[k] > a[l])
						std::swap(a[k], a[l]);
			return NULL;
		}
		while (i <= j)
		{
			while (a[i] < x)
				i++;
			while (a[j] > x)
				j--;
			if (i <= j)
			{
				if (i < j)
					std::swap(a[i], a[j]);
				i++;
				j--;
			}
		}
		int refcount = 0;
		if (j > l)
		{
			tbb::empty_task& c = *new(allocate_continuation()) tbb::empty_task;
			quicksort &t1 = *new(c.allocate_child()) quicksort(a, l, j);
			c.set_ref_count(c.ref_count()+1);
			c.spawn(t1);
		}
		if (i < r)
		{
			tbb::empty_task& c = *new(allocate_continuation()) tbb::empty_task;
			quicksort &t2 = *new(c.allocate_child()) quicksort(a, i, r);
			c.set_ref_count(c.ref_count() + 1);
			c.spawn(t2);
		}
		return NULL;
	}
};

void usualquicksort(int *a, int first, int last)
{

	std::queue<std::pair<int, int>>q;
	q.push({ first, last });
	while (!q.empty())
	{
		std::pair<int, int> cur = q.front();
		q.pop();
		int i = cur.first, j = cur.second, x = a[(cur.first + cur.second) / 2];
		while (i <= j)
		{
			while (a[i] < x)
				i++;
			while (a[j] > x)
				j--;
			if (i <= j)
			{
				if (i < j)
					std::swap(a[i], a[j]);
				i++;
				j--;
			}
		}
		if (i < cur.second)
			q.push({ i, cur.second });
		if (j > cur.first)
			q.push({ cur.first, j });
	}
}

int main()
{
	int *a = new int[N];
	int *b = new int[N];
	std::mt19937 mt = std::mt19937(std::time(0));
	std::uniform_int_distribution<int> rand = std::uniform_int_distribution<int>(-N, N);
	for (int i = 0; i < N; i++)
		a[i] = b[i] = rand(mt);
	tbb::task_scheduler_init init;
	std::cout << "!";
	tbb::tick_count t1 = tbb::tick_count::now();
	quicksort &q = *new(tbb::task::allocate_root()) quicksort(a, 0, N-1);
	tbb::task::spawn_root_and_wait(q);
	tbb::tick_count t2 = tbb::tick_count::now();
	std::cout << (t2 - t1).seconds() << " ";
	usualquicksort(b, 0, N-1);
	tbb::tick_count t3 = tbb::tick_count::now();
	std::cout << (t3 - t2).seconds() << "\n";
	//for (int i = 0; i < N; i++)
	//	std::cout << a[i] << " " << b[i]<<"\n";
	for (int i = 0; i < N; i++)
	{
		//std::cout << a[i] << " " << b[i] << "\n";
		if (a[i] != b[i])
		{
			std::cout << "ALYARMA!!!\n";
			std::cout << i << " " << a[i] << " " << b[i];
			system("pause");
			return 0;
		}
	}
	delete[] a, b;
	system("pause");
	return 0;
}
