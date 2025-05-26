OS Term Project - CPU Scheduling Simulator

Term Project 의 목적
• OS의 기본 개념을 바탕으로 CPU 스케줄러의 기능 및 작동 원리를 이해하는데 있다.
• Project 의 수행을 통해 Linux 환경에서의 코딩 능력을 향상시킬 수 있다.

CPU Scheduling Simulator 의 구성 (예시 함수)
• Create_Process(): 실행할 프로세스를 생성하고 각 프로세스에 데이터가 주어진다. (Random data 부여)
  o Process ID
  o Arrival time
  o CPU burst time
  o I/O burst time / I/O request time o Priority
• Config(): 시스템 환경 설정
  o Ready Queue / Waiting Queue
• Schedule(): CPU 스케줄링 알고리즘을 구현한다. o FCFS(First Come First Served)
  o SJF(Shortest Job First)
  o Priority
  o RR(Round Robin)
  o Preemptive 방식 적용 – SJF, Priority
• Evaluation(): 각 스케줄링 알고리즘들간 비교 평가한다.
  o Average waiting time
  o Average turnaround time

보고서
보고서의 양식은 서론, 본론, 결론, 참고문헌, 부록 등의 순서를 따른다. 
o 서론
  * CPU 스케줄러의 개념 등
  * 본인이 구현한 스케줄러에 대한 요약 설명(특징 등)
o 본론
  * 다른 CPU 스케줄링 시뮬레이터에 대한 소개
  * 본인이 구현한 시뮬레이터의 시스템 구성도
  * 각 모듈에 대한 설명(알고리즘으로 표현)
  * 시뮬레이터 실행 결과 화면
  * 알고리즘들간의 성능 비교 등
o 결론
  * 구현한 시뮬레이터에 대한 정리
  * 프로젝트 수행 소감 및 향후 발전방향 등

소스코드는 보고서 마지막에 <부록>으로 첨부
<img width="466" alt="Monosnap CPU Scheduling Simulator pdf 2025-05-26 17-45-04" src="https://github.com/user-attachments/assets/72707a98-fe3c-4ca3-b699-8cc7bf832bc8" />

