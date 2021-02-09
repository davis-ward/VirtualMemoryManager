/*
 * Davis Ward
 * 
 * How to compile using Makefile?
 * $make
 *
 * How to manually compile?
 * $gcc -c open.c
 * $gcc -c read.c
 * $gcc -c print.c
 * $gcc open.o read.o print.o scheduler.c -o scheduler
 *
 * How to run?
 * 
 * Usage: $ ./scheduler <file_name> <FCFS|RR|SRTF> <time_quantum>
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler.h"
#include "print.h"
#include "open.h"
#include "read.h"

int RR(task_t task_list[], int size, int quantum);
int FCFS(task_t task_list[], int size);
int SRTF(task_t task_list[], int size, int chosen);
void move_to_front(task_t task_list[], int size, int index);
int sort_processes(task_t task_array[], u_int size);
void rotate_task(task_t input_array[], u_int size, int index);
void calculate_statistics(task_t completion_task_array[], int completion_count, u_int process_running_time, u_int time);


int main( int argc, char *argv[] )  {
    char *file_name; /* file name from the commandline */
    FILE *fp; /* file descriptor */

    task_t task_array[MAX_TASK_NUM];
    task_t completion_task_array[MAX_TASK_NUM];
    task_t ready_array[MAX_TASK_NUM];

    char *algorithm_type; /* Stores the algorithm type */

    int error_code;
    u_int time = 0;
    u_int process_running_time = 0;
    int quantum;

    u_int task_count = 0;
    u_int ready_count = 0;
    u_int completion_count = 0;

    /* Make sure enough arguments were given by the user */
    if (argc < 2 || argc > 4) {
        printf("Usage: command file_name [FCFS|RR|SRTF] [time_quantum]\n");
        return EXIT_FAILURE;
    }

    /* Make sure a valid algorithm type was inputed */
    if (strcmp(argv[2], "FCFS") != 0 && strcmp(argv[2], "RR") !=0 && strcmp(argv[2], "SRTF") !=0) {
        printf("Error: Not a valid algorithm type.\n");
        printf("Usage: command file_name [FCFS|RR|SRTF] [time_quantum]\n");
        return EXIT_FAILURE;
    }

    /* Handle if the algorithm was RR but [time_quantum] was not set */
    if (strcmp(argv[2], "RR") == 0) {
        if (argc == 4) {
            printf("The time_quantum was set to %s\n", argv[3]);
        } else {
            printf("A time_quantum is required for the RR Policy.\n");
            printf("Usage: command file_name [FCFS|RR|SRTF] [time_quantum]\n");
            return EXIT_FAILURE;
        }
        quantum = atoi(argv[3]);
    }

    /* set the algorithm type */
    algorithm_type = argv[2];

    /* try to open the file, if file cannot be opened throw an error */
    file_name = argv[1];
    error_code = open_file(file_name, &fp);
    if (error_code == 1) {
        printf("Error: File %s cannot be open. Please retry ...\n", file_name);
        return EXIT_FAILURE;
    }

    /* Read the file now that it is opened */
    read_file(fp, task_array, &task_count);

    /* If Round-Robin Scheduling Policy, set the quantum time var. */
    if (strcmp(algorithm_type, "RR") == 0) {
        for (int i = 0; i < task_count; i++) {
            task_array[i].quantum_time = quantum;
        }
    }

    /* sort the tasks */
    sort_processes(task_array, task_count);
    //print_task_list(task_array, task_count);
    printf("Scheduling Policy: %s\nThere are %d tasks loaded from the file named \"%s\". Press any key to continue ...\n", algorithm_type, task_count, file_name);
    getchar();
    printf("============================================================\n");
    int chosen = 0;
    /* simulate the scheduling of all task */
    while (task_count != 0 || ready_count != 0) {
        /* add arriving processes to the ready array */
        while (task_count > 0) {
            if (task_array[0].arrival_time <= time) {
                ready_array[ready_count] = task_array[0];
                ready_array[ready_count].arrival_time = time;
                ready_array[ready_count].remaining_time = task_array[0].burst_time;
                ready_array[ready_count].waiting_time = 0;
                ready_count++;
                rotate_task(task_array, task_count, 0);
                task_count--;
            } else { break; }
        }

        //print_task_list(ready_array, ready_count);

        if (ready_count > 0) {
            
            if (ready_array[chosen].remaining_time == 0) {
                printf("<time %d> process %d is finished...\n", time, ready_array[chosen].pid);
                completion_task_array[completion_count] = ready_array[chosen];
                completion_task_array[completion_count].completion_time = time;
                completion_count++;
                rotate_task(ready_array, ready_count, chosen);
                ready_count--;
            }

            if (ready_count > 0 && strcmp(algorithm_type, "SRTF") != 0) {
                printf("<time %d> process %d is running\n", time, ready_array[chosen].pid);
            }

            //int chosen = 0;
            if (strcmp(algorithm_type, "FCFS") == 0) {
                chosen = FCFS(ready_array, ready_count);
            } else if (strcmp(algorithm_type, "RR") == 0) {
                chosen = RR(ready_array, ready_count, quantum);
            } else if (strcmp(algorithm_type, "SRTF") == 0) {
                chosen = SRTF(ready_array, ready_count, chosen);
                //print_task_list(ready_array, ready_count);
            }

            if (ready_count > 0 && strcmp(algorithm_type, "SRTF") == 0) {
                printf("<time %d> process %d is running\n", time, ready_array[chosen].pid);
            }

            process_running_time++;

            for (int i = 0; i < ready_count; i++) {
                if (ready_array[i].remaining_time == ready_array[i].burst_time) {
                    ready_array[i].waiting_time++;
                }
            }
        } else {
            printf("<time %d> no process is running...\n", time);
        }

        for (int i = 0; i < ready_count; i++) {
                //printf("Process %d waiting time: %d remaining time: %d\n", ready_array[i].pid, ready_array[i].waiting_time, ready_array[i].remaining_time);
            }
        if (ready_count > 0) {
            time++;
        }
    }
    printf("<time %d> All processes finish ......\n", time);
    calculate_statistics(completion_task_array, completion_count, process_running_time, time);
    fclose(fp);
    return EXIT_SUCCESS;
}

/* this sorts the task array from lowest arrival time to greatest, returns 1 if it succeeds */
int sort_processes(task_t task_array[], u_int size) {
    for (int i = 0; i < size-1; i++)
    {
        int min = i;
        for(int j = i+1; j < size - 1; j++)
        {
            if(task_array[j].arrival_time < task_array[min].arrival_time)
            { 
                min = j;
                task_t temp = task_array[i];
                task_array[i] = task_array[j];
                task_array[j] = temp;
            }
        }
    }
    return 1;
}

/* This moves a specific task from the given array to last index within the task_count and shifts everything over to the left once. */
void rotate_task(task_t input_array[], u_int size, int index) {
    task_t holder = input_array[index];
    for (int x = index; x < size; x++) {
        input_array[x] = input_array[x+1];
    }
    input_array[size - 1] = holder;
}

/*
 * Returns a index of the chosen process based on round-robin scheduling * policies.
 */
int RR(task_t task_list[], int size, int quantum) {

    /* see if the process currently running has gone over the given quantum time */
    if (task_list[0].quantum_time == 0) {
        task_list[0].quantum_time = quantum;
        rotate_task(task_list, size, 0);
    }
    task_list[0].remaining_time--;
    task_list[0].quantum_time--;
    return 0;
}

/*
 * Returns a index of the chosen process based on first come first serve scheduling * policies.
 */
int FCFS(task_t task_list[], int size) {
    task_list[0].remaining_time--;
    return 0;
}

/*
 * Returns a index of the chosen process based on shortest remaining time first scheduling * policies.
 */
int SRTF(task_t task_list[], int size, int chosen) {
    int shortest_chosen = chosen;
    for (int i = 0; i < size; i++) {
        if (task_list[i].remaining_time < task_list[shortest_chosen].remaining_time) {
            shortest_chosen = i;
        }
    }
    //move_to_front(task_list, size, shortest_chosen);
    task_list[shortest_chosen].remaining_time--;
    return shortest_chosen;
}

void move_to_front(task_t task_list[], int size, int index) {
    task_t holder = task_list[0];
    task_list[0] = task_list[index];
    task_list[index] = holder;
}

void calculate_statistics(task_t completion_task_array[], int completion_count, u_int process_running_time, u_int time) {
    double avg_wait = 0;
    double avg_response = 0;
    double avg_turnaround = 0;
    double cpu_usage = (double)(process_running_time/time)*100;

    for (int i = 0; i < completion_count; i++) {
        avg_wait += (double)completion_task_array[i].waiting_time;
        avg_response += (double)completion_task_array[i].completion_time - completion_task_array[i].burst_time - completion_task_array[i].arrival_time;
        avg_turnaround += (double)completion_task_array[i].completion_time - completion_task_array[i].arrival_time;

        printf("process %d waiting final time: %d\n", completion_task_array[i].pid, completion_task_array[i].waiting_time);
    }

    avg_wait = avg_wait;
    avg_wait = avg_wait/(double)completion_count;
    avg_response = avg_response/(double)completion_count;
    avg_turnaround = avg_turnaround/(double)completion_count;
    printf("============================================================\n");
    printf("Average Waiting time: %.2f\n", avg_wait);
    printf("Average Response time: %.2f\n", avg_response);
    printf("Average Turnaround time: %.2f\n", avg_turnaround);
    printf("Overall CPU usage: %.2f%%\n", cpu_usage);
    printf("============================================================\n");
}
