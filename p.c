#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#define MAX_NUM 10
#define MAX_THREADS 4
struct thread_info_type
{
	int *inputArr, *lsumArr, *rsumArr, *mirrorArr;
	int start, end, leaderId, threadsInPartition, totalElementsInPartition; 
	pthread_barrier_t *commonBarr;
	pthread_barrier_t *ownBarr;
	int *pivotElementArr;

};
struct thread_local_type
{
	int myId;
	struct thread_info_type * threadInfoArr;
};
int compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}

void * parallel_quick_sort(void *);
int *pqsort(int* inputArr, int numElements, int numThreads)
{
	int *lsumArr = (int *)malloc(sizeof(int)*numThreads);
	int *rsumArr = (int *)malloc(sizeof(int)*numThreads);
	pthread_barrier_t * commonBarr =(pthread_barrier_t *)malloc(sizeof(pthread_barrier_t)*numThreads) ;
	pthread_barrier_t * ownBarr =(pthread_barrier_t *)malloc(sizeof(pthread_barrier_t)*numThreads) ;
	int *mirrorArr = (int *)malloc(sizeof(int)*numElements);
	struct thread_info_type *threadInfoArr = (struct thread_info_type *)malloc(sizeof(struct thread_info_type)*numThreads);
	int *pivotElementArr = (int *)malloc(sizeof(int)*numThreads);
	int *pivotIndexArr = (int *)malloc(sizeof(int)*numThreads);
	pthread_t *p_threads= (pthread_t *)malloc(sizeof(pthread_t)*numThreads);
	struct thread_local_type * threadLocalArr = (struct thread_local_type *)malloc(sizeof(struct thread_local_type)*numThreads);
	int i;
	for(i=0;i<numThreads;i++)
	{
		pthread_barrier_init(&ownBarr[i], NULL, 2);
	}
	pthread_barrier_init(&commonBarr[0], NULL, numThreads);
	int elementsPerThread = numElements/numThreads;
	int excessElements = numElements%numThreads;
	int lastIndex = 0;
	for(i=0;i<numThreads;i++)
	{
		threadInfoArr[i].totalElementsInPartition = numElements;
		threadInfoArr[i].start = lastIndex;
		threadInfoArr[i].leaderId = 0; // initially thread 0 is leader
		threadInfoArr[i].inputArr = inputArr;
		threadInfoArr[i].lsumArr = lsumArr;
		threadInfoArr[i].rsumArr = rsumArr;
		threadInfoArr[i].mirrorArr = mirrorArr;
		threadInfoArr[i].commonBarr = commonBarr;
		threadInfoArr[i].ownBarr = ownBarr;
		threadInfoArr[i].threadsInPartition = numThreads;
		threadInfoArr[i].pivotElementArr = pivotElementArr;
		lastIndex+= (elementsPerThread -1);
		if(excessElements > 0)
		{
			lastIndex++;
			excessElements--;
		}
		threadInfoArr[i].end = lastIndex;
		lastIndex++; 
	}
	for(i=0;i<numThreads;i++)
	{
		threadLocalArr[i].myId = i;
		threadLocalArr[i].threadInfoArr = threadInfoArr;
		pthread_create(&p_threads[i],NULL,parallel_quick_sort,&threadLocalArr[i]);
	}
	for (i=0; i< numThreads; i++)
		pthread_join(p_threads[i], NULL);
	return inputArr;
}

void * parallel_quick_sort(void * arg)
{
	int i;
	struct thread_local_type threadLocal = *((struct thread_local_type *)arg);
	struct thread_info_type * threadInfoArr = threadLocal.threadInfoArr;
	int *inputArr = threadInfoArr[threadLocal.myId].inputArr;
	int *lsumArr = threadInfoArr[threadLocal.myId].lsumArr;
	int *rsumArr = threadInfoArr[threadLocal.myId].rsumArr;
	int *mirrorArr = threadInfoArr[threadLocal.myId].mirrorArr;
	int *pivotElementArr = threadInfoArr[threadLocal.myId].pivotElementArr;
	int myId = threadLocal.myId;
	pthread_barrier_t *commonBarr = threadInfoArr[threadLocal.myId].commonBarr;
	pthread_barrier_t *ownBarr = threadInfoArr[threadLocal.myId].ownBarr;
	int *currentArr, *currentMirrorArr, start,end,leaderId,numElements,totalElementsInPartition,threadsInPartition;
	int ping=1;
	struct thread_info_type threadInfo;
	int localMedian, *a,n,k, pivotElement, firstIndex,secondIndex,
numLessElements,numGreaterElements,lessStartIndex,greaterStartIndex,
offset,targetLessStartIndex,targetGreaterStartIndex,
totalLessElements, totalGreaterElements, lessThreads, greaterThreads,
lastIndex,elementsPerThread,excessElements, temp;
	while(1)
	{
		threadInfo = threadInfoArr[threadLocal.myId];
		start = threadInfo.start; // start element Index
		end = threadInfo.end; // end element Index
		leaderId = threadInfo.leaderId;
		numElements = end-start+1;
		totalElementsInPartition = threadInfo.totalElementsInPartition;
		threadsInPartition = threadInfo.threadsInPartition;
		if(ping)
		{
			currentArr = inputArr;
			currentMirrorArr = mirrorArr;
		}
		else
		{
			currentArr = mirrorArr;
			currentMirrorArr = inputArr;
		}

		if(threadsInPartition == 1) // terminating condition
		{
			if(currentArr != inputArr)
			{
				memcpy(inputArr+start,mirrorArr+start,sizeof(int)*(numElements));
			}
			qsort(inputArr+start,numElements,sizeof(int),compare);
			break;
		}
		a=currentArr+start;
		n = numElements;
		if(numElements & 1)
		{
			k = numElements/2;
		}
		else
		{
			k = numElements/2-1;
		}
		{
			register int i,j,l,m, x, temp;
			l=0 ; m=n-1 ;
			while (l<m) {
				x=a[k] ;
				i=l ;
				j=m ;
				do {
					while (a[i]<x) i++ ;
					while (x<a[j]) j-- ;
					if (i<=j) {
						temp = a[i];
						a[i] = a[j];
						a[j] = temp;
						i++ ; j-- ;
					}
				} while (i<=j) ;
				if (j<k) l=i ;
				if (k<i) m=j ;
			}
		}
		localMedian = a[k];
		pivotElementArr[myId] = localMedian;
		pthread_barrier_wait(&commonBarr[leaderId]);
		int *localPivotArr = (int *)malloc(sizeof(int)*threadsInPartition);
		memcpy(localPivotArr,pivotElementArr+leaderId,threadsInPartition*sizeof(int));
		a = localPivotArr;
		n = threadsInPartition;
		if(threadsInPartition&1)
			k=threadsInPartition/2;
		else
			k=threadsInPartition/2-1;
		{
			register int i,j,l,m, x, temp;
			l=0 ; m=n-1 ;
			while (l<m) {
				x=a[k] ;
				i=l ;
				j=m ;
				do {
					while (a[i]<x) i++ ;
					while (x<a[j]) j-- ;
					if (i<=j) {
						temp = a[i];
						a[i] = a[j];
						a[j] = temp;
						i++ ; j-- ;
					}
				} while (i<=j) ;
				if (j<k) l=i ;
				if (k<i) m=j ;
			}
		}
		pivotElement = a[k];
		free(localPivotArr);
		firstIndex = start;
		secondIndex = start;
		while(secondIndex <= end)
		{
		if(currentArr[secondIndex] <= pivotElement)
			{
				temp = currentArr[secondIndex];
				currentArr[secondIndex] = currentArr[firstIndex];
				currentArr[firstIndex] = temp;
				firstIndex++;
			}
			secondIndex++;
		}numLessElements = firstIndex - start;
		numGreaterElements = numElements - numLessElements;
		lessStartIndex = start;
		greaterStartIndex = firstIndex;
		lsumArr[myId] = numLessElements;
		rsumArr[myId] = numGreaterElements;
		pthread_barrier_wait(&commonBarr[leaderId]);
		if(myId == leaderId)
		{
			for(i=leaderId+1;i < (leaderId+threadsInPartition);i++)
			{
				lsumArr[i]+=lsumArr[i-1];
				rsumArr[i]+=rsumArr[i-1];
			}

		}
		pthread_barrier_wait(&commonBarr[leaderId]);
		offset = threadInfoArr[leaderId].start;
		targetLessStartIndex = lsumArr[myId]-numLessElements;
		targetGreaterStartIndex = lsumArr[leaderId+threadsInPartition-1]+rsumArr[myId]-numGreaterElements;
		memcpy(currentMirrorArr+offset+targetLessStartIndex,currentArr+lessStartIndex,numLessElements*sizeof(int));
		memcpy(currentMirrorArr+offset+targetGreaterStartIndex,currentArr+greaterStartIndex,numGreaterElements*sizeof(int));
		pthread_barrier_wait(&commonBarr[leaderId]);
totalLessElements = lsumArr[leaderId+threadsInPartition-1];
		totalGreaterElements = rsumArr[leaderId+threadsInPartition-1];
		lessThreads = round((double)totalLessElements / (totalElementsInPartition-1) * threadsInPartition);
		if(totalLessElements>0 && lessThreads < 1)
			lessThreads=1;
		if(lessThreads == threadsInPartition)
			lessThreads--;
		greaterThreads = threadsInPartition - lessThreads;
		if(greaterThreads == threadsInPartition)
		{
			lessThreads++;
			greaterThreads--;
		}
		if(myId == leaderId)
		{
			lastIndex = start;
			if(lessThreads!=0)
			{
				elementsPerThread = totalLessElements/lessThreads;
				excessElements = totalLessElements%lessThreads;

				for(i=leaderId;i<(leaderId+lessThreads);i++)
				{
					threadInfoArr[i].start = lastIndex;
					lastIndex+= (elementsPerThread -1);
					if(excessElements > 0)
					{
						lastIndex++;
						excessElements--;
					}
					threadInfoArr[i].end = lastIndex;
					lastIndex++; 
threadInfoArr[i].threadsInPartition = lessThreads;
					threadInfoArr[i].totalElementsInPartition = totalLessElements;
				}
				pthread_barrier_destroy(&commonBarr[leaderId]);
				pthread_barrier_init(&commonBarr[leaderId], NULL, lessThreads);
			}

			if(greaterThreads!=0)
			{

				elementsPerThread = totalGreaterElements/greaterThreads;
				excessElements = totalGreaterElements%greaterThreads;
				int greaterPivotPosition = lastIndex;

				for(i=(leaderId+lessThreads);i<(leaderId+lessThreads+greaterThreads);i++)
				{
					threadInfoArr[i].start = lastIndex;
					lastIndex+= (elementsPerThread -1);
					if(excessElements > 0)
					{
						lastIndex++;
						excessElements--;
					}
					threadInfoArr[i].end = lastIndex;
					lastIndex++; 
					threadInfoArr[i].threadsInPartition = greaterThreads;
					threadInfoArr[i].totalElementsInPartition = totalGreaterElements;
					threadInfoArr[i].leaderId = leaderId+lessThreads;
				}
				pthread_barrier_init(&commonBarr[leaderId+lessThreads], NULL, greaterThreads);
			}
			for(i=leaderId+1;i < (leaderId+threadsInPartition);i++)
				pthread_barrier_wait(&ownBarr[i]);
		}
		else
			pthread_barrier_wait(&ownBarr[myId]);

		if(ping==1)
			ping=0;
		else
			ping=1;
	}
}

int main()
{
	int *inputArr = (int *)malloc(sizeof(int)*MAX_NUM);
	assert(inputArr!=NULL);
	srand(time(NULL));
	int i;
	for(i=0;i<MAX_NUM;i++)
	{
		inputArr[i] = rand() % 1000000;
	}
	struct timeval tz;
	struct timezone tx;
	double start_time, end_time;
	gettimeofday(&tz, &tx);
	start_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	pqsort(inputArr,MAX_NUM,MAX_THREADS);
	gettimeofday(&tz, &tx);
	end_time = (double)tz.tv_sec + (double) tz.tv_usec / 1000000.0;
	double stime = ((double)end_time - (double)start_time);
}