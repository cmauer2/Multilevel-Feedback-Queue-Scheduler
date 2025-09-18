//Carter, some of this is AI generated (For tidying and assitance).
//I will clarify what is generated for you. 
//The homework document makes no limitation on the usage of AI either.

//Also when making adjustments make sure you recompile the code.
//Use the following command:
//gcc -o mlfqs.exe mlfqs.c prioque.c -lpthread

//For ease of access when inputting data preform the folling in the terminal:
    //type test_input1.txt |.\mlfqs
    //type test_input2.text |.\mlfqs
//This will automatclly inputt he required data. I have taken the input from both SampleOutput.txt files
//and isolated them for ease if input. The top one is from Sample1 and the Bottom is from Sample2.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prioque.h"

//Constants for the MLFQS
#define MAX_LEVELS 4       
#define MAX_PROCESSES 100  
#define IDLE_PID 0         

//Quantum for Each Level
const int TIME_QUANTUM[] = {10, 30, 100, 200};


typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    FINISHED
} ProcessState;


typedef struct {
    int pid;              // Process ID
    int unique_id;        // Unique identifier for each process
    int arrival_time;     // Time when process arrives
    int burst_time;       // Total CPU time needed
    int io_frequency;     // How often the process performs I/O
    int io_duration;      // How long I/O operations take
    int remaining_time;   // Remaining CPU time
    int level;            // Current priority level (0-3, with 0 being highest)
    int time_in_level;    // Time spent at current level
    ProcessState state;   // Current state of the process
    int next_io_time;     // When the next I/O will occur
    int repeat_count;     // Number of times to repeat the process
    int original_burst;   // Original burst time (for repeating)
} Process;

// Structure to store process input data
typedef struct {
    int arrival_time;
    int pid;
    int burst_time;
    int io_frequency;
    int repeat_count;
} ProcessInput;

// Queues for each priority level (Ready & Blocked)
Queue ready_queues[MAX_LEVELS];
Queue blocked_queue;

    //Active Process Data and Inputs
    ProcessInput process_inputs[MAX_PROCESSES];
    int input_count = 0;

    Process processes[MAX_PROCESSES];
    int process_count = 0;

    //Current time
    int current_time = 0;

    //Currently running process
    Process *running_process = NULL;


//Process Tracking (Idle & Current Usage)
int idle_time = 0;
int cpu_usage[MAX_PROCESSES + 1] = {0}; // +1 for the idle process

// Function prototypes (AI GENERATED)
void init_scheduler();
void read_processes();
void run_scheduler();
void handle_process_arrival(int arrival_time);
void handle_process_completion(Process *proc);
void handle_io_block(Process *proc);
void handle_io_completion();
void handle_quantum_expiration(Process *proc);
void schedule_next_process();
int process_compare(const void *a, const void *b);
void print_process_stats();
void update_process_state(Process *proc, ProcessState new_state, int new_level);
Process create_process(int pid, int arrival_time, int burst_time, int io_freq, int repeat_count);


//Process comparison function for the priority queue
//This is to ensure that processes are compared by their unique IDs and to make sure they have their own unique ID.
int process_compare(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    
    //Compare unique IDs for equality
    return (p1->unique_id == p2->unique_id) ? 0 : 1;
}


//Initialization of the MLFQS scheduler
void init_scheduler() {
    int i;
    
    // Initialize ready queues for each priority level
    for (i = 0; i < MAX_LEVELS; i++) {
        init_queue(&ready_queues[i], sizeof(Process), TRUE, process_compare, TRUE);
    }
    
    // Initialize blocked queue
    init_queue(&blocked_queue, sizeof(Process), TRUE, process_compare, TRUE);
    
    // Reset current time
    current_time = 0;
    
    // No running process initially
    running_process = NULL;
    
    // Reset CPU usage tracking
    memset(cpu_usage, 0, sizeof(cpu_usage));
}

// Read process information from standard input
void read_processes() {
    char line[256];
    
    printf("Queue entries one per line:\n");
    
    // Read until EOF or empty line
    while (fgets(line, sizeof(line), stdin) && line[0] != '\n') {
        // Print the input line
        printf("%s", line);
        
        // Parse the line
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

//Creating a new process
Process create_process(int pid, int arrival_time, int burst_time, int io_freq, int repeat_count) {
    Process new_proc;
    new_proc.pid = pid;
    new_proc.unique_id = process_count;
    new_proc.arrival_time = arrival_time;
    new_proc.burst_time = burst_time;
    new_proc.original_burst = burst_time;
    new_proc.io_frequency = io_freq;
    new_proc.io_duration = 20;
    new_proc.remaining_time = burst_time;
    new_proc.level = 0;
    new_proc.time_in_level = 0;
    new_proc.state = READY;
    new_proc.next_io_time = io_freq;
    new_proc.repeat_count = repeat_count;
    return new_proc;
}

// Simplified handle_process_arrival (AI GENERATED)
void handle_process_arrival(int arrival_time) {
    for (int i = 0; i < input_count; i++) {
        if (process_inputs[i].arrival_time == arrival_time) {
            // Check if process already exists
            int exists = 0;
            for (int j = 0; j < process_count; j++) {
                if (processes[j].pid == process_inputs[i].pid && 
                    processes[j].arrival_time == process_inputs[i].arrival_time &&
                    processes[j].state != FINISHED) {
                    exists = 1;
                    break;
                }
            }
            
            if (!exists) {
                Process new_proc = create_process(
                    process_inputs[i].pid,
                    process_inputs[i].arrival_time,
                    process_inputs[i].burst_time,
                    process_inputs[i].io_frequency,
                    process_inputs[i].repeat_count
                );
                
                processes[process_count++] = new_proc;
                add_to_queue(&ready_queues[0], &new_proc, 0);
                
                printf("PID: %d, ARRIVAL TIME: %d\n", new_proc.pid, new_proc.arrival_time);
                printf("CREATE: Process %d entered the ready queue at time %d\n", 
                       new_proc.pid, current_time);
            }
        }
    }
}

// Simplified handle_io_completion
void handle_io_completion() {
    rewind_queue(&blocked_queue);
    while (!end_of_queue(&blocked_queue)) {
        Process proc;
        peek_at_current(&blocked_queue, &proc);
        
        if (proc.arrival_time <= current_time) {
            proc.state = READY;
            
            // Update process in array
            for (int j = 0; j < process_count; j++) {
                if (processes[j].unique_id == proc.unique_id) {
                    processes[j].state = READY;
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

// Simplified schedule_next_process
void schedule_next_process() {
    for (int i = 0; i < MAX_LEVELS; i++) {
        if (!empty_queue(&ready_queues[i])) {
            Process proc;
            rewind_queue(&ready_queues[i]);
            peek_at_current(&ready_queues[i], &proc);
            delete_current(&ready_queues[i]);
            
            proc.state = RUNNING;
            
            for (int j = 0; j < process_count; j++) {
                if (processes[j].unique_id == proc.unique_id) {
                    processes[j].state = RUNNING;
                    running_process = &processes[j];
                    
                    printf("RUN: Process %d started execution from level %d at time %d; wants to execute for %d ticks.\n", 
                           running_process->pid, running_process->level + 1, 
                           current_time, running_process->remaining_time);
                    return;
                }
            }
        }
    }
    running_process = NULL;
}

// Combined process state management function
void update_process_state(Process *proc, ProcessState new_state, int new_level) {
    proc->state = new_state;
    if (new_level >= 0) {
        proc->level = new_level;
    }
    proc->time_in_level = 0;
    
    //Updating Queues based on new state of process
    switch (new_state) {
        case READY:
            add_to_queue(&ready_queues[proc->level], proc, 0);
            printf("QUEUED: Process %d queued at level %d at time %d.\n", 
                   proc->pid, proc->level + 1, current_time);
            break;
        case BLOCKED:
            add_to_queue(&blocked_queue, proc, 0);
            printf("I/O: Process %d blocked for I/O at time %d.\n", 
                   proc->pid, current_time);
            break;
        case FINISHED:
            printf("FINISHED: Process %d finished at time %d.\n", 
                   proc->pid, current_time);
            break;
        default:
            break;
    }
}

//Simplified handle_process_completion (AI GENERATED)
void handle_process_completion(Process *proc) {
    update_process_state(proc, FINISHED, -1);
    
    if (proc->repeat_count > 0) {
        Process new_proc;
        new_proc.pid = proc->pid;
        new_proc.unique_id = process_count;
        new_proc.arrival_time = current_time + proc->io_duration;
        new_proc.burst_time = proc->original_burst;
        new_proc.original_burst = proc->original_burst;
        new_proc.io_frequency = proc->io_frequency;
        new_proc.io_duration = proc->io_duration;
        new_proc.remaining_time = proc->original_burst;
        new_proc.level = 0;
        new_proc.time_in_level = 0;
        new_proc.state = BLOCKED;
        new_proc.next_io_time = proc->io_frequency;
        new_proc.repeat_count = proc->repeat_count - 1;
        
        processes[process_count++] = new_proc;
        add_to_queue(&blocked_queue, &new_proc, 0);
    }
    
    running_process = NULL;
}

// Simplified handle_io_block
void handle_io_block(Process *proc) {
    update_process_state(proc, BLOCKED, -1);
    proc->next_io_time = proc->io_frequency;
    running_process = NULL;
}

// Simplified handle_quantum_expiration
void handle_quantum_expiration(Process *proc) {
    int new_level = (proc->level < MAX_LEVELS - 1) ? proc->level + 1 : proc->level;
    update_process_state(proc, READY, new_level);
    running_process = NULL;
}

// Simplified run_scheduler
void run_scheduler() {
    int last_event_time = 0;
    
    // Find last arrival time
    for (int i = 0; i < input_count; i++) {
        last_event_time = (process_inputs[i].arrival_time > last_event_time) ? 
                          process_inputs[i].arrival_time : last_event_time;
    }
    
    while (1) {
        handle_process_arrival(current_time);
        handle_io_completion();
        
        if (running_process == NULL) {
            schedule_next_process();
            if (running_process == NULL) {
                idle_time++;
                cpu_usage[0]++;
            }
        }
        
        if (running_process != NULL) {
            running_process->remaining_time--;
            running_process->time_in_level++;
            cpu_usage[running_process->unique_id + 1]++;
            
            if (running_process->remaining_time <= 0) {
                handle_process_completion(running_process);
            } else if (running_process->next_io_time > 0 && 
                      running_process->time_in_level >= running_process->next_io_time) {
                handle_io_block(running_process);
            } else if (running_process->time_in_level >= TIME_QUANTUM[running_process->level]) {
                handle_quantum_expiration(running_process);
            }
        }
        
        current_time++;
        
        // Check if all processes are done
        if (current_time > last_event_time) {
            int all_done = 1;
            for (int i = 0; i < process_count && all_done; i++) {
                if (processes[i].state != FINISHED) {
                    all_done = 0;
                }
            }
            
            for (int i = 0; i < MAX_LEVELS && all_done; i++) {
                if (!empty_queue(&ready_queues[i])) {
                    all_done = 0;
                }
            }
            
            if (all_done && empty_queue(&blocked_queue)) {
                break;
            }
        }
    }
    
    printf("Scheduler shutdown at time %d.\n", current_time - 1);
}

// Simplified print_process_stats
void print_process_stats() {
    int pid_usage[MAX_PROCESSES] = {0};
    int unique_pids[MAX_PROCESSES] = {0};
    int pid_count = 0;
    
    printf("Total CPU usage for all processes scheduled:\n");
    printf("Process <<null>>:\t%d time units.\n", cpu_usage[0]);
    
    // Collect unique PIDs and aggregate CPU usage
    for (int i = 0; i < process_count; i++) {
        int pid = processes[i].pid;
        int pid_idx = -1;
        
        // Find or add PID to unique list
        for (int j = 0; j < pid_count; j++) {
            if (unique_pids[j] == pid) {
                pid_idx = j;
                break;
            }
        }
        if (pid_idx == -1) {
            pid_idx = pid_count;
            unique_pids[pid_count++] = pid;
        }
        
        pid_usage[pid_idx] += cpu_usage[processes[i].unique_id + 1];
    }
    
    // Print usage for each unique PID
    for (int i = 0; i < pid_count; i++) {
        printf("Process %d:\t\t%d time units.\n", unique_pids[i], pid_usage[i]);
    }
}

//Main function
int main() {

    init_scheduler();
    read_processes();
    run_scheduler();
    print_process_stats();

    return 0;
}
