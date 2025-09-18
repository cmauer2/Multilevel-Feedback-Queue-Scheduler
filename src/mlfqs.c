// Updated mlfqs.c to match SampleOutput1.txt exactly
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prioque.h"

#define MAX_LEVELS 4
#define MAX_PROCESSES 100
#define IDLE_PID 0

const int TIME_QUANTUM[MAX_LEVELS] = {10, 30, 100, 200};
const int demotion_threshold[MAX_LEVELS] = {1, 2, 2, 2};
const int promotion_threshold[MAX_LEVELS] = {0, 1, 2, 2};

typedef enum { READY, RUNNING, BLOCKED, FINISHED } ProcessState;

typedef struct {
    int pid, unique_id, arrival_time, burst_time, io_frequency, io_duration;
    int remaining_time, level, repeat_count, original_burst;
    int b_count, g_count, quantum_used, next_io_time, io_finish_time;
    ProcessState state;
} Process;

typedef struct {
    int arrival_time, pid, burst_time, io_frequency, repeat_count;
} ProcessInput;

Queue ready_queues[MAX_LEVELS];
Queue blocked_queue;
ProcessInput process_inputs[MAX_PROCESSES];
Process processes[MAX_PROCESSES];

int input_count = 0, process_count = 0, current_time = 0;
Process *running_process = NULL;
int idle_time = 0, cpu_usage[MAX_PROCESSES + 1] = {0};

void init_scheduler();
void read_processes();
void handle_process_arrival(int time);
void handle_io_completion();
void schedule_next_process();
void update_process_state(Process *proc, ProcessState new_state, int new_level);
Process create_process(int pid, int arrival_time, int burst_time, int io_freq, int repeat_count);
int unfinished_processes_exist();

int main() {
    init_scheduler();
    read_processes();

    while (unfinished_processes_exist() || !empty_queue(&blocked_queue)) {
        handle_process_arrival(current_time);
        handle_io_completion();

        if (running_process) {
            for (int lvl = 0; lvl < running_process->level; lvl++) {
                if (!empty_queue(&ready_queues[lvl])) {
                    printf("PREEMPT: Process %d preempted at time %d.\n", running_process->pid, current_time);
                    update_process_state(running_process, READY, running_process->level);
                    running_process = NULL;
                    break;
                }
            }
        }

        if (!running_process) schedule_next_process();

        if (!running_process) {
            idle_time++;
        } else {
            running_process->remaining_time--;
            running_process->quantum_used++;
            cpu_usage[running_process->pid]++;

            if (running_process->remaining_time > 0 &&
                running_process->quantum_used == running_process->next_io_time) {
                running_process->g_count++;
                running_process->b_count = 0;
                printf("I/O: Process %d blocked for I/O at time %d.\n", running_process->pid, current_time + 1);
                running_process->state = BLOCKED;
                running_process->io_finish_time = current_time + running_process->io_duration;
                add_to_queue(&blocked_queue, running_process, 0);
                running_process = NULL;
            } else if (running_process->remaining_time == 0) {
                if (running_process->repeat_count > 0) {
                    printf("I/O: Process %d blocked for I/O at time %d.\n", running_process->pid, current_time + 1);
                    running_process->state = BLOCKED;
                    running_process->io_finish_time = current_time + running_process->io_duration;
                    running_process->repeat_count--;
                    running_process->remaining_time = running_process->original_burst;
                    running_process->quantum_used = 0;
                    running_process->next_io_time = running_process->io_frequency;
                    add_to_queue(&blocked_queue, running_process, 0);
                } else {
                    running_process->state = FINISHED;
                    printf("FINISHED: Process %d finished at time %d.\n", running_process->pid, current_time + 1);
                }
                running_process = NULL;
            } else if (running_process->quantum_used == TIME_QUANTUM[running_process->level]) {
                running_process->b_count++;
                running_process->quantum_used = 0;
                if (running_process->b_count >= demotion_threshold[running_process->level] &&
                    running_process->level < MAX_LEVELS - 1) {
                    running_process->level++;
                    running_process->b_count = 0;
                }
                printf("QUEUED: Process %d queued at level %d at time %d.\n",
                       running_process->pid, running_process->level + 1, current_time + 1);
                update_process_state(running_process, READY, running_process->level);
                running_process = NULL;
            }
        }
        current_time++;
    }

    printf("Scheduler shutdown at time %d.\n", current_time);
    printf("Total CPU usage for all processes scheduled:\n");
    printf("Process <<null>>:\t%d time units.\n", idle_time);
    for (int i = 0; i < process_count; i++) {
        printf("Process %d:\t\t%d time units.\n", processes[i].pid, cpu_usage[processes[i].pid]);
    }
    return 0;
}

void init_scheduler() {
    for (int i = 0; i < MAX_LEVELS; i++)
        init_queue(&ready_queues[i], sizeof(Process), 1, NULL, 1);
    init_queue(&blocked_queue, sizeof(Process), 1, NULL, 1);
    current_time = 0;
    running_process = NULL;
    memset(cpu_usage, 0, sizeof(cpu_usage));
}

void read_processes() {
    char line[256];
    printf("Queue entries one per linee:\n");
    while (fgets(line, sizeof(line), stdin) && line[0] != '\n') {
        printf("%s", line);
        if (sscanf(line, "%d %d %d %d %d",
                   &process_inputs[input_count].arrival_time,
                   &process_inputs[input_count].pid,
                   &process_inputs[input_count].burst_time,
                   &process_inputs[input_count].io_frequency,
                   &process_inputs[input_count].repeat_count) == 5) {
            input_count++;
        }
    }
}

Process create_process(int pid, int arrival_time, int burst_time, int io_freq, int repeat_count) {
    Process p = { pid, process_count, arrival_time, burst_time, io_freq, 20, burst_time, 0,
                  READY, repeat_count, burst_time, 0, 0, 0, io_freq, 0 };
    return p;
}

void handle_process_arrival(int time) {
    for (int i = 0; i < input_count; i++) {
        if (process_inputs[i].arrival_time == time) {
            Process p = create_process(process_inputs[i].pid,
                                       process_inputs[i].arrival_time,
                                       process_inputs[i].burst_time,
                                       process_inputs[i].io_frequency,
                                       process_inputs[i].repeat_count);
            processes[process_count++] = p;
            add_to_queue(&ready_queues[0], &processes[process_count - 1], 0);
            printf("PID: %d, ARRIVAL TIME: %d\n", p.pid, p.arrival_time);
            printf("CREATE: Process %d entered the ready queue at time %d\n", p.pid, time);
        }
    }
}

void handle_io_completion() {
    rewind_queue(&blocked_queue);
    while (!end_of_queue(&blocked_queue)) {
        Process proc;
        peek_at_current(&blocked_queue, &proc);
        if (proc.io_finish_time <= current_time) {
            proc.state = READY;
            proc.quantum_used = 0;
            proc.next_io_time = proc.io_frequency;
            printf("I/O COMPLETE: Process %d completed I/O at time %d.\n", proc.pid, current_time);
            for (int j = 0; j < process_count; j++) {
                if (processes[j].unique_id == proc.unique_id) {
                    processes[j] = proc;
                    break;
                }
            }
            add_to_queue(&ready_queues[proc.level], &proc, 0);
            delete_current(&blocked_queue);
            rewind_queue(&blocked_queue);
        } else {
            next_element(&blocked_queue);
        }
    }
}

void schedule_next_process() {
    for (int i = 0; i < MAX_LEVELS; i++) {
        if (!empty_queue(&ready_queues[i])) {
            Process proc;
            rewind_queue(&ready_queues[i]);
            peek_at_current(&ready_queues[i], &proc);
            delete_current(&ready_queues[i]);
            for (int j = 0; j < process_count; j++) {
                if (processes[j].unique_id == proc.unique_id) {
                    processes[j].state = RUNNING;
                    running_process = &processes[j];
                    printf("RUN: Process %d started execution from level %d at time %d; wants to execute for %d ticks.\n",
                           running_process->pid, running_process->level + 1, current_time, running_process->remaining_time);
                    return;
                }
            }
        }
    }
    running_process = NULL;
}

void update_process_state(Process *proc, ProcessState new_state, int new_level) {
    proc->state = new_state;
    if (new_level >= 0) proc->level = new_level;
    proc->quantum_used = 0;
    if (new_state == READY)
        add_to_queue(&ready_queues[proc->level], proc, 0);
}

int unfinished_processes_exist() {
    for (int i = 0; i < process_count; i++) {
        if (processes[i].state != FINISHED)
            return 1;
    }
    return 0;
}
