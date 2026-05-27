#ifndef RULES_H
#define RULES_H

#include <string>
#include <sys/types.h>

enum class AlertLevel {
    NONE,
    MEDIA,
    ALTA,
    CRITICA
};

struct SyscallInfo {
    pid_t pid = -1;
    long syscall_nr = -1;
    std::string syscall_name;
    std::string path;
    int fork_count = 0;
};

struct Alert {
    AlertLevel level = AlertLevel::NONE;
    pid_t pid = -1;
    std::string syscall;
    std::string reason;
};

class Rules {
public:
    static Alert check(const SyscallInfo& info);
};

#endif
