#pragma warning(disable:4996)
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

int Rank_Waiting_Time[6] = { 0, 1, 2, 3, 4, 5 };
int Rank_Turnaround_Time[6] = { 0, 1, 2, 3, 4, 5 };
int Rank_CPU_Utilization[6] = { 0, 1, 2, 3, 4, 5 };
float Average_Waiting_Time[6], Average_Turnaround_Time[6], CPU_Utilization[6];
int left_IO[6][5];

typedef struct {
	int Process_ID;
	int CPU_Burst_Time;
	int IO_Burst_Time;
	int Arrival_Time;
	int Priority;
	int IO_Burst_Timing;
}Process;

typedef struct {
	int* ID;
	int cnt;
}QUEUE;

QUEUE* Create_Queue(void) {
	QUEUE* buf = (QUEUE*)malloc(sizeof(QUEUE));
	buf->ID = (int*)malloc(sizeof(int) * 1000);
	buf->cnt = 0;
	return buf;
}

void Enqueue(QUEUE* queue, int name) {
	*(queue->ID + (queue->cnt)) = name;
	queue->cnt++;
}

void Dequeue(QUEUE* queue) {
	queue->cnt--;
	memmove(queue->ID, queue->ID + 1, sizeof(int) * (queue->cnt));
}