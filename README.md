# VirtualMemoryManager
This is a project done for a Operating Systems class. It's a C program that is a standalone virtual memory manager simulator. It takes in a 'task.list' file that includes processes with their task PID, arrival Time, and burst time. It simulates the processes running using various scheduling algorithms and prints out statistics in the end including average waiting time, average response time, average turnaround time, and overall cpu usage. 

## Technologies Used
C, make

## How to Run
1. Clone the repo.
2. Compile the code using the make file `make` or manual compilation `gcc -c open.c` `gcc -c read.c` `gcc -c print.c` `gcc open.o read.o print.o scheduler.c -o scheduler`
3. (Optional) Change the task.list values – each line is a new task/process along with their PID, arrival time, and burst time. Make sure to keep the same formatting.
4. Run the code `./scheduler <file_name> <FCFS | RR | SRTF> <time_quantum>`

## Scheduler Algorithms Supported
- First Come First Serve
- Round Robin
- Shortest Remaining Time First

