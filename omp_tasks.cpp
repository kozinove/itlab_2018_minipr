#include <omp.h>
#include <algorithm>
#include <stdio.h>

void QuickSort(int* a, const long n) {
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
  if (j > 0) quickSort(a, j);
  if (n > i) quickSort(a + i, n - i);
}

void OMP_QuickSort(int* a, const long n) {
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
		if (j > 0) quickSort(a, j);
	} 
#pragma omp task shared(a)
	{
		if (n > i) quickSort(a + i, n - i);
	} 
#pragma omp taskwait
}


int main(int argc, char *argv[]) {
	int *a,b;
	double t1, t2, t3;
	int n = 10000000;
	a = new int[n];
	b = new int[n];
	for (int i = 0; i < n; ++i){
		a[i] = rand() % 100;
		b[i] = a[i];
	}
	// последовательная
	t1 = omp_get_wtime();
	QuickSort(a, n - 1);
	t2 = omp_get_wtime();
	// последовательная
	
#pragma omp parallel shared(a)
	{
		#pragma omp single nowait 
		{
			OMP_QuickSort(b, n - 1);
		} 
	} 

	t3 = omp_get_wtime();

	std::cout<<t2-t1<<" "<<t3-t2<<"\n";

  delete []a;
  return 0;
}
