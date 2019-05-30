#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#define count 1000000
#define swap(v, a, b) {unsigned tmp; tmp=v[a]; v[a]=v[b]; v[b]=tmp;}
static int *array;
static void init() {
 int i;
 array = (int *) malloc(count*sizeof(int));
 for (i = 0; i < count; i++) {
 array[i] = rand();
}
}
static unsigned partArray(int *array, unsigned start, unsigned end, unsigned point) {
 if (point != start)
 swap(array, start, point);
 point = start;
 start++;
 while (start <= end) {
 if (array[start] <= array[point])
 start++;
 else if (array[end] > array[point])
 end--;
 else
 swap(array, start, end);
 }
 if (end != point)
 swap(array, point, end);
 return end;
}
static void auickSort(int *array, unsigned start, unsigned end) {

 unsigned pivot;

 if (start >= end)
 return;
 pivot = (start+end)/2;
 pivot = partArray(array, start, end, pivot);
 if (start < pivot)
 auickSort(array, start, pivot-1);
 if (pivot < end)
 auickSort(array, pivot+1, end);
}
int main(int argc, char **argv) {

 int i = 0;
 for(; i < 50; i++) {
 init();
 double start_time =  clock();
 auickSort(array, 0, count-1);
 double end_time = clock();
 double search_time = end_time - start_time;
 printf("%f \n", search_time/CLOCKS_PER_SEC);

 }
}
