#pragma warning(disable:4996)
#include <stdio.h>    // 입출력: printf, scanf
#include <string.h>   // 문자열 처리 함수들: strcpy, strcmp 등
#include <math.h>     // 수학 함수들: sqrt, pow 등
#include <stdlib.h>   // rand, malloc, exit 등
#include <time.h>     // 시간 관련 함수들: time, srand

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 전역 변수 및 평가 지표 배열 선언
// - Rank_*: 스케줄링 알고리즘 순위 저장용
// - Average_*, CPU_Utilization: 스케줄링 결과 기록용
// - left_IO: 알고리즘별 I/O 대기 시간 보정용
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Rank_Waiting_Time[6] = { 0, 1, 2, 3, 4, 5 };
int Rank_Turnaround_Time[6] = { 0, 1, 2, 3, 4, 5 };
int Rank_CPU_Utilization[6] = { 0, 1, 2, 3, 4, 5 };
float Average_Waiting_Time[6], Average_Turnaround_Time[6], CPU_Utilization[6];
int left_IO[6][5];//6개 알고리즘 * 5개 프로세스

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// Process 및 Queue 구조체 정의
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//process 구조체 정의
typedef struct {
	int Process_ID;
	int CPU_Burst_Time;
	int IO_Burst_Time;
	int Arrival_Time;
	int Priority;
	int IO_Burst_Timing;
} Process;

//QUEUE 구조체 정의
typedef struct {
	int* ID;
	int cnt;
} QUEUE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// Queue 관련 함수
// - Create_Queue: 빈 큐 생성
// - Enqueue: 큐에 삽입
// - Dequeue: 큐에서 제거
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// Process 관련 함수
// - Create_Process: 랜덤 속성으로 프로세스 생성
// - Copy_Process: 프로세스 복사
// - Print_Process: 프로세스 정보 출력
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 간트차트 출력 함수
// - CPU_Info[0][i] : 종료 시각
// - CPU_Info[1][i] : 프로세스 번호 (-1이면 IDLE)
// - flag : 구간 수
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 평균 대기 시간 계산 함수
// - Keep_Value[0][i]: 누적 대기 시간
// - Keep_Value[1][i]: 마지막으로 CPU 사용한 시각
// - I/O 보정 포함
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 평균 반환 시간 계산 함수
// - 종료 시각 - 도착 시각 = 반환 시간
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// CPU 활용률 계산 함수
// - 전체 시간 중 IDLE 시간을 제외한 비율 계산
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//선입선출
void FCFS(Process Process_Info[], int Process_Cnt) {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. 초기화
//   - Ready Queue, Waiting Queue 생성
//   - 프로세스 복사 및 상태 변수 초기화
//   - 간트차트 기록용 CPU_Info 배열 준비
//   - 시뮬레이션용 변수 초기화

// 2. while (모든 프로세스 종료 전까지 반복)
//   2-1. 프로세스 도착 확인 및 Ready Queue 삽입
//   2-2. CPU 시간 1 증가
//   2-3. CPU 상태에 따라 분기
//     a) 아무도 없고 Ready Queue도 비었으면 IDLE
//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
//     c) 현재 프로세스 1초 실행
//     d) 종료 시: 간트차트 기록, 종료 수 증가
//     e) I/O 요청 도달 시: I/O 큐로 이동
//   2-4. I/O 처리
//     a) 현재 I/O 프로세스 1초 수행
//     b) I/O 완료되면 Ready로 복귀 (CPU 작업 남은 경우)
//     c) 대기 중인 프로세스를 I/O 장치에 올림

// 3. 결과 출력 및 기록
//   - Gantt Chart 출력
//   - 평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//1. 초기화

	//1-1 준비 큐(CPU 올라가려고 기다리는 프로세스들), 대기 큐 생성(I/O 기다리는 프로세스들)                                              
	QUEUE* Ready_Queue = Create_Queue();
	QUEUE* Waiting_Queue = Create_Queue();

	//1-2 프로세스 복사 및 상태 변수 초기화
	//프로세스 정보 복사할 공간 확보(원본 건드리지 않기 위해)
	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);
	for (int i = 0; i < Process_Cnt; i++) 
		process[i] = Copy_Process(Process_Info[i]);

	int Process_End_Cnt = 0;//끝난 애들 개수 초기화
	int flag = 0;//총 구간 수 초기화

	//1-3 간트차트 기록용 CPU_Info 배열 준비
	//간트 차트 저장하는 2차원 배열//[0][i]=끝나는 시각,[1][i]=프로세스 번호
	int** CPU_Info = (int**)malloc(sizeof(int) * 2);
	for (int i = 0; i < 2; i++) 
		CPU_Info[i] = (int*)malloc(sizeof(int) * 1000);

	//1-4 시뮬레이션 변수 초기화
	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;//현재 시뮬레이션 초
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//2. while 루프 시작 - 프로세스 종료 전까지 계속 실행
	while (Process_End_Cnt < Process_Cnt) {

		//2-1. 프로세스 도착 확인 및 Ready Queue 삽입
		//현재 시간에 도착한 프로세스를 Ready Queue에 넣는다
		for (int i = 0; i < Process_Cnt; i++) 
			if (CPU_Time == process[i].Arrival_Time) 
				Enqueue(Ready_Queue, i + 1);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//2-2. CPU 시간 1초 경과
		CPU_Time++;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//2-3. CPU 상태에 따라 분기
		//a) 아무도 없고 Ready Queue도 비었으면 IDLE
		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) 
			CPU_Idle_Time++;//cpu도 놀고 있고, 줄에도 아무도 없으면 그냥 쉬는중
		//b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
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

			// c) CPU에서 1초 작업 수행
			process[Currunt_CPU_Process - 1].CPU_Burst_Time--;

			// d) 프로세스가 CPU 작업을 모두 마쳤다면(종료 시: 간트차트 기록, 종료 수 증가)
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
				Currunt_CPU_Process = -1;
				Process_End_Cnt++;
			}

			// e) CPU 작업 중 I/O 요청 시점 도달 시 I/O 큐로 이동
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
				left_IO[0][Currunt_CPU_Process - 1] = -CPU_Time;
				Enqueue(Waiting_Queue, Currunt_CPU_Process);
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
				Currunt_CPU_Process = -1;
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//2-4. I/O 처리
		//a) 지금 I/O 장치에서 작업 중인 프로세스가 있다면 그 프로세스의 I/O 남은 시간을 1초 줄인다
		if (Currunt_IO_Process != -1) 
			process[Currunt_IO_Process - 1].IO_Burst_Time--;

		//b) I/O가 끝났다면 남은 CPU 작업이 있다면 다시 Ready Queue로 보내고, I/O 종료 처리
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			if(process[Currunt_IO_Process - 1].CPU_Burst_Time != 0) 
				Enqueue(Ready_Queue, Currunt_IO_Process);
			Currunt_IO_Process = -1;
		}

		//c) 지금 아무도 I/O 사용 중이 아닌데, Waiting Queue에 사람이 있다면 대기 줄에서 한 명 꺼내서 I/O 실행 시작
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt != 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];
			left_IO[0][Currunt_IO_Process - 1] += CPU_Time;
			Dequeue(Waiting_Queue);
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//3. 결과 출력 및 기록
	printf("FCFS:\n");
	Print_Gantt_Chart(CPU_Info, flag);

	float FCFS_Average_Waiting_Time = Calculate_Average_Waiting_Time(Process_Info, Process_Cnt, CPU_Info, flag, 0);
	float FCFS_Average_Turnaround_Time = Calculate_Average_Turnaround_Time(Process_Info, Process_Cnt, CPU_Info, flag);
	float FCFS_CPU_Utilization = Calculate_CPU_Utilization(CPU_Info, flag);

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//도착한 프로세스들 중 가장 작업 시간이 짧은 것 먼저 실행(비선점형)
void SJF(Process Process_Info[], int Process_Cnt) {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. 초기화
//   1-1 Ready Queue, Waiting Queue 생성
//   1-2 프로세스 복사 및 상태 변수 초기화
//   1-3 간트차트 기록용 CPU_Info 배열 준비
//	 1-4 시뮬레이션용 변수 초기화
// 2. while (모든 프로세스 종료 전까지 반복)
//   2-1. 프로세스 도착 확인 및 Ready Queue 삽입
//     a) 현재 시간에 도착한 프로세스를 Enqueue
//     b) Ready Queue 정렬 (CPU Burst Time 오름차순)
//   2-2. CPU 시간 1 증가 (CPU_Time++)
//   2-3. CPU 상태에 따라 분기
//     a) 아무도 없고 Ready Queue도 비었으면 IDLE
//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
//     c) 현재 프로세스 1초 실행 (Burst Time--)
//     d) 종료 시: 간트차트 기록, 종료 수 증가
//     e) I/O 요청 시점 도달 시: I/O 큐로 이동
//   2-4. I/O 처리
//     a) 현재 I/O 프로세스 1초 수행
//     b) I/O 완료되면 CPU 작업 남은 경우 Ready로 + Ready Queue 재정렬
//     c) Waiting Queue에서 다음 I/O 프로세스 꺼내기 + 대기 시간 계산
// 3. 결과 출력 및 기록
//   - Gantt Chart 출력
//   - 평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. 초기화
	
	//1-1 Ready Queue, Waiting Queue 생성
	//준비 큐(CPU 올라가려고 기다리는 프로세스들), 대기 큐 생성(I/O 기다리는 프로세스들)                                              
	QUEUE* Ready_Queue = Create_Queue();																																						
	QUEUE* Waiting_Queue = Create_Queue();																																					
	//프로세스 정보 복사할 공간 확보(원본 건드리지 않기 위해)																																		 
	
	//1-2 프로세스 복사 및 상태 변수 초기화
	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);																						  
	//원본 프로세스 하나씩 복사																																														
	for (int i = 0; i < Process_Cnt; i++) 																																					
			process[i] = Copy_Process(Process_Info[i]);
																								
	int Process_End_Cnt = 0;//끝난 애들 개수 초기화																																
	int flag = 0;//총 구간 수 초기화

	//1-3 간트 차트 기록용 CPU+Info 배열 준비
	//간트 차트 저장하는 2차원 배열//[0][i]=끝나는 시각,[1][i]=프로세스 번호
	int** CPU_Info = (int**)malloc(sizeof(int) * 2);
	//CPU 실행기록 저장용 표 생성(행이 2개, 각 행마다 1000칸)
	for (int i = 0; i < 2; i++) 
			CPU_Info[i] = (int*)malloc(sizeof(int) * 1000);

	//1-4 시뮬레이션 용 변수 초기화
	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;//현재 시뮬레이션 초
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2. while (모든 프로세스 종료 전까지 반복)

//   2-1. 프로세스 도착 확인 및 Ready Queue 삽입
//     a) 현재 시간에 도착한 프로세스를 Enqueue
//     b) Ready Queue 정렬 (CPU Burst Time 오름차순)->버블 정렬 코드

	//a) 현재 시간에 도착한 프로세스를 Enqueue
	while (Process_End_Cnt < Process_Cnt) {//아직 안 끝난 프로세스 있다면 지속
		for (int i = 0; i < Process_Cnt; i++) 
			if (CPU_Time == process[i].Arrival_Time) {//현재 시각(CPU_Time)에 도착한 프로세스 있는지 확인
			Enqueue(Ready_Queue, i + 1);//도착한 프로세스 레디큐에 넣기 
			
			//b) Ready Queue 정렬 (CPU Burst Time 오름차순)->버블 정렬 코드
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//2-2 CPU 시간 1 증가
		CPU_Time++;//시간 증가
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//   2-3. CPU 상태에 따라 분기
		//     a) 아무도 없고 Ready Queue도 비었으면 IDLE
		//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
		//     c) 현재 프로세스 1초 실행 (Burst Time--)
		//     d) 종료 시: 간트차트 기록, 종료 수 증가
		//     e) I/O 요청 시점 도달 시: I/O 큐로 이동

		//a) 아무도 없고 Ready Queue도 비었으면 IDLE
		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) 
				CPU_Idle_Time++;//cpu사용하는애, 레디큐에 있는애 둘 다 없으면 IDLE 상태
		//b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
		else {
			if (Currunt_CPU_Process == -1) {//cpu에 아무것도 안 올라와 있다면?
				if (CPU_Idle_Time != 0) {//이전까지 cpu가 쉬고 있었다면 쉬는 구간 기록해야함
					CPU_Info[0][flag] = CPU_Time - 1;//쉬는 구간 끝 시각 기록
					CPU_Info[1][flag] = -1;//-1은 'IDLE' 상태 의미
					flag++;//다음 구간을 위한 인덱스 증가
					CPU_Idle_Time = 0;//IDLE 상태 초기화
				}
				Currunt_CPU_Process = Ready_Queue->ID[0];//레디큐 맨 앞에 있는애 cpu로 ㄱㄱ
				Dequeue(Ready_Queue);//레디큐에서 꺼냄
			}			
			//c) 현재 프로세스 1초 실행 (Burst Time--)
			process[Currunt_CPU_Process - 1].CPU_Burst_Time--;
			
			//d) 종료 시: 간트차트 기록, 종료 수 증가
			//프로세스가 끝났나? 중간에 I/O 하러 가야 하나? 체크해서 처리하는 구간
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {//프로세스가 cpu 작업을 모두 완료!
				CPU_Info[0][flag] = CPU_Time;//종료 시간
				CPU_Info[1][flag] = Currunt_CPU_Process;//종료한 프로세스의 ID
				flag++;//칸 증가
			//프로세스가 종료되긴 했지만, cpu 작업이 I/O 요청 타이밍과 정확히 일치해서 끝났다면, 남은 I/O 작업이 발생한 것처럼 처리
				if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
					left_IO[1][Currunt_CPU_Process - 1] = -CPU_Time;//I/O 작업에 걸린 시간 추적 배열
					Enqueue(Waiting_Queue, Currunt_CPU_Process);//해당 프로세스를 I/O 대기 큐로 보냄
				}
				Currunt_CPU_Process = -1;//cpu에서 이 프로세스 내림
				Process_End_Cnt++;
			}

			//e) I/O 요청 시점 도달 시: I/O 큐로 이동
			//if에서 else if로 수정
			//아직 프로세스가 끝나지는 않았는데 I/O 요청 타이밍인 경우(CPU 더 써야 되는데 I/O 먼저 해야 되는 경우)
			else if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
				left_IO[1][Currunt_CPU_Process - 1] = -CPU_Time;//I/O 처리 시간 기록을 위해 시작 지점 음수로 저장
				Enqueue(Waiting_Queue, Currunt_CPU_Process);//I/O 대기 큐에 현재 프로세스 넣음
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
				Currunt_CPU_Process = -1;//CPU에서 해당 프로세스 내림
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
		//   2-4. I/O 처리
		//     a) 현재 I/O 프로세스 1초 수행
		//     b) I/O 완료되면 CPU 작업 남은 경우 Ready로 + Ready Queue 재정렬
		//     c) Waiting Queue에서 다음 I/O 프로세스 꺼내기 + 대기 시간 계산
		
		// a) 현재 I/O 프로세스 1초 수행
		if (Currunt_IO_Process != -1) //현재 I/O 장치에서 일하는 프로세스 있으면 작업 시간 1 줄이기
			process[Currunt_IO_Process - 1].IO_Burst_Time--;
		
		// b) I/O 완료되면 CPU 작업 남은 경우 Ready로 + Ready Queue 재정렬
		//현재 I/O 프로세스의 작업이 끝난 경우?	
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			//I/O 작업은 끝났지만 CPU 작업이 남아있는 경우는 readyqueue에 재진입해야함
			if (process[Currunt_IO_Process - 1].CPU_Burst_Time > 0) {
				Enqueue(Ready_Queue, Currunt_IO_Process);
				//readyqueue 재정렬(cpu burst 짧은 순서로!)
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
			Currunt_IO_Process = -1;//해당 i/o 프로세스 i/o 장치에서 내려갔다.
		}
		//c) Waiting Queue에서 다음 I/O 프로세스 꺼내기 + 대기 시간 계산
		//현재 I/O 장치가 비어 있고, 대기 중인 프로세스가 있다면 그 중 하나를 꺼내서 I/O 장치에 올린다
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt > 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];//waiting queue의 가장 앞에 있는 프로세스를 I/O 장치에 올린다 
			left_IO[1][Currunt_IO_Process - 1] += CPU_Time;//(음수로 기록해둔 대기 시작 시각) + (지금 시각) → I/O 대기 시간 계산
			Dequeue(Waiting_Queue);//waiting queue에서 제거
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3. 결과 출력 및 기록
//   - Gantt Chart 출력
//   - 평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Priority(Process Process_Info[], int Process_Cnt) {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. 초기화
// 	-Ready Queue, Waiting Queue 생성
// 	-프로세스 복사 및 상태 변수 초기화
// 	-간트차트 기록용 CPU_Info 배열 준비

// 2. while (모든 프로세스 종료 전까지 반복)
// 	2-1. 프로세스 도착 확인 및 Ready Queue 삽입
// 		-현재 시간에 도착한 프로세스를 Enqueue
// 		-Ready Queue 정렬 (Priority 내림차순)
// 	2-2. CPU 시간 1 증가 (CPU_Time++)
// 	2-3. CPU 상태에 따라 분기
// 		a) 아무도 없고 Ready Queue도 비었으면 IDLE
//		b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
//		c) 현재 프로세스 1초 실행 (Burst Time--)
// 		d) 종료 시: 간트차트 기록, 종료 수 증가
// 		e) I/O 요청 시점 도달 시: I/O 큐로 이동
// 	2-4. I/O 처리
// 		a) 현재 I/O 프로세스 1초 수행
// 		b) I/O 완료되면 CPU 작업 남은 경우 Ready로 + Ready Queue 재정렬
//		c) Waiting Queue에서 다음 I/O 프로세스 꺼내기 + 대기 시간 계산
// 3. 결과 출력 및 기록
// 	-Gantt Chart 출력
// 	-평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. 초기화
	
	//1-1 Ready Queue, Waiting Queue 생성
	//준비 큐(CPU 올라가려고 기다리는 프로세스들), 대기 큐 생성(I/O 기다리는 프로세스들)                                              
	QUEUE* Ready_Queue = Create_Queue();																																						
	QUEUE* Waiting_Queue = Create_Queue();																																					
	//프로세스 정보 복사할 공간 확보(원본 건드리지 않기 위해)																																		 
	
	//1-2 프로세스 복사 및 상태 변수 초기화
	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);																						  
	//원본 프로세스 하나씩 복사																																														
	for (int i = 0; i < Process_Cnt; i++) 																																					
			process[i] = Copy_Process(Process_Info[i]);
																								
	int Process_End_Cnt = 0;//끝난 애들 개수 초기화																																
	int flag = 0;//총 구간 수 초기화

	//1-3 간트 차트 기록용 CPU+Info 배열 준비
	//간트 차트 저장하는 2차원 배열//[0][i]=끝나는 시각,[1][i]=프로세스 번호
	int** CPU_Info = (int**)malloc(sizeof(int) * 2);
	//CPU 실행기록 저장용 표 생성(행이 2개, 각 행마다 1000칸)
	for (int i = 0; i < 2; i++) 
			CPU_Info[i] = (int*)malloc(sizeof(int) * 1000);

	//1-4 시뮬레이션 용 변수 초기화
	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;//현재 시뮬레이션 초
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//2. while (모든 프로세스 종료 전까지 반복)
	
	//2-1.프로세스 도착 확인 및 readyqueue 삽입
	//    - 현재 시간에 도착한 프로세스를 enqueue
	//    - ready queue 정렬(priority 내림차순)
	while (Process_End_Cnt < Process_Cnt) {
		// 현재 시간에 도착한 프로세스가 있다면 레디큐에 추가!
		for (int i = 0; i < Process_Cnt; i++) 
			if (CPU_Time == process[i].Arrival_Time) {
				Enqueue(Ready_Queue, i + 1);
				// Ready Queue를 우선순위(Priority) 기준으로 정렬 (내림차순)
				for (int j = 0; j < Ready_Queue->cnt; j++) {
					for (int k = j + 1; k < Ready_Queue->cnt; k++) {
						if (process[Ready_Queue->ID[j] - 1].Priority < process[Ready_Queue->ID[k] - 1].Priority) {
							int temp = Ready_Queue->ID[j];
							Ready_Queue->ID[j] = Ready_Queue->ID[k];
							Ready_Queue->ID[k] = temp;
						}
					}
				}
			}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//2-2. CPU 시간 1 증가

		CPU_Time++;//cpu의 시간은 언제나 흘러간다..
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//2-3. CPU 상태에 따라 분기
		  // a) 아무도 없고 Ready Queue도 비었으면 IDLE
			// b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
			// c) 현재 프로세스 1초 실행 (Burst Time--)
			// d) 종료 시: 간트차트 기록, 종료 수 증가
			// e) I/O 요청 시점 도달 시: I/O 큐로 이동

		// a) 아무도 없고 Ready Queue도 비었으면 IDLE 상태
		// CPU에 할당된 프로세스가 없고 Ready Queue도 비어있으면 IDLE
		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) {
    		CPU_Idle_Time++;				
		}
		// b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
		else {//그 외의 경우 = CPU가 놀고 있지 않은 경우
    		if (Currunt_CPU_Process == -1) {
				//이전에 CPU가 놀고 있었다면 그 IDLE 구간을 간트차트에 기록하자
        		if (CPU_Idle_Time != 0) {
            		CPU_Info[0][flag] = CPU_Time - 1;
            		CPU_Info[1][flag] = -1;
            		flag++;
            CPU_Idle_Time = 0;
        }
        // Ready Queue에서 프로세스를 꺼내 CPU에 할당
        Currunt_CPU_Process = Ready_Queue->ID[0];
        Dequeue(Ready_Queue);
    }
		// c) CPU에서 실행 중인 프로세스의 작업 시간 감소
    // 현재 CPU 프로세스의 작업 시간 1초 감소
    process[Currunt_CPU_Process - 1].CPU_Burst_Time--;

		// d) CPU 작업이 모두 끝난 경우
		//프로세스가 더 이상 작업할 cpu 시간이 없다면 -> 작업 종료 상태
    if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {
        // 간트차트에 종료 시간 기록
        CPU_Info[0][flag] = CPU_Time;
        CPU_Info[1][flag] = Currunt_CPU_Process;
        flag++;

        // 종료 시점이 I/O 요청 타이밍과 겹치면 I/O대기 큐로 보냄
        if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
            left_IO[2][Currunt_CPU_Process - 1] = -CPU_Time;
            Enqueue(Waiting_Queue, Currunt_CPU_Process);
        }

        Currunt_CPU_Process = -1;
        Process_End_Cnt++;
    }

    // 아직 작업은 남았지만 I/O 요청 시점에 도달한 경우
    if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
        //I/O 대기 큐에 넣고, I/O 대기 시작 시점을 -CPU_Time으로 저장
				left_IO[2][Currunt_CPU_Process - 1] = -CPU_Time;
        Enqueue(Waiting_Queue, Currunt_CPU_Process);

        // 간트차트에 해당 시점 기록
        CPU_Info[0][flag] = CPU_Time;
        CPU_Info[1][flag] = Currunt_CPU_Process;
        flag++;

        Currunt_CPU_Process = -1;//프로세스 CPU에서 내리기
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//2-4. I/O 처리
		//	a) 현재 I/O 프로세스 1초 수행
		//	b) I/O 완료되면 CPU 작업 남은 경우 Ready로 + Ready Queue 재정렬
		//	c) Waiting Queue에서 다음 I/O 프로세스 꺼내기 + 대기 시간 계산

		// a) I/O 장치에 프로세스가 있다면 I/O 작업 1초 수행
		if (Currunt_IO_Process != -1) 
			process[Currunt_IO_Process - 1].IO_Burst_Time--;

		// b) I/O 작업이 끝났다면 CPU로 다시 보낼지 확인
		//현재 I/O 장치에 있는 프로세스의 I/O가 완료되었는지 확인	
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			//I/O는 끝났는데 아직 CPU 작업이 남아있다?->다시 ready queue에 넣어야 함
			if (process[Currunt_IO_Process - 1].CPU_Burst_Time > 0) {
				Enqueue(Ready_Queue, Currunt_IO_Process);
				//readyqueue 재정렬하기(우선순위 기준으로 priority 값이 더 큰 프로세스가 앞에 오도록 내림차순 정렬)
				for (int i = 0; i < Ready_Queue->cnt; i++) {
					for (int j = i + 1; j < Ready_Queue->cnt; j++) {
						if (process[Ready_Queue->ID[i] - 1].Priority < process[Ready_Queue->ID[j] - 1].Priority) {
							int temp = Ready_Queue->ID[i];
							Ready_Queue->ID[i] = Ready_Queue->ID[j];
							Ready_Queue->ID[j] = temp;
						}
					}
				}
			}
			Currunt_IO_Process = -1;//I/O 장치가 비우기
		}

		// c) I/O 장치가 비었고 대기 중인 프로세스가 있다면 할당
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt > 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];//waiting queue에 있던애 I/O 장치에 올려주기
			left_IO[2][Currunt_IO_Process - 1] += CPU_Time;//대기 시간 계산
			Dequeue(Waiting_Queue);
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. 결과 출력 및 기록
	// 	-Gantt Chart 출력
	// 	-평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
	printf("Priority:\n");
	Print_Gantt_Chart(CPU_Info, flag);

	float Priority_Average_Waiting_Time = Calculate_Average_Waiting_Time(Process_Info, Process_Cnt, CPU_Info, flag, 2);
	float Priority_Average_Turnaround_Time = Calculate_Average_Turnaround_Time(Process_Info, Process_Cnt, CPU_Info, flag);
	float Priority_CPU_Utilization = Calculate_CPU_Utilization(CPU_Info, flag);

	Average_Waiting_Time[2] = Priority_Average_Waiting_Time;
	Average_Turnaround_Time[2] = Priority_Average_Turnaround_Time;
	CPU_Utilization[2] = Priority_CPU_Utilization;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RR(Process Process_Info[], int Process_Cnt, int quantum) {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. 초기화
//   - Ready Queue, Waiting Queue 생성
//   - 프로세스 복사 및 상태 변수 초기화
//   - 간트차트 기록용 CPU_Info 배열 준비
//   - 시뮬레이션용 변수 초기화 (quantum 포함)

// 2. while (모든 프로세스 종료 전까지 반복)
//   2-1. 프로세스 도착 확인 및 Ready Queue 삽입
//   2-2. CPU 시간 1 증가
//   2-3. CPU 상태에 따라 분기
//     a) 아무도 없고 Ready Queue도 비었으면 IDLE
//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
//     c) 현재 프로세스 1초 실행 (Burst Time--, Quantum Time++)
//     d) 종료 시: 간트차트 기록, 종료 수 증가
//     e) I/O 요청 도달 시: I/O 큐로 이동
//     f) Quantum 다 찼을 경우: 간트차트 기록 후 다시 Ready Queue로
//   2-4. I/O 처리
//     a) 현재 I/O 프로세스 1초 수행
//     b) I/O 완료되면 Ready로 복귀 (CPU 작업 남은 경우)
//     c) 대기 중인 프로세스를 I/O 장치에 올림

// 3. 결과 출력 및 기록
//   - Gantt Chart 출력
//   - 평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. 초기화
	
	//1-1 Ready Queue, Waiting Queue 생성
	//준비 큐(CPU 올라가려고 기다리는 프로세스들), 대기 큐 생성(I/O 기다리는 프로세스들)                                              
	QUEUE* Ready_Queue = Create_Queue();																																						
	QUEUE* Waiting_Queue = Create_Queue();																																																																						 
	
	//1-2 프로세스 복사 및 상태 변수 초기화
	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);																						  
	//원본 프로세스 하나씩 복사																																														
	for (int i = 0; i < Process_Cnt; i++) 																																					
			process[i] = Copy_Process(Process_Info[i]);
																								
	int Process_End_Cnt = 0;//끝난 애들 개수 초기화																																
	int flag = 0;//총 구간 수 초기화

	//1-3 간트 차트 기록용 CPU_Info 배열 준비
	//간트 차트 저장하는 2차원 배열//[0][i]=끝나는 시각,[1][i]=프로세스 번호
	int** CPU_Info = (int**)malloc(sizeof(int) * 2);
	//CPU 실행기록 저장용 표 생성(행이 2개, 각 행마다 1000칸)
	for (int i = 0; i < 2; i++) 
			CPU_Info[i] = (int*)malloc(sizeof(int) * 1000);

	//1-4 시뮬레이션 용 변수 초기화
	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;//현재 시뮬레이션 초
	int Quantum_Time = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 2. 시뮬레이션 루프 시작
	while (Process_End_Cnt < Process_Cnt) {

		// 2-1. 현재 시간에 도착한 프로세스 Ready Queue에 삽입
		for (int i = 0; i < Process_Cnt; i++)
			if (CPU_Time == process[i].Arrival_Time)
				Enqueue(Ready_Queue, i + 1);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2-2. 시간 경과
		CPU_Time++;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2-3. CPU 상태에 따라 분기
		// a) CPU도 비어있고 ReadyQueue도 없으면 IDLE
		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) {
			CPU_Idle_Time++;
		} else {
			// b) CPU가 비어있고 Ready Queue에 누군가 있으면 꺼내기
			if (Currunt_CPU_Process == -1) {
				if (CPU_Idle_Time != 0) {//최근까지 IDLE 상태였다면 이 IDLE 구간을 간트차트에 기록
					CPU_Info[0][flag] = CPU_Time - 1;//간트차트에서 IDLE 상태의 종료 시간 저장
					CPU_Info[1][flag] = -1;//간트차트에서 해당 구간이 IDLE이었다는 표시로 -1 넣기
					flag++;//간트차트 다음 칸을 위한 인덱스 증가
					CPU_Idle_Time = 0;//IDLE 구간 기록 끝났응게 다시 0으로 초기화
				}
				Currunt_CPU_Process = Ready_Queue->ID[0];//레디큐 대기 중이던 맨 앞 프로세스 cpu로
				Dequeue(Ready_Queue);
			}

			// c) 실행 중인 프로세스 1초 수행
			process[Currunt_CPU_Process - 1].CPU_Burst_Time--;
			Quantum_Time++;//퀀텀 소요 시간 1초 증가도 반영

			// d) 프로세스가 종료된 경우
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {
				CPU_Info[0][flag] = CPU_Time;//현재 시점이 해당 프로세스 종료 시각
				CPU_Info[1][flag] = Currunt_CPU_Process;//이 구간에 실행된 프로세스 번호 기록
				flag++;//간트 차트 다음 칸으로
				Process_End_Cnt++;//종료 프로세스 수 증가
				Quantum_Time = 0;//퀀텀 시간 초기화
				Currunt_CPU_Process = -1;//CPU 비우기
			}

			// e) I/O 요청 도달 시점이면 I/O 큐로 이동
			else if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
				left_IO[3][Currunt_CPU_Process - 1] = -CPU_Time;//I/O 대기 시간 계산을 위해 음수로 시작 시점 기록
				Enqueue(Waiting_Queue, Currunt_CPU_Process);//웨이팅 큐에 삽입
				CPU_Info[0][flag] = CPU_Time;//간트차트 구간 종료 시각 기록
				CPU_Info[1][flag] = Currunt_CPU_Process;//실행 프로세스 기록
				flag++;//간트차트 다음 칸 이동
				Quantum_Time = 0;//퀀텀 시간 초기화
				Currunt_CPU_Process = -1;//CPU 비우기
			}

			// f) Quantum 다 찬 경우 → 프로세스는 다시 Ready Queue로
			else if (Quantum_Time == quantum) {
				CPU_Info[0][flag] = CPU_Time;//현재 시각을 이 실행 구간 종료 시간으로 기록
				CPU_Info[1][flag] = Currunt_CPU_Process;//이 구간에서 실행된 프로세스 번호 기록
				flag++;//간트 차트 다음 칸으로
				Enqueue(Ready_Queue, Currunt_CPU_Process);//퀀텀이 다 찼기 때문에 다시 레디큐에 넣어서 줄 서게 하자
				Quantum_Time = 0;//퀀텀 시간 초기화
				Currunt_CPU_Process = -1;//CPU 비우기
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2-4. I/O 처리
		// a) 현재 I/O 프로세스 1초 수행
		if (Currunt_IO_Process != -1)//I/O 장치에서 현재 실행 중인 프로세스 있다면
			process[Currunt_IO_Process - 1].IO_Burst_Time--;

		// b) I/O 완료 시 Ready로 복귀(I/O burst time이 0이 되었다면 I/O 완료!)
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			//CPU_Burest_Time이 남아 있다면 다시 레디큐로~
			if (process[Currunt_IO_Process - 1].CPU_Burst_Time != 0)
				Enqueue(Ready_Queue, Currunt_IO_Process);
			Currunt_IO_Process = -1;//I/O 장치 비우기
		}

		// c) I/O 장치가 비어있고, 대기 큐에 프로세스가 있다면 할당
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt != 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];
			left_IO[3][Currunt_IO_Process - 1] += CPU_Time;//I/O 대기 시간 기록
			Dequeue(Waiting_Queue);//웨이링 큐에서 제거
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. 결과 출력 및 기록
	printf("RR:\n");
	Print_Gantt_Chart(CPU_Info, flag);

	float RR_Average_Waiting_Time = Calculate_Average_Waiting_Time(Process_Info, Process_Cnt, CPU_Info, flag, 3);
	float RR_Average_Turnaround_Time = Calculate_Average_Turnaround_Time(Process_Info, Process_Cnt, CPU_Info, flag);
	float RR_CPU_Utilization = Calculate_CPU_Utilization(CPU_Info, flag);

	Average_Waiting_Time[3] = RR_Average_Waiting_Time;
	Average_Turnaround_Time[3] = RR_Average_Turnaround_Time;
	CPU_Utilization[3] = RR_CPU_Utilization;

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P_SJF(Process Process_Info[], int Process_Cnt) {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. 초기화
//   - Ready Queue, Waiting Queue 생성
//   - 프로세스 복사 및 상태 변수 초기화
//   - 간트차트 기록용 CPU_Info 배열 준비
//   - 시뮬레이션용 변수 초기화

// 2. while (모든 프로세스 종료 전까지 반복)
//   2-1. 프로세스 도착 확인 및 Ready Queue 삽입
//     a) 현재 시간에 도착한 프로세스를 Enqueue
//     b) Ready Queue를 CPU Burst Time 기준으로 오름차순 정렬
//     c) 현재 실행 중인 프로세스보다 짧은 Job이 있다면 선점 발생
//   2-2. CPU 시간 1 증가
//   2-3. CPU 상태에 따라 분기
//     a) 아무도 없고 Ready Queue도 비었으면 IDLE
//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
//     c) 현재 프로세스 1초 실행
//     d) 종료 시: 간트차트 기록, 종료 수 증가
//     e) I/O 요청 도달 시: I/O 큐로 이동
//   2-4. I/O 처리
//     a) 현재 I/O 프로세스 1초 수행
//     b) I/O 완료되면 Ready로 복귀 + Ready Queue 재정렬
//     c) 필요 시 현재 CPU 프로세스를 Ready Queue 최단 작업으로 교체

// 3. 결과 출력 및 기록
//   - Gantt Chart 출력
//   - 평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. 초기화
	
	//1-1 Ready Queue, Waiting Queue 생성
	//준비 큐(CPU 올라가려고 기다리는 프로세스들), 대기 큐 생성(I/O 기다리는 프로세스들)                                              
	QUEUE* Ready_Queue = Create_Queue();																																						
	QUEUE* Waiting_Queue = Create_Queue();																																					
	//프로세스 정보 복사할 공간 확보(원본 건드리지 않기 위해)																																		 
	
	//1-2 프로세스 복사 및 상태 변수 초기화
	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);																						  
	//원본 프로세스 하나씩 복사																																														
	for (int i = 0; i < Process_Cnt; i++) 																																					
			process[i] = Copy_Process(Process_Info[i]);
																								
	int Process_End_Cnt = 0;//끝난 애들 개수 초기화																																
	int flag = 0;//총 구간 수 초기화

	//1-3 간트 차트 기록용 CPU+Info 배열 준비
	//간트 차트 저장하는 2차원 배열//[0][i]=끝나는 시각,[1][i]=프로세스 번호
	int** CPU_Info = (int**)malloc(sizeof(int) * 2);
	//CPU 실행기록 저장용 표 생성(행이 2개, 각 행마다 1000칸)
	for (int i = 0; i < 2; i++) 
			CPU_Info[i] = (int*)malloc(sizeof(int) * 1000);

	//1-4 시뮬레이션 용 변수 초기화
	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;//현재 시뮬레이션 초
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2. while (모든 프로세스 종료 전까지 반복)
//   2-1. 프로세스 도착 확인 및 Ready Queue 삽입
//     a) 현재 시간에 도착한 프로세스를 Enqueue
//     b) Ready Queue를 CPU Burst Time 기준으로 오름차순 정렬
//     c) 현재 실행 중인 프로세스보다 짧은 Job이 있다면 선점 발생
	
	while (Process_End_Cnt < Process_Cnt) {
		// a) 현재 시간에 도착한 프로세스를 Enqueue
		// 모든 프로세스에 대해 현재 CPU시간에 도착한 애 있나 확인
		for (int i = 0; i < Process_Cnt; i++) {
			if (CPU_Time == process[i].Arrival_Time) {
				Enqueue(Ready_Queue, i + 1);//일치하면 레디큐에 넣기

				// b) Ready Queue를 CPU Burst Time 기준으로 오름차순 정렬
				for (int j = 0; j < Ready_Queue->cnt; j++) {
					for (int k = j + 1; k < Ready_Queue->cnt; k++) {
						if (process[Ready_Queue->ID[j] - 1].CPU_Burst_Time > process[Ready_Queue->ID[k] - 1].CPU_Burst_Time) {
							int temp = Ready_Queue->ID[j];
							Ready_Queue->ID[j] = Ready_Queue->ID[k];
							Ready_Queue->ID[k] = temp;
						}
					}
				}

				// c) 선점 발생: 현재 실행 중인 것보다 더 짧은 Job이 들어옴
				// 현재 실행 중인 프로세스 존재 + 레디큐 맨 앞의 cpu burst time이 더 짧으면 조건 충족
				if (Currunt_CPU_Process != -1 &&
					process[Currunt_CPU_Process - 1].CPU_Burst_Time > process[Ready_Queue->ID[0] - 1].CPU_Burst_Time) {
					CPU_Info[0][flag] = CPU_Time;//구간 종료 시각 기록
					CPU_Info[1][flag] = Currunt_CPU_Process;//실행 프로세스 기록
					flag++;//간트 차트 다음 칸으로
					//레디큐 맨 앞에 있는 녀석과 교체
					int temp = Ready_Queue->ID[0];
					Ready_Queue->ID[0] = Currunt_CPU_Process;
					Currunt_CPU_Process = temp;
				}
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2-2. 시간 흐름
		CPU_Time++;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  2-3. CPU 상태에 따라 분기
//     a) 아무도 없고 Ready Queue도 비었으면 IDLE
//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE이면 간트차트에 기록
//     c) 현재 프로세스 1초 실행
//     d) 종료 시: 간트차트 기록, 종료 수 증가
//     e) I/O 요청 도달 시: I/O 큐로 이동
// 				e-1) I/O 요청 도달과 동시에 종료된 경우
// 				e-2) CPU 작업은 남았지만 I/O 요청 도달

		//a) 아무도 없고 Ready Queue도 비었으면 IDLE
		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) {
			CPU_Idle_Time++;//cpu 실행도 없고~ 레디큐에도 없으면~ IDLE
		} else {
			if (Currunt_CPU_Process == -1) {//cpu 비어 있는데 레디큐에 프로세스 있으니 새로운 애 할당
				if (CPU_Idle_Time != 0) {//그 전에 쉬고 있었다면 IDLE 구간 차트에 기록
					CPU_Info[0][flag] = CPU_Time - 1;
					CPU_Info[1][flag] = -1;
					flag++;
					CPU_Idle_Time = 0;
				}
				Currunt_CPU_Process = Ready_Queue->ID[0];//레디큐에서 맨 앞 프로세스를  cpu에 할당
				Dequeue(Ready_Queue);//큐에서 꺼낸 프로세스 제거
			}

			// c) CPU 실행 1초
			process[Currunt_CPU_Process - 1].CPU_Burst_Time--;

			// d) 프로세스 종료
			//더 이상 실행할 CPU 없으니 종료하고 간트차트 기록
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;

				// e-1) I/O 요청 도달과 동시에 종료된 경우
				if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
					//I/O 대기 시간 기록 및 대기 큐로 이동
					left_IO[4][Currunt_CPU_Process - 1] = -CPU_Time;
					Enqueue(Waiting_Queue, Currunt_CPU_Process);
				}
				//cpu 해제 맟 종료 프로세스 수 증가
				Currunt_CPU_Process = -1;
				Process_End_Cnt++;
			}

			// e-2) CPU 작업은 남았지만 I/O 요청 도달
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
				//I/O 시작 시각 기록용(추후 실제 대기시간 계산에 활용)
				left_IO[4][Currunt_CPU_Process - 1] = -CPU_Time;
				//I/O 대기 큐에 현재 프로세스를 추가
				Enqueue(Waiting_Queue, Currunt_CPU_Process);
				//간트차트 기록
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
				//cpu 비우기
				Currunt_CPU_Process = -1;
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2-4. I/O 처리
		//<흐름 도식>
		//[IO 종료] → [CPU 작업 남음?] → [Ready Queue에 삽입] → [정렬] → [선점 조건 검사]
    //                                                           ↓ yes
    //                                                         [간트차트 기록]
    //                                                         [프로세스 교체]
		// a) I/O에서 어떤 프로세스 실행 중이라면 1초 감소
		if (Currunt_IO_Process != -1)
			process[Currunt_IO_Process - 1].IO_Burst_Time--;
		//I/O 작업이 끝났는데 cpu 작업 남아 있다면 레디큐로 복귀
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			if (process[Currunt_IO_Process - 1].CPU_Burst_Time > 0) {
				Enqueue(Ready_Queue, Currunt_IO_Process);

				// Ready Queue 재정렬
				for (int i = 0; i < Ready_Queue->cnt; i++) {
					for (int j = i + 1; j < Ready_Queue->cnt; j++) {
						if (process[Ready_Queue->ID[i] - 1].CPU_Burst_Time > process[Ready_Queue->ID[j] - 1].CPU_Burst_Time) {
							int temp = Ready_Queue->ID[i];
							Ready_Queue->ID[i] = Ready_Queue->ID[j];
							Ready_Queue->ID[j] = temp;
						}
					}
				}

				// 선점 검사
				//현재 실행 중인 프로세스가 있고 + 레디큐에 막 들어온 프로세스의 cpu 작업량이 더 짧다면?
				if (Currunt_CPU_Process != -1 &&
					process[Currunt_CPU_Process - 1].CPU_Burst_Time > process[Ready_Queue->ID[0] - 1].CPU_Burst_Time) {
					//cpu 상태 간트차트에 기록
					CPU_Info[0][flag] = CPU_Time;
					CPU_Info[1][flag] = Currunt_CPU_Process;
					flag++;
					//프로세스 교체
					int temp = Ready_Queue->ID[0];
					Ready_Queue->ID[0] = Currunt_CPU_Process;
					Currunt_CPU_Process = temp;
				}
			}
			Currunt_IO_Process = -1;//현재 프로세스 비우기
		}
		//I/O 장치가 비어 있고, 대기 큐에 사람 있으면 맨 앞을 I/O로
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt > 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];
			left_IO[4][Currunt_IO_Process - 1] += CPU_Time;//대기 시간 계산
			Dequeue(Waiting_Queue);
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. 결과 출력 및 기록
	printf("P_SJF:\n");
	Print_Gantt_Chart(CPU_Info, flag);

	float P_SJf_Average_Waiting_Time = Calculate_Average_Waiting_Time(Process_Info, Process_Cnt, CPU_Info, flag, 4);
	float P_SJF_Average_Turnaround_Time = Calculate_Average_Turnaround_Time(Process_Info, Process_Cnt, CPU_Info, flag);
	float P_SJF_CPU_Utilization = Calculate_CPU_Utilization(CPU_Info, flag);

	Average_Waiting_Time[4] = P_SJf_Average_Waiting_Time;
	Average_Turnaround_Time[4] = P_SJF_Average_Turnaround_Time;
	CPU_Utilization[4] = P_SJF_CPU_Utilization;

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void P_Priority(Process Process_Info[], int Process_Cnt) {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. 초기화
//   - Ready Queue, Waiting Queue 생성
//   - 프로세스 복사 및 상태 변수 초기화
//   - 간트차트 기록용 CPU_Info 배열 준비
//   - 시뮬레이션용 변수 초기화

// 2. while (모든 프로세스 종료 전까지 반복)
//   2-1. 프로세스 도착 확인 및 Ready Queue 삽입
//     a) 도착 프로세스 Ready Queue에 삽입
//     b) Ready Queue 정렬 (Priority 내림차순)
//   2-2. 선점 조건 검사 (현재 실행 중인 프로세스보다 높은 우선순위 도착 시)
//   2-3. CPU 시간 증가
//   2-4. CPU 처리
//     a) 아무도 없고 Ready Queue도 비었으면 IDLE
//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE 기록
//     c) 현재 프로세스 1초 실행
//     d) 종료 시: 간트차트 기록, 종료 수 증가
//     e) I/O 요청 도달 시: I/O 큐로 이동
//				e-1) 종료 시점에 I/O 요청 도달(동시 발생)
//				e-2) 작업은 남았지만 I/O 요청 시점 도달
//   2-5. I/O 처리
//     a) 현재 I/O 프로세스 1초 수행
//     b) 완료 시 Ready로 복귀 + Ready Queue 정렬 + 필요 시 선점
//     c) I/O 장치가 비었으면 대기 중 하나 꺼내기

// 3. 결과 출력 및 기록
//   - Gantt Chart 출력
//   - 평균 대기 시간, 반환 시간, CPU 활용률 계산 및 저장
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. 초기화
//   - Ready Queue, Waiting Queue 생성
//   - 프로세스 복사 및 상태 변수 초기화
//   - 간트차트 기록용 CPU_Info 배열 준비
//   - 시뮬레이션용 변수 초기화

	//1-1 Ready Queue, Waiting Queue 생성
	//준비 큐(CPU 올라가려고 기다리는 프로세스들), 대기 큐 생성(I/O 기다리는 프로세스들)                                              
	QUEUE* Ready_Queue = Create_Queue();																																						
	QUEUE* Waiting_Queue = Create_Queue();																																					
	//프로세스 정보 복사할 공간 확보(원본 건드리지 않기 위해)																																		 
	
	//1-2 프로세스 복사 및 상태 변수 초기화
	Process* process = (Process*)malloc(sizeof(Process) * Process_Cnt);																						  
	//원본 프로세스 하나씩 복사																																														
	for (int i = 0; i < Process_Cnt; i++) 																																					
			process[i] = Copy_Process(Process_Info[i]);
																								
	int Process_End_Cnt = 0;//끝난 애들 개수 초기화																																
	int flag = 0;//총 구간 수 초기화

	//1-3 간트 차트 기록용 CPU+Info 배열 준비
	//간트 차트 저장하는 2차원 배열//[0][i]=끝나는 시각,[1][i]=프로세스 번호
	int** CPU_Info = (int**)malloc(sizeof(int) * 2);
	//CPU 실행기록 저장용 표 생성(행이 2개, 각 행마다 1000칸)
	for (int i = 0; i < 2; i++) 
			CPU_Info[i] = (int*)malloc(sizeof(int) * 1000);

	//1-4 시뮬레이션 용 변수 초기화
	int Currunt_CPU_Process = -1;
	int Currunt_IO_Process = -1;
	int CPU_Idle_Time = 0;
	int CPU_Time = 0;//현재 시뮬레이션 초
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. while (모든 프로세스 종료 전까지 반복)
	while (Process_End_Cnt < Process_Cnt) {

	// 2-1. 도착한 프로세스를 Ready Queue에 추가 + 정렬
		//     a) 도착 프로세스 Ready Queue에 삽입
		//     b) Ready Queue 정렬 (Priority 내림차순)

		//도착한 프로세스를 레디 큐에 추가 +  정렬
		for (int i = 0; i < Process_Cnt; i++) {
			if (CPU_Time == process[i].Arrival_Time) {
				Enqueue(Ready_Queue, i + 1);
				for (int j = 0; j < Ready_Queue->cnt; j++) {
					for (int k = j + 1; k < Ready_Queue->cnt; k++) {
						if (process[Ready_Queue->ID[j] - 1].Priority < process[Ready_Queue->ID[k] - 1].Priority) {
							int temp = Ready_Queue->ID[j];
							Ready_Queue->ID[j] = Ready_Queue->ID[k];
							Ready_Queue->ID[k] = temp;
						}
					}
				}
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2-2. 현재 실행 중인 프로세스보다 더 높은 우선순위가 들어왔는지 확인 → 선점
		if (Currunt_CPU_Process != -1 &&
			process[Currunt_CPU_Process - 1].Priority < process[Ready_Queue->ID[0] - 1].Priority) {
			//간트 차트에 기록
			CPU_Info[0][flag] = CPU_Time;
			CPU_Info[1][flag] = Currunt_CPU_Process;
			flag++;
			//프로세스 교체
			int temp = Ready_Queue->ID[0];
			Ready_Queue->ID[0] = Currunt_CPU_Process;
			Currunt_CPU_Process = temp;
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 2-3. 시간 증가
		CPU_Time++;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2-4. CPU 처리
//		 a) 아무도 없고 Ready Queue도 비었으면 IDLE
//     b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE 기록
//     c) 현재 프로세스 1초 실행
//     d) 종료 시: 간트차트 기록, 종료 수 증가
//     e) I/O 요청 도달 시: I/O 큐로 이동

		// a) 아무도 없고 Ready Queue도 비었으면 IDLE
		if (Currunt_CPU_Process == -1 && Ready_Queue->cnt == 0) {
			CPU_Idle_Time++;
		}
		// b) CPU에 프로세스 할당 필요 시 Ready Queue에서 꺼내고, 이전 IDLE 기록
		else {
			if (Currunt_CPU_Process == -1) {//레디큐에 프로세스 있다면 cpu 할당
				if (CPU_Idle_Time != 0) {
					// IDLE 상태였다면 기록
					CPU_Info[0][flag] = CPU_Time - 1;
					CPU_Info[1][flag] = -1;
					flag++;
					CPU_Idle_Time = 0;
				}
				//새 프로세스 할당
				Currunt_CPU_Process = Ready_Queue->ID[0];
				Dequeue(Ready_Queue);
			}
//    c) 현재 프로세스 1초 실행
			process[Currunt_CPU_Process - 1].CPU_Burst_Time--;

//    d) 종료 시: 간트차트 기록, 종료 수 증가
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == 0) {
				//간트 차트 기록+인덱스 증가
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
//    e) I/O 요청 도달 시: I/O 큐로 이동

				// e-1: 종료 시점에 I/O 요청 도달 (동시 발생)
				if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
					left_IO[5][Currunt_CPU_Process - 1] = -CPU_Time;//I/O 대기 시작 시점을 음수로 저장
					Enqueue(Waiting_Queue, Currunt_CPU_Process);//해당 프로세스 I/O 대기큐에 삽입
				}

				Currunt_CPU_Process = -1;//cpu qldnrl
				Process_End_Cnt++;//종료 프로세스 수 증가
			}

//    e-2: 작업은 남았지만 I/O 요청 시점 도달
			if (process[Currunt_CPU_Process - 1].CPU_Burst_Time == process[Currunt_CPU_Process - 1].IO_Burst_Timing) {
				left_IO[5][Currunt_CPU_Process - 1] = -CPU_Time;//I/O 대기 시작 시점을 음수로 저장
				Enqueue(Waiting_Queue, Currunt_CPU_Process);//해당 프로세스 I/O 대기 큐로 이동
				//간트차트 기록+인덱스 증가
				CPU_Info[0][flag] = CPU_Time;
				CPU_Info[1][flag] = Currunt_CPU_Process;
				flag++;
				//cpu 비우고 I/O 처리 기다림
				Currunt_CPU_Process = -1;
			}
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 2-5. I/O 처리
//     a) 현재 I/O 프로세스 1초 수행
//     b) 완료 시 Ready로 복귀 + Ready Queue 정렬 + 필요 시 선점
//     c) I/O 장치가 비었으면 대기 중 하나 꺼내기

		// a) 현재 I/O 프로세스가 있다면 1초 작업 감소 
		if (Currunt_IO_Process != -1)
			process[Currunt_IO_Process - 1].IO_Burst_Time--;
		
		// b) 완료 시 Ready로 복귀 + Ready Queue 정렬 + 필요 시 선점
		
		//I/O 작업이 모두 끝났다면? 처리할 작업 남았나 확인
		if (Currunt_IO_Process != -1 && process[Currunt_IO_Process - 1].IO_Burst_Time == 0) {
			//남아 있다면? 레디큐에 다시 넣는다
			if (process[Currunt_IO_Process - 1].CPU_Burst_Time > 0) {
				Enqueue(Ready_Queue, Currunt_IO_Process);

				// Ready Queue 재정렬 (Priority 내림차순)
				for (int i = 0; i < Ready_Queue->cnt; i++) {
					for (int j = i + 1; j < Ready_Queue->cnt; j++) {
						if (process[Ready_Queue->ID[i] - 1].Priority < process[Ready_Queue->ID[j] - 1].Priority) {
							int temp = Ready_Queue->ID[i];
							Ready_Queue->ID[i] = Ready_Queue->ID[j];
							Ready_Queue->ID[j] = temp;
						}
					}
				}

				// 정렬 후에 현재 cpu 프로세스와 비교하여 선점 조건 검사
				if (Currunt_CPU_Process != -1 &&
					process[Currunt_CPU_Process - 1].Priority > process[Ready_Queue->ID[0] - 1].Priority) {
					//간트차트 기록+인덱스 증가
					CPU_Info[0][flag] = CPU_Time;
					CPU_Info[1][flag] = Currunt_CPU_Process;
					flag++;
					//프로세스 교체
					int temp = Ready_Queue->ID[0];
					Ready_Queue->ID[0] = Currunt_CPU_Process;
					Currunt_CPU_Process = temp;
				}
			}
			//I/O 비우기
			Currunt_IO_Process = -1;
		}

//   c) I/O 장치가 비었으면 대기 중 하나 꺼내서 I/O 장치에 할당
		if (Currunt_IO_Process == -1 && Waiting_Queue->cnt > 0) {
			Currunt_IO_Process = Waiting_Queue->ID[0];
			left_IO[5][Currunt_IO_Process - 1] += CPU_Time;//대기 시간 측정을 위해
			Dequeue(Waiting_Queue);
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 3. 결과 출력 및 기록
	printf("P_Priority:\n");
	Print_Gantt_Chart(CPU_Info, flag);

	float P_Priority_Average_Waiting_Time = Calculate_Average_Waiting_Time(Process_Info, Process_Cnt, CPU_Info, flag, 5);
	float P_Priority_Average_Turnaround_Time = Calculate_Average_Turnaround_Time(Process_Info, Process_Cnt, CPU_Info, flag);
	float P_Priority_CPU_Utilization = Calculate_CPU_Utilization(CPU_Info, flag);

	Average_Waiting_Time[5] = P_Priority_Average_Waiting_Time;
	Average_Turnaround_Time[5] = P_Priority_Average_Turnaround_Time;
	CPU_Utilization[5] = P_Priority_CPU_Utilization;

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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 6가지 스케줄링 알고리즘을 차례대로 실행하는 함수
void Schedule(Process Process_Info[], int Process_Cnt, int quantum) {
	FCFS(Process_Info, Process_Cnt);           // 1. FCFS
	SJF(Process_Info, Process_Cnt);            // 2. SJF
	Priority(Process_Info, Process_Cnt);       // 3. Priority
	RR(Process_Info, Process_Cnt, quantum);    // 4. RR (quantum 사용)
	P_SJF(Process_Info, Process_Cnt);          // 5. Preemptive SJF
	P_Priority(Process_Info, Process_Cnt);     // 6. Preemptive Priority
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 스케줄링 평가 지표 출력 및 정렬 (평균 대기시간 / 반환시간 / CPU 활용률)
void Evaluation() {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. Rank_Name 초기화
// 2. 지표별로 정렬 수행 (오름차순 또는 내림차순)
//   - Average_Waiting_Time: 오름차순
//   - Average_Turnaround_Time: 오름차순
//   - CPU_Utilization: 내림차순
// 3. 결과 출력
//   - 평균 대기 시간 순위
//   - 평균 반환 시간 순위
//   - CPU 활용률 순위
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float temp;//값 스왑을 위한 임시 변수
	char Rank_Name[6][11];//최대 6개 알고리즘, 각 이름 최대 10(null 포함해서 11)

	// 1. 스케줄링 알고리즘 이름 저장
	strcpy(Rank_Name[0], "FCFS");
	strcpy(Rank_Name[1], "SJF");
	strcpy(Rank_Name[2], "Priority");
	strcpy(Rank_Name[3], "RR");
	strcpy(Rank_Name[4], "P_SJF");
	strcpy(Rank_Name[5], "P_Priority");

	// 2. 각 지표에 대해 정렬 수행 (단순 버블정렬)
	for (int i = 0; i < 6; i++) {
		for (int j = i + 1; j < 6; j++) {

			// 평균 대기 시간 오름차순 정렬
			if (Average_Waiting_Time[i] > Average_Waiting_Time[j]) {
				temp = Average_Waiting_Time[i];
				Average_Waiting_Time[i] = Average_Waiting_Time[j];
				Average_Waiting_Time[j] = temp;

				temp = Rank_Waiting_Time[i];
				Rank_Waiting_Time[i] = Rank_Waiting_Time[j];
				Rank_Waiting_Time[j] = temp;
			}

			// 평균 반환 시간 오름차순 정렬
			if (Average_Turnaround_Time[i] > Average_Turnaround_Time[j]) {
				temp = Average_Turnaround_Time[i];
				Average_Turnaround_Time[i] = Average_Turnaround_Time[j];
				Average_Turnaround_Time[j] = temp;

				temp = Rank_Turnaround_Time[i];
				Rank_Turnaround_Time[i] = Rank_Turnaround_Time[j];
				Rank_Turnaround_Time[j] = temp;
			}

			// CPU 활용률 내림차순 정렬
			if (CPU_Utilization[i] < CPU_Utilization[j]) {
				temp = CPU_Utilization[i];
				CPU_Utilization[i] = CPU_Utilization[j];
				CPU_Utilization[j] = temp;

				temp = Rank_CPU_Utilization[i];
				Rank_CPU_Utilization[i] = Rank_CPU_Utilization[j];
				Rank_CPU_Utilization[j] = temp;
			}
		}
	}

	// 3. 결과 출력
	printf("\nRank of Average Waiting Time: \n");
	for (int i = 0; i < 6; i++) {
		printf("%d. %s : %.1f\n", i + 1, Rank_Name[Rank_Waiting_Time[i]], Average_Waiting_Time[i]);
	}
	printf("\n");

	printf("\nRank of Average Turnaround Time: \n");
	for (int i = 0; i < 6; i++) {
		printf("%d. %s : %.1f\n", i + 1, Rank_Name[Rank_Turnaround_Time[i]], Average_Turnaround_Time[i]);
	}
	printf("\n");

	printf("\nRank of CPU Utilization: \n");
	for (int i = 0; i < 6; i++) {
		printf("%d. %s : %.0f%%\n", i + 1, Rank_Name[Rank_CPU_Utilization[i]], CPU_Utilization[i] * 100);
	}
	printf("\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//<정리용>
// 1. 프로세스 수 및 Quantum 설정
// 2. 랜덤 시드 초기화
// 3. 프로세스 생성
// 4. 프로세스 정보 출력
// 5. 스케줄링 실행 (Schedule 함수)
// 6. 성능 평가 출력 (Evaluation 함수)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. 프로세스 수 및 Quantum 설정
	int Process_Cnt = 5;
	int quantum = 4;

	// 2. 랜덤 시드 설정
	srand(time(NULL));

	// 3. 프로세스 정보 배열 선언 및 생성
	Process Process_Info[5];
	for (int i = 0; i < Process_Cnt; i++) {
		Process_Info[i] = Create_Process(i + 1);
	}

	// 4. 프로세스 정보 출력
	Print_Process(Process_Info, Process_Cnt);

	// 5. 스케줄링 실행
	Schedule(Process_Info, Process_Cnt, quantum);

	// 6. 평가 지표 출력
	Evaluation();

	return 0;
}