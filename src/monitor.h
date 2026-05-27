#ifndef MONITOR_H
#define MONITOR_H

#include <string>
#include <sys/types.h>

class Logger;

class Monitor {
public:
    Monitor(pid_t pid, Logger& logger);
    int run();

private:
    pid_t target_pid;
    Logger& logger;
    int fork_count;
    bool entering_syscall;

    std::string read_string(pid_t pid, unsigned long addr) const;
    void analyze_syscall(pid_t pid);
    void handle_syscall(pid_t pid, long syscall_nr, unsigned long arg0, unsigned long arg1);
};

#endif
