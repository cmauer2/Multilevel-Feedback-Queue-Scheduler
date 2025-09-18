# Multilevel Feedback Queue Scheduler (MLFQS)

This project implements a **four-level multilevel feedback queue scheduler** in C, based on the requirements of CSC 4103 Programming Assignment #3 at LSU.

## 📂 Project Structure
- **src/**
  - `mlfqs.c` – Main scheduler implementation
  - `OLDmlfqs.c` – Previous version kept for reference
  - `prioque.c`, `prioque.h` – Priority queue library (thread-safe)
  - `test-prioque.c` – Demonstrates usage of the priority queue
- **inputs/**
  - `test_input.txt`, `test_input1.txt`, `test_input2.txt` – Example input files
- **outputs/**
  - `SampleOutput1.txt`, `SampleOutput2.txt` – Sample output file
- **docs/**
  - `assignment-instructions.pdf` – Assignment instructions and requirements

## ⚙️ Compilation
Use `gcc` to compile:

```bash
make
```

This will produce an executable `mlfqs.exe`.

Or manually:

```bash
gcc -o mlfqs.exe src/mlfqs.c src/prioque.c -lpthread
```

## ▶️ Running
Run with interactive input:
```bash
./mlfqs.exe
```

Or redirect from a file:
```bash
type inputs/test_input.txt | ./mlfqs.exe
```

## 📊 Example
Input (`test_input.txt`):
```
5 1000 8 20 5
200 1583 1000 10 1
...
```

Produces output similar to (`SampleOutput1.txt`):
```
PID: 1000, ARRIVAL TIME: 5
CREATE: Process 1000 entered the ready queue at time 5
RUN: Process 1000 started execution from level 1 at time 5 ...
```

## 📝 Notes
- Scheduling rules: round-robin within each queue, demotion/promotion handled by counters, preemption on higher priority arrivals.
- Includes test files to validate priority queue functions separately.

## 👥 Authors
- Original queue package: Golden G. Richard III (prioque)
- Scheduler implementation: Carter R. Mauer
- Assistance/adjustments: Prof. Ali-Gombe
