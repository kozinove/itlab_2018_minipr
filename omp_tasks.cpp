#include <omp.h>
#include <algorithm>
#include <iostream>
#include <stdlib.h>

void QuickSort(int* a, const int n) {
  int i = 0, j = n;
  int pivot = a[n / 2];
  do {
    while (a[i] < pivot) i++;
    while (a[j] > pivot) j--;
    if (i <= j) {
      std::swap(a[i], a[j]);
      i++; j--;
    }
  } while (i <= j);
  if (j > 0) QuickSort(a, j);
  if (n > i) QuickSort(a + i, n - i);
}

void OMP_QuickSort(int* a, const int n) {
	int i = 0, j = n;
	int pivot = a[n / 2];
	
	do {
		while (a[i] < pivot) i++;
		while (a[j] > pivot) j--;

		if (i <= j) {
      std::swap(a[i], a[j]);
			i++; j--;
		}
	} while (i <= j);

#pragma omp task shared(a)
	{
		if (j > 0) OMP_QuickSort(a, j);
	} 
#pragma omp task shared(a)
	{
		if (n > i) OMP_QuickSort(a + i, n - i);
	} 
#pragma omp taskwait
}


int main(int argc, char** argv) {
	int *a,*b;
	double t1, t2, t3;
	int n = atoi (argv[1]);
	a = new int[n];
	b = new int[n];
	for (int i = 0; i < n; ++i){
		a[i] = rand() % 100;
		b[i] = a[i];
	}
	// последовательная
	t1 = omp_get_wtime();
	QuickSort(b, n - 1);
	t2 = omp_get_wtime();
	// последовательная
	
#pragma omp parallel shared(a)
	{
		#pragma omp single nowait 
		{
			OMP_QuickSort(a, n - 1);
		} 
	} 

	t3 = omp_get_wtime();

	std::cout<<t2-t1<<" "<<t3-t2<<"\n";

  delete []a;
  return 0;
}