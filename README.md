<div align="center">

```
███╗   ██╗███████╗██████╗ ██╗   ██╗██╗      █████╗      ██████╗ ███████╗
████╗  ██║██╔════╝██╔══██╗██║   ██║██║     ██╔══██╗    ██╔═══██╗██╔════╝
██╔██╗ ██║█████╗  ██████╔╝██║   ██║██║     ███████║    ██║   ██║███████╗
██║╚██╗██║██╔══╝  ██╔══██╗██║   ██║██║     ██╔══██║    ██║   ██║╚════██║
██║ ╚████║███████╗██████╔╝╚██████╔╝███████╗██║  ██║    ╚██████╔╝███████║
╚═╝  ╚═══╝╚══════╝╚═════╝  ╚═════╝ ╚══════╝╚═╝  ╚═╝     ╚═════╝ ╚══════╝
                              V 3 . 0
```

**A Unix-Inspired, Terminal-Based Simulated Operating System**

*Built entirely in C/C++ — demonstrating real OS concepts through a working simulation*

---

**Salman Shoaib** (24P-0583) · **Abdullah Nadeem** (24P-0563) · **Shoaib Altaf** (24F-0737)

*Operating Systems Lab — Spring 2026*

</div>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [OS Concepts Demonstrated](#os-concepts-demonstrated)
- [Application Modules](#application-modules)
- [Kernel Architecture](#kernel-architecture)
- [IPC & Communication](#ipc--communication)
- [Scheduling Algorithm](#scheduling-algorithm)
- [How to Build & Run](#how-to-build--run)
- [Usage Guide](#usage-guide)
- [Team](#team)

---

## Overview

**Nebula OS V3** is a fully functional simulated operating system that runs as a process on **Arch Linux**. It manages its own child processes, enforces scheduling, allocates resources, and provides users with 23 interactive application modules — all through a terminal interface.

The project was built to demonstrate and implement real operating system concepts including:
- Process lifecycle management (fork/exec/wait)
- Multi-level queue scheduling with aging
- Resource allocation and memory management
- Inter-process communication via named pipes (FIFOs)
- Deadlock detection using Resource Allocation Graphs
- Signal handling and interrupt simulation

> Nebula OS is not an emulator — it *is* an OS kernel loop managing real Linux processes. Every "app" you launch is a real child process, allocated real RAM/HDD budgets, and managed through a real PCB table.

---

## Features

| Feature | Description |
|---|---|
| 🖥️ **23 Application Modules** | From calculator to music player to a space guessing game |
| 📋 **Process Control Block (PCB)** | Full PCB table with PID, state, priority, resource allocation |
| ⚙️ **Multi-Level Queue Scheduler** | Q0 (System), Q1 (Interactive), Q2 (Background) with Round Robin |
| 🧠 **Memory Manager** | RAM and HDD quota enforcement per process |
| 🔗 **IPC via Named Pipes** | Processes request resources and receive grants through FIFOs |
| 🔒 **Deadlock Detector** | Resource Allocation Graph + DFS cycle detection, runs every 5s |
| 📡 **Signal Handling** | SIGCHLD, SIGUSR1, SIGTSTP, SIGCONT for process control |
| 📜 **System Logger** | Timestamped event log with rotation via Log Daemon |
| 🔄 **Process Aging** | Prevents starvation — background tasks promoted over time |
| 🗂️ **Virtual HDD** | Simulated filesystem at `nebula_hdd/` for all apps to use |
| 🎵 **Music Player** | Real `.mp3` playback via `mpg123` |
| 👻 **Background Daemons** | Clock Daemon, Log Daemon, System Monitor run at startup |

---

## Project Structure

```
Nebula_V3/
│
├── Makefile                        # Build system for all binaries
│
├── OS                              # Main OS binary (compiled)
│
├── include/
│   └── kernel/
│       ├── kernel.h                # Core types: Process, KernelQueue, globals
│       ├── deadlock_detector.h
│       ├── interrupt_handler.h
│       ├── ipc_manager.h           # Pipe-based IPC + resource request protocol
│       ├── logger.h
│       ├── memory_manager.h
│       ├── process_manager.h
│       ├── scheduler.h
│       └── storage_manager.h
│
├── src/
│   ├── os.cpp                      # Main OS loop, menu, task launcher
│   │
│   └── kernel/
│       ├── deadlock_detector.cpp   # RAG + DFS deadlock detection
│       ├── interrupt_handler.cpp   # Signal registration and handlers
│       ├── ipc_manager.cpp         # Named pipe creation, IPC listener
│       ├── logger.cpp              # Thread-safe event logger
│       ├── memory_manager.cpp      # RAM/HDD/Core allocation & reclaim
│       ├── process_manager.cpp     # PCB CRUD, queue assignment
│       ├── scheduler.cpp           # MLQ scheduler + aging loop
│       └── storage_manager.cpp     # nebula_hdd/ filesystem abstraction
│
├── tasks/                          # Compiled task binaries (generated by make)
│
├── src/tasks/
│   ├── alarm.cpp
│   ├── calculator.cpp
│   ├── calendar.cpp
│   ├── clock.cpp
│   ├── copy_file.cpp
│   ├── create_file.cpp
│   ├── delete_file.cpp
│   ├── dice_roller.cpp
│   ├── file_compression.cpp
│   ├── file_info.cpp
│   ├── instruction_guide.cpp
│   ├── log_daemon.cpp
│   ├── log_viewer.cpp
│   ├── mini_terminal.cpp
│   ├── minigame.cpp
│   ├── move_file.cpp
│   ├── music_player.cpp
│   ├── notepad.cpp
│   ├── rename_file.cpp
│   ├── resource_dashboard.cpp
│   ├── stopwatch.cpp
│   ├── sys_monitor.cpp
│   ├── task_manager.cpp
│   ├── text_search.cpp
│   └── todo_list.cpp
│
├── nebula_hdd/                     # Virtual HDD (auto-created on first run)
├── music/                          # Place .mp3 files here for Music Player
└── system.log                      # Runtime system event log
```

---

## OS Concepts Demonstrated

### 1. Process Management
Every application is launched via `fork()` + `execl()`. Each process gets its own **Process Control Block (PCB)** stored in a shared table, containing:
- PID, name, state (`READY`, `RUNNING`, `MINIMIZED`, `BLOCKED`, `TERMINATED`)
- Priority level (0–10)
- Queue assignment (Q0/Q1/Q2)
- RAM and HDD allocation
- Arrival time, wait time, turnaround

### 2. Multi-Level Queue (MLQ) Scheduling
Three queues with different policies:

| Queue | Type | Processes |
|---|---|---|
| **Q0** | Priority-based FCFS | System daemons (Clock, Log Daemon) |
| **Q1** | Round Robin (200ms quantum) | Interactive apps (Calculator, Notepad, etc.) |
| **Q2** | FCFS | Background tasks (File ops, compression) |

**Aging**: Every 5 seconds of waiting, a process's priority decreases. Once it crosses a threshold, it is promoted to a higher-priority queue — preventing indefinite starvation.

### 3. Memory Management
Before any task runs, it sends a resource request via IPC:
```
REQ:<task_name>:<RAM_MB>:<HDD_MB>
```
The kernel checks availability and responds with `GRANT` or `DENY`. On process termination, all resources are automatically reclaimed.

### 4. Inter-Process Communication (IPC)
Named pipes (FIFOs) at `/tmp/nebula_pipe_<PID>` serve as the communication channel between each child process and the kernel. The IPC manager spawns a dedicated thread per process to handle requests.

### 5. Deadlock Detection
A background thread runs every 5 seconds, building a **Resource Allocation Graph (RAG)** from the current PCB table and running **DFS** to detect cycles. Detected deadlocks are logged to `system.log`.

### 6. Signal Handling
| Signal | Purpose |
|---|---|
| `SIGCHLD` | Detect child termination, clean up PCB |
| `SIGTSTP` (Ctrl+Z) | Minimize a task to background |
| `SIGCONT` | Resume a minimized task |
| `SIGUSR1` | Simulate interrupt — blocks a process temporarily |
| `SIGTERM` | Graceful shutdown of daemons |

### 7. Synchronization
- `pthread_mutex_t` protects the PCB table, resource counters, and log file
- `pthread_cond_t` used for core availability and queue signaling
- Autosave thread in Notepad uses mutex-guarded buffer access

---

## Application Modules

| # | Application | Description |
|---|---|---|
| 1 | **Calculator** | Arithmetic operations: `+`, `-`, `*`, `/`, `%` |
| 2 | **Notepad** | Multi-line editor with autosave and `:w`, `:q` commands |
| 3 | **Clock** | Live real-time clock display |
| 4 | **Calendar** | Navigate monthly calendar, prev/next month |
| 5 | **Create File** | Create files in `nebula_hdd/` |
| 6 | **Delete File** | List and delete files with confirmation |
| 7 | **Move File** | Move files to subdirectories within `nebula_hdd/` |
| 8 | **Copy File** | Duplicate files with `_copy` suffix |
| 9 | **File Info** | View file size and last modified time |
| 10 | **Task Manager** | Live system event log, kill processes, change priority |
| 11 | **Music Player** | Browse and play `.mp3` files via `mpg123` |
| 12 | **Minigame** | Space Guesser — number guessing game (1–100, 7 attempts) |
| 13 | **Instruction Guide** | Built-in OS usage reference |
| 14 | **Dice Roller** | Roll 1–6 dice with ASCII art display |
| 15 | **Rename File** | Rename files in `nebula_hdd/` |
| 16 | **Text Search** | Keyword search across all `.txt` files |
| 17 | **Alarm** | Set timed alarms; supports foreground and background mode |
| 18 | **Stopwatch** | Stopwatch with lap tracking OR countdown timer |
| 19 | **File Compression** | Simulate file compression with progress bar |
| 20 | **Log Viewer** | Paginated system log viewer with keyword filtering |
| 21 | **Mini Terminal** | Subset shell: `ls`, `pwd`, `cat`, `mkdir`, `echo >` |
| 22 | **To-Do List** | Persistent task list (add, done, remove) saved to `nebula_hdd/todo.txt` |
| 23 | **Resource Dashboard** | Live RAM/HDD usage bars, active process count |

---

## Kernel Architecture

```
┌─────────────────────────────────────────────────────┐
│                    NEBULA OS KERNEL                  │
│                                                     │
│  ┌──────────┐  ┌──────────┐  ┌───────────────────┐ │
│  │ Scheduler│  │  Memory  │  │  Process Manager  │ │
│  │ (MLQ+    │  │  Manager │  │  (PCB Table,      │ │
│  │  Aging)  │  │ (RAM/HDD)│  │   State Machine)  │ │
│  └────┬─────┘  └────┬─────┘  └────────┬──────────┘ │
│       │              │                 │             │
│  ┌────▼──────────────▼─────────────────▼──────────┐ │
│  │              IPC Manager                        │ │
│  │      (Named Pipes / FIFO per Process)           │ │
│  └────────────────────┬────────────────────────────┘ │
│                       │                              │
│  ┌────────────────────▼────────────────────────────┐ │
│  │  Interrupt Handler │ Logger │ Deadlock Detector  │ │
│  └─────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
          │               │               │
    ┌─────▼──┐      ┌─────▼──┐     ┌──────▼──┐
    │ Task 1 │      │ Task 2 │     │ Task N  │
    │(child  │      │(child  │     │(child   │
    │process)│      │process)│     │process) │
    └────────┘      └────────┘     └─────────┘
```

---

## IPC & Communication

Each task communicates with the OS kernel through a named pipe:

```
Task startup:
  1. Task calls send_resource_request("Calculator", 30, 2)
  2. Writes "REQ:Calculator:30:2" → /tmp/nebula_pipe_<PID>
  3. Kernel IPC thread reads request, calls allocate_resources()
  4. Kernel writes "GRANT:<PID>" or "DENY" back to pipe
  5. Task reads response via wait_for_grant()
  6. If GRANT → task runs; if DENY → task exits

Task termination:
  1. Task calls send_termination_notice(getpid())
  2. Writes "TERM:<PID>" to pipe
  3. Kernel reclaims RAM/HDD, removes PCB, deletes pipe
```

---

## Scheduling Algorithm

```
Every scheduler tick:
  1. Check Q0 (System) → dispatch highest-priority READY process
  2. If Q0 empty → Check Q1 (Interactive) → Round Robin among READY
  3. If Q1 empty → Check Q2 (Background) → FCFS among READY

Every second (Aging loop):
  - Increment wait_time for all non-running processes
  - Every AGING_INTERVAL (5s): decrement priority
  - If Q2 process priority ≤ 1 → promote to Q1
  - If Q1 process priority = 0 → promote to Q0
```

---

## How to Build & Run

### Prerequisites

Make sure you have the following installed on **Arch Linux** (or any Linux distro):

```bash
# Required
sudo pacman -S gcc g++ make

# Optional — only needed for Music Player
sudo pacman -S mpg123
```

### Build

```bash
make
```

This compiles the kernel, the main OS binary, and all 25 task binaries into the `tasks/` directory.

### Run

```bash
./OS
```

That's it. The OS will prompt you for hardware configuration on first boot:

```
NEBULA OS CONFIGURATION
RAM (GB): 1
HDD (GB): 1
Cores:    1
```

After configuration, the boot sequence runs, daemons start, and the main menu appears.

### Clean Build

```bash
make clean
make
```

---

## Usage Guide

### Main Menu Navigation

```
NEBULA OS V3 | RAM: 20/1024 MB | Tasks: 3 (3 active)
------------------------------------------------------------
[ 1] Calculator        [13] Instruction Guide
[ 2] Notepad           [14] Dice Roller
...
------------------------------------------------------------
[M] Tasks  [F] Foreground  [K] Kernel  [L] Logs  [0] Shutdown
NebulaOS>
```

| Key | Action |
|---|---|
| `1`–`23` | Launch application by number |
| `M` | Show background tasks |
| `F` | Bring a minimized task back to foreground (enter PID) |
| `K` | View kernel process table and resource monitor |
| `L` | View system log (with option to clear) |
| `0` | Gracefully shut down Nebula OS |

### Inside Applications

| Key | Action |
|---|---|
| `Ctrl+Z` | Minimize current app to background |
| `q` / `Q` | Quit the current application |
| `Enter` | Confirm / proceed |

### Music Player Setup

Place `.mp3` files in a `music/` folder next to the `OS` binary:

```bash
mkdir music
cp your_songs/*.mp3 music/
./OS
# → Select [11] Music Player
```

---

## Team

| Name | Student ID | Role |
|---|---|---|
| Salman Shoaib | 24P-0583 | Kernel Core, Scheduler, IPC |
| Abdullah Nadeem | 24P-0563 | Process Manager, Deadlock Detector |
| Shoaib Altaf | 24F-0737 | Task Modules, Storage Manager |

---

<div align="center">

*Nebula OS V3 — Operating Systems Lab Project*
*Built with C/C++ · Runs on Linux · No dependencies except POSIX*

</div>
