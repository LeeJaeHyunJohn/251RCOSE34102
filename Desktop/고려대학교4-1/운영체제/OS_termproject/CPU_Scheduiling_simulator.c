#pragma warning(disable:4996)
#include <stdio.h>    // 입출력: printf, scanf
#include <string.h>   // 문자열 처리 함수들: strcpy, strcmp 등
#include <math.h>     // 수학 함수들: sqrt, pow 등
#include <stdlib.h>   // rand, malloc, exit 등
#include <time.h>     // 시간 관련 함수들: time, srand


int Rank_Waiting_Time[6] = { 0, 1, 2, 3, 4, 5 };
int Rank_Turnaround_Time[6] = { 0, 1, 2, 3, 4, 5 };
int Rank_CPU_Utilization[6] = { 0, 1, 2, 3, 4, 5 };
float Average_Waiting_Time[6], Average_Turnaround_Time[6], CPU_Utilization[6];
int left_IO[6][5];//6개 알고리즘 * 5개 프로세스

//process 구조체 정의
typedef struct {
	int Process_ID;
	int CPU_Burst_Time;
	int IO_Burst_Time;
	int Arrival_Time;
	int Priority;
	int IO_Burst_Timing;
}Process;

//QUEUE 구조체 정의
typedef struct {
	int* ID;
	int cnt;
}QUEUE;

//QUEUE 구조체는 실행 중 만들어지기 때문에 이름 붙여서 딱 정해진 변수로 쓸 수 없다. 포인터로 주소만 저장
QUEUE* Create_Queue(void) {//새로운 QUEUE 하나를 동적으로 만들어서 그 주소를 리턴(프로그램 실행 도중 '줄' 하나 새로 만들고 돌려주는 역할)
//QUEUE* : 이 함수는 QUEUE 구조체를 가리키는 포인터 (주소)를 반환한다	
	// 종이 한 장으로 줄 서는 명단판을 만들어서 buf라는 이름표 붙여둠
	QUEUE* buf = (QUEUE*)malloc(sizeof(QUEUE));//메모리 공간을 QUEUE 하나 크기만큼 요청해서 새로 만든다//QUEUE* buf는 새로 만든 큐의 주소를 저장하는 포인터
	// 줄판 안에 1000명까지 줄 설 수 있는 칸 마련.//줄 서는 명단판 하나를 메모리에 새로 만들고, 그 이름표(buf)를 붙인 것으로 이해
	buf->ID = (int*)malloc(sizeof(int) * 1000);//큐 안에 있는 ID는 사람들 번호를 담을 배열의 주소. 이 배열이 줄에 서 있는 사람들 번호를 저장하는 공간. 
	buf->cnt = 0;//줄에 서 있는 사람 수를 0으로 초기화//방금 만들어서 아무도 없음
	return buf;//만든 큐를 바깥에 넘겨줌(만든 구조체 주소를 함수 밖으로 넘겨주기)
}

void Enqueue(QUEUE* queue, int name) {//queue는 줄을 나타내는 구조체, *가 붙었으니 queue라는 줄의 주소를 받겠다
	*(queue->ID + (queue->cnt)) = name;//줄 맨뒤 cnt 번째에 사람 번호(name)을 넣는다
	queue->cnt++;//한 명 추가했으니 줄에 있는 사람 수 1 증가
}

void Dequeue(QUEUE* queue) {//줄 맨 앞에 있는 사람 한 명을 제거하고, 나머지 사람들을 앞으로 한 칸씩 당기는 함수
	queue->cnt--;
	memmove(//맨 앞사람 빼고 뒷사람들 한 칸씩 당기기
		queue->ID, //대상(어디에)
		queue->ID + 1, //원본(어디서부터)
		sizeof(int) * (queue->cnt)); //얼마나(복사할 바이트 수)
}
//process 구조체 하나 만들어서 리턴하는 함수
Process Create_Process(int ID) {
	Process buf;//프로세스 정보 담을 구조체 변수

	buf.Process_ID = ID; //프로세스는 고유 번호이므로 random x
	buf.CPU_Burst_Time = rand() % 20 + 1; //1~20 사이의 랜덤 숫자
	buf.IO_Burst_Time = rand() % 5 + 1;
	buf.Arrival_Time = rand() % 20 + 1;
	buf.Priority = rand() % 10 + 1;
	buf.IO_Burst_Timing = rand() % buf.CPU_Burst_Time;//CPU 작업 중 언제쯤 I/O가 발생할지

	return buf;
}

Process Copy_Process(Process process) {//원본 프로세스 복사
	Process buf;

	buf.Process_ID = process.Process_ID;
	buf.CPU_Burst_Time = process.CPU_Burst_Time;
	buf.IO_Burst_Time = process.IO_Burst_Time;
	buf.Arrival_Time = process.Arrival_Time;
	buf.Priority = process.Priority;
	buf.IO_Burst_Timing = process.IO_Burst_Timing;

	return buf;
}

void Print_Process(Process process_Info[], int Process_Cnt) {

	for (int i = 0; i < Process_Cnt; i++) {
		printf("Process %d: \n", i + 1);//화면에 1번부터 표시되게
		printf("CPU_Burst_Time = %d\n", process_Info[i].CPU_Burst_Time);
		printf("IO_Burst_Time = %d\n", process_Info[i].IO_Burst_Time);
		printf("Arrival_Time = %d\n", process_Info[i].Arrival_Time);
		printf("Priority = %d\n", process_Info[i].Priority);
		printf("IO_Burst_Timing = %d\n", process_Info[i].IO_Burst_Timing);
		printf("\n");
	}
}
//CPU_Info는 2차원 배열. CPU_Info[0][i]: 해당 구간의 종료 시간. CPU_Info[1][i]: 해당 구간에서 실행된 프로세스 번호 (또는 -1이면 IDLE). 
void Print_Gantt_Chart(int** CPU_Info, int flag) {//flag : 간트차트에 몇 칸 그릴지
	for (int i = 0; i < flag; i++) printf("|      ");
	printf("|\n");//첫줄 오른쪽 끝 바깥 테두리
	for (int i = 0; i < flag; i++) {
		if (CPU_Info[1][i] == -1) printf("| IDLE ");
		else printf("|  P%d  ", CPU_Info[1][i]);
	}//각 칸 안에 실행 중인 프로세스 번호 출력 ex.|  P1  |  P2  | IDLE |  P3  |
	printf("|\n");//가운데 줄 오른쪽 끝 바깥 테두리
	for (int i = 0; i < flag; i++) printf("|      ");
	printf("|\n");//세번째(마지막) 줄 오른쪽 끝 바깥 테두리
	printf("0      ");
	for (int i = 0; i < flag; i++) {
		if (CPU_Info[0][i] < 10) printf("%d      ", CPU_Info[0][i]);
		else printf("%d     ", CPU_Info[0][i]);
	}//밑에 시간 숫자 출력되게 

	printf("\n\n");
}

float Calculate_Average_Waiting_Time(Process Process_Info[], int Process_Cnt, int** CPU_Info, int flag, int left_IO_flag) {
//프로세스 정보 배열,프로세스 개수,간트 차트 정보,간트 차트 구간 수,left_IO 사용 시 인덱스 용	
	int** Keep_Value = (int**)malloc(sizeof(int*) * 2);//계산 중간 값 저장용 배열
	int Total = 0; //모든 프로세스 대기 시간 총합
	int Future_Time = 0; //현재 구간의 종료 시각
	int Currunt_Time = 0; //현재 구간의 시작 시각
//2행짜리 배열 초기화
	for (int i = 0; i < 2; i++) 
			Keep_Value[i] = (int*)malloc(sizeof(int) * Process_Cnt);

	for (int i = 0; i < Process_Cnt; i++) {
		Keep_Value[0][i] = 0;//i번째 프로세스의 누적 대기 시간 0으로 초기화
		Keep_Value[1][i] = 0;//i번째 프로세스가 마지막으로 CPU에서 빠진 시각 저장
	}
	for (int i = 0; i < flag; i++) {//간트 차트의 각 구간 훑기
		if (i == 0) Currunt_Time = 0;//i=0일 때는 당연히 시작 시간=0
		else Currunt_Time = CPU_Info[0][i - 1];//그 외의 경우는 이전 구간의 종료 시각=현재 구간의 시작 시각

		Future_Time = CPU_Info[0][i];
		if (CPU_Info[1][i] != -1) {//이 구간에 CPU 사용 프로세스가 있다면?(IDLE은 건너뛰자)
				int pid = CPU_Info[1][i]-1;//배열 인덱스랑 pid 맞추기 위해 -1			
				if (Keep_Value[1][pid] == 0) {//첫실행인 경우
    			//현재 시각 - 도착한 시각
					Keep_Value[0][pid] = Currunt_Time - Process_Info[pid].Arrival_Time;
    			//종료 시각 저장하기
					Keep_Value[1][pid] = Future_Time;
				} else {//두 번째 이후 실행인 경우
					//이전 종료 시점부터 현재 시작 시점까지의 대기 시간 누적
    			Keep_Value[0][pid] += (Currunt_Time - Keep_Value[1][pid]);
					//이번 CPU를 마친 시간으로 갱신
    			Keep_Value[1][pid] = Future_Time;
				}
		}
	}
	//average waiting time을 출력하는 파트
	for (int i = 0; i < Process_Cnt; i++) {
		if(Process_Info[i].IO_Burst_Timing > 0) Keep_Value[0][i] -= (Process_Info[i].IO_Burst_Time + left_IO[left_IO_flag][i]);
	}//대기 시간 보정(I/O 작업을 하러 갔기에 CPU를 기다린 게 아님)
	printf("Average Waiting Time :\n");
	printf("( ");
	for (int i = 0; i < Process_Cnt; i++) {
		if (i != Process_Cnt - 1)//마지막 프로세스가 아닐 때 
				printf("%d + ", Keep_Value[0][i]);
		else //마지막 항목일 경우
				printf("%d )", Keep_Value[0][i]);
		Total += Keep_Value[0][i];
	}

	float ans = (float)Total / Process_Cnt;

	printf(" / %d = %.1f\n\n", Process_Cnt, ans);
	return ans;
}

float Calculate_Average_Turnaround_Time(Process Process_Info[], int Process_Cnt, int** CPU_Info, int flag) {
	//각 프로세스의 Turnaround Time 저장할 배열 생성
	int* Keep_Value = (int*)malloc(sizeof(int) * Process_Cnt);
	int Total = 0;
	//배열 초기화하기
	for (int i = 0; i < Process_Cnt; i++) Keep_Value[i] = 0;
	//Gantt 차트 돌면서 마지막으로 실행된 시점 저장
	for (int i = 0; i < flag; i++) 
			if (CPU_Info[1][i] != -1) 
					Keep_Value[CPU_Info[1][i] - 1] = CPU_Info[0][i];
	//종료 시간 - 도착 시간 = 반환시간
	for (int i = 0; i < Process_Cnt; i++) 
			Keep_Value[i] -= Process_Info[i].Arrival_Time;

	//for (int i = 0; i < Process_Cnt; i++) Keep_Value[i] -= Process_Info[i].IO_Burst_Time;

	//출력 파트
	printf("Average Turnaround Time :\n");
	printf("( ");
	for (int i = 0; i < Process_Cnt; i++) {
		if (i != Process_Cnt - 1) printf("%d + ", Keep_Value[i]);
		else printf("%d )", Keep_Value[i]);
		Total += Keep_Value[i];
	}

	float ans = (float)Total / Process_Cnt;
	printf(" / %d = %.1f\n\n", Process_Cnt, ans);
	return ans;
}//ex. (7 + 10 + 5 ) / 3 = 7.3

float Calculate_CPU_Utilization(int** CPU_Info, int flag) {//flag=간트차트 칸수..
	int CPU_Idle = 0;
	int Last_Time = CPU_Info[0][flag - 1];//간트차트 마지막 시각
	//if
	//CPU_Info[0] = [3, 6, 9, 13]→ Last_Time = 13
	for (int i = 0; i < flag; i++) {
		if (CPU_Info[1][i] == -1) {//IDLE 상태일 경우
			if (i == 0)//첫구간(시작 시각=0)이면 종료시간 자체가 IDLE 시간 
				CPU_Idle += CPU_Info[0][i];
			else //두 번째 구간 이후라면 이전 종료와 현재 종료의 차이
				CPU_Idle += CPU_Info[0][i] - CPU_Info[0][i - 1];
		}
	}
	//전체에서 논 시간 빼면 일한 시간ㅇㅇ
	int CPU_Not_Idle = Last_Time - CPU_Idle;
	float ans = (float)CPU_Not_Idle / Last_Time;

	printf("CPU Utilization :\n");
	printf("%d / %d = %.0f%%\n\n", CPU_Not_Idle, Last_Time, ans * 100);

	return ans;
}
//선입선출
void FCFS(Process Process_Info[], int Process_Cnt) {
	//준비 큐(CPU 올라가려고 기다리는 프로세스들), 대기 큐 생성(I/O 기다리는 프로세스들)
	QUEUE* Ready_Queue = Create_Queue();
	QUEUE* Waiting_Queue = Create_Queue();
	//프로세스 정보 복사할 공간 확보(원본 건드리지 않기 위해)
	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);
	//원본 프로세스 하나씩 복사
	for (int i = 0; i < Process_Cnt; i++) 
			process[i] = Copy_Process(Process_Info[i]);

	int Process_End_Cnt = 0;//끝난 애들 개수 초기화
	int flag = 0;//총 구간 수 초기화
	//간트 차트 저장하는 2차원 배열//[0][i]=끝나는 시각,[1][i]=프로세스 번호
	int** CPU_Info = (int**)malloc(sizeof(int) * 2);

	for (int i = 0; i < 2; i++) 
			CPU_Info[i] = (int*)malloc(sizeof(int) * 1000);

	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;//현재 시뮬레이션 초

	//현재 시간에 도착한 프로세스를 Ready Queue에 넣는다
	while (Process_End_Cnt < Process_Cnt) {
		for (int i = 0; i < Process_Cnt; i++) 
				if (CPU_Time == process[i].Arrival_Time) 
						Enqueue(Ready_Queue, i + 1);

		CPU_Time++;

		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) 
				CPU_Idle_Time++;//cpu도 놀고 있고, 줄에도 아무도 없으면 그냥 쉬는중
		else {//ReadyQueue에 누군가 있거나 CPU 누군가 실행중이라면?
			if (Currunt_CPU_Process == -1) {//CPU 비어있고, 누군가 대기 중이라면?
				if (CPU_Idle_Time != 0) {//이전에 쉬고 있던 시간 있었다면 차트에 기록해야함
					CPU_Info[0][flag] = CPU_Time - 1;//종료 시각 기록
					CPU_Info[1][flag] = -1;//IDLE
					flag++;
					CPU_Idle_Time = 0;//기록 끝났으니 다시 0으로
				}
				Currunt_CPU_Process = Ready_Queue->ID[0];//대기 줄 맨 앞 프로세스를 CPU에 올린다
				Dequeue(Ready_Queue);//올라간 애 제거
			}
			//프로세스 실행->남은 작업 시간 줄이기
			process[Currunt_CPU_Process - 1].CPU_Burst_Time--;
			//작업시간이 0이 되었다(프로세스 할일 없을 때))
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {
				CPU_Info[0][flag] = CPU_Time;//종료시간
				CPU_Info[1][flag] = Currunt_CPU_Process;//프로세스 번호
				flag++;
				Currunt_CPU_Process = -1;//CPU 비우기
				Process_End_Cnt++;//종료 프로세스 1 증가
			}
			////아직 CPU 작업 안 끝났지만 I/O 요청 해야 하는지 검사하는 구간(남은 CPU 시간이 I/O 요청 시점과 같아졌나?)
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
				left_IO[0][Currunt_CPU_Process - 1] = -CPU_Time;//
				Enqueue(Waiting_Queue, Currunt_CPU_Process);//이 프로세스를 대기 큐에 넣는다
				CPU_Info[0][flag] = CPU_Time;//현재 시간=해당 구간의 종료 시간
				CPU_Info[1][flag] = Currunt_CPU_Process;//현재 프로세스 번호
				flag++;
				Currunt_CPU_Process = -1;//CPU에서 내려감
			}
		}
		
		// 1.지금 I/O 장치에서 작업 중인 프로세스가 있다면 그 프로세스의 I/O 남은 시간을 1초 줄인다
		// 2.I/O가 끝났다면 남은 CPU 작업이 있다면 다시 Ready Queue로 보내고, I/O 종료 처리
		// 3.지금 아무도 I/O 사용 중이 아닌데, Waiting Queue에 사람이 있다면 대기 줄에서 한 명 꺼내서 I/O 실행 시작
		if (Currunt_IO_Process != -1) 
				process[Currunt_IO_Process - 1].IO_Burst_Time--;
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			if(process[Currunt_IO_Process - 1].CPU_Burst_Time != 0) 
					Enqueue(Ready_Queue, Currunt_IO_Process);//남아 있다면 다시 올려보냄
			Currunt_IO_Process = -1;
		}
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt != 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];//맨 앞 프로세스 I/O에 올림
			left_IO[0][Currunt_IO_Process - 1] += CPU_Time;
			Dequeue(Waiting_Queue);//I/O 쓰기 시작했으니 대기줄에서 제거
		}

	}
	//결과 출력
	printf("FCFS:\n");
	Print_Gantt_Chart(CPU_Info, flag);

	float FCFS_Average_Waiting_Time = Calculate_Average_Waiting_Time(Process_Info, Process_Cnt, CPU_Info, flag, 0);
	float FCFS_Average_Turnaround_Time = Calculate_Average_Turnaround_Time(Process_Info, Process_Cnt, CPU_Info, flag);
	float FCFS_CPU_Utilization = Calculate_CPU_Utilization(CPU_Info, flag);
	//결과 저장
	Average_Waiting_Time[0] = FCFS_Average_Waiting_Time;
	Average_Turnaround_Time[0] = FCFS_Average_Turnaround_Time;
	CPU_Utilization[0] = FCFS_CPU_Utilization;

	/*
	free(Ready_Queue->ID);
	free(Waiting_Queue->ID);
	free(Ready_Queue);
	free(Waiting_Queue);
	free(process);
	for (int i = 0; i < 2; i++) free(CPU_Info[i]);
	free(CPU_Info);
	*/
}

//도착한 프로세스들 중 가장 작업 시간이 짧은 것 먼저 실행(비선점형)
void SJF(Process Process_Info[], int Process_Cnt) {
	QUEUE* Ready_Queue = Create_Queue();
	QUEUE* Waiting_Queue = Create_Queue();

	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);
	for (int i = 0; i < Process_Cnt; i++) process[i] = Copy_Process(Process_Info[i]);

	int Process_End_Cnt = 0;
	int flag = 0;

	int** CPU_Info = (int**)malloc(sizeof(int) * 2);
	for (int i = 0; i < 2; i++) CPU_Info[i] = (int*)malloc(sizeof(int) * 500);

	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;

	while (Process_End_Cnt < Process_Cnt) {
		for (int i = 0; i < Process_Cnt; i++) if (CPU_Time == process[i].Arrival_Time) {
			Enqueue(Ready_Queue, i + 1);
			for (int j = 0; j < Ready_Queue->cnt; j++) {
				for (int k = j + 1; k < Ready_Queue->cnt; k++) {
					if (process[Ready_Queue->ID[j] - 1].CPU_Burst_Time > process[Ready_Queue->ID[k] - 1].CPU_Burst_Time) {
						int temp = Ready_Queue->ID[j];
						Ready_Queue->ID[j] = Ready_Queue->ID[k];
						Ready_Queue->ID[k] = temp;
					}
				}
			}
		}

		CPU_Time++;

		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) CPU_Idle_Time++;
		else {
			if (Currunt_CPU_Process == -1) {
				if (CPU_Idle_Time != 0) {
					CPU_Info[0][flag] = CPU_Time - 1;
					CPU_Info[1][flag] = -1;
					flag++;
					CPU_Idle_Time = 0;
				}
				Currunt_CPU_Process = Ready_Queue->ID[0];
				Dequeue(Ready_Queue);
			}
			process[Currunt_CPU_Process - 1].CPU_Burst_Time--;
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
				if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
					left_IO[1][Currunt_CPU_Process - 1] = -CPU_Time;
					Enqueue(Waiting_Queue, Currunt_CPU_Process);
				}
				Currunt_CPU_Process = -1;
				Process_End_Cnt++;
			}
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
				left_IO[1][Currunt_CPU_Process - 1] = -CPU_Time;
				Enqueue(Waiting_Queue, Currunt_CPU_Process);
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
				Currunt_CPU_Process = -1;
			}
		}

		if (Currunt_IO_Process != -1) process[Currunt_IO_Process - 1].IO_Burst_Time--;
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			if (process[Currunt_IO_Process - 1].CPU_Burst_Time > 0) {
				Enqueue(Ready_Queue, Currunt_IO_Process);
				for (int i = 0; i < Ready_Queue->cnt; i++) {
					for (int j = i + 1; j < Ready_Queue->cnt; j++) {
						if (process[Ready_Queue->ID[i] - 1].CPU_Burst_Time > process[Ready_Queue->ID[j] - 1].CPU_Burst_Time) {
							int temp = Ready_Queue->ID[i];
							Ready_Queue->ID[i] = Ready_Queue->ID[j];
							Ready_Queue->ID[j] = temp;
						}
					}
				}
			}
			Currunt_IO_Process = -1;
		}
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt > 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];
			left_IO[1][Currunt_IO_Process - 1] += CPU_Time;
			Dequeue(Waiting_Queue);
		}
	}

	printf("SJF:\n");
	Print_Gantt_Chart(CPU_Info, flag);

	float SJF_Average_Waiting_Time = Calculate_Average_Waiting_Time(Process_Info, Process_Cnt, CPU_Info, flag, 1);
	float SJF_Average_Turnaround_Time = Calculate_Average_Turnaround_Time(Process_Info, Process_Cnt, CPU_Info, flag);
	float SJF_CPU_Utilization = Calculate_CPU_Utilization(CPU_Info, flag);

	Average_Waiting_Time[1] = SJF_Average_Waiting_Time;
	Average_Turnaround_Time[1] = SJF_Average_Turnaround_Time;
	CPU_Utilization[1] = SJF_CPU_Utilization;

	/*
	free(Ready_Queue->ID);
	free(Waiting_Queue->ID);
	free(Ready_Queue);
	free(Waiting_Queue);
	free(process);
	for (int i = 0; i < 2; i++) free(CPU_Info[i]);
	free(CPU_Info);
	*/
}