#include "monitor.h"

#include "logger.h"
#include "rules.h"

#include <cerrno>
#include <cstring>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#if defined(__aarch64__)
#include <asm/ptrace.h>
#include <elf.h>
#endif

Monitor::Monitor(pid_t pid, Logger& log)
    : target_pid(pid), logger(log), fork_count(0), entering_syscall(true) {}

std::string Monitor::read_string(pid_t pid, unsigned long addr) const {
    std::string result;
    if (addr == 0) {
        return result;
    }

    for (size_t offset = 0; offset < 4096; offset += sizeof(long)) {
        errno = 0;
        long word = ptrace(PTRACE_PEEKDATA, pid, reinterpret_cast<void*>(addr + offset), nullptr);
        if (word == -1 && errno != 0) {
            break;
        }

        const char* bytes = reinterpret_cast<const char*>(&word);
        for (size_t i = 0; i < sizeof(long); ++i) {
            if (bytes[i] == '\0') {
                return result;
            }
            result.push_back(bytes[i]);
        }
    }

    return result;
}

void Monitor::handle_syscall(pid_t pid, long syscall_nr, unsigned long arg0, unsigned long arg1) {
    SyscallInfo info;
    info.pid = pid;
    info.syscall_nr = syscall_nr;
    info.fork_count = fork_count;

    switch (syscall_nr) {
#ifdef SYS_open
        case SYS_open:
            info.syscall_name = "open";
            info.path = read_string(pid, arg0);
            break;
#endif
        case SYS_openat:
            info.syscall_name = "openat";
            info.path = read_string(pid, arg1);
            break;
        case SYS_execve:
            info.syscall_name = "execve";
            info.path = read_string(pid, arg0);
            break;
#ifdef SYS_unlink
        case SYS_unlink:
            info.syscall_name = "unlink";
            info.path = read_string(pid, arg0);
            break;
#endif
#ifdef SYS_unlinkat
        case SYS_unlinkat:
            info.syscall_name = "unlinkat";
            info.path = read_string(pid, arg1);
            break;
#endif
#ifdef SYS_chmod
        case SYS_chmod:
            info.syscall_name = "chmod";
            info.path = read_string(pid, arg0);
            break;
#endif
#ifdef SYS_fchmodat
        case SYS_fchmodat:
            info.syscall_name = "fchmodat";
            info.path = read_string(pid, arg1);
            break;
#endif
#ifdef SYS_fork
        case SYS_fork:
            info.syscall_name = "fork";
            ++fork_count;
            info.fork_count = fork_count;
            break;
#endif
#ifdef SYS_vfork
        case SYS_vfork:
            info.syscall_name = "vfork";
            ++fork_count;
            info.fork_count = fork_count;
            break;
#endif
#ifdef SYS_clone
        case SYS_clone:
            info.syscall_name = "clone";
            ++fork_count;
            info.fork_count = fork_count;
            break;
#endif
        default:
            return;
    }

    Alert alert = Rules::check(info);
    if (alert.level != AlertLevel::NONE) {
        logger.alert(alert);
    }
}

void Monitor::analyze_syscall(pid_t pid) {
#if defined(__x86_64__)
    user_regs_struct regs{};
    if (ptrace(PTRACE_GETREGS, pid, nullptr, &regs) == -1) {
        logger.error(std::string("PTRACE_GETREGS fallo: ") + std::strerror(errno));
        return;
    }
    handle_syscall(pid, static_cast<long>(regs.orig_rax), regs.rdi, regs.rsi);
#elif defined(__aarch64__)
    user_pt_regs regs{};
    iovec iov{&regs, sizeof(regs)};
    if (ptrace(PTRACE_GETREGSET, pid, reinterpret_cast<void*>(NT_PRSTATUS), &iov) == -1) {
        logger.error(std::string("PTRACE_GETREGSET fallo: ") + std::strerror(errno));
        return;
    }
    handle_syscall(pid, static_cast<long>(regs.regs[8]), regs.regs[0], regs.regs[1]);
#else
#error "Arquitectura no soportada. Usa Linux x86_64 o Linux aarch64."
#endif
}

int Monitor::run() {
    int status = 0;
    if (waitpid(target_pid, &status, 0) == -1) {
        logger.error(std::string("waitpid inicial fallo: ") + std::strerror(errno));
        return 1;
    }

    if (!WIFSTOPPED(status)) {
        logger.error("El proceso hijo no entro en modo trazado.");
        return 1;
    }

    long options = PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL;
    if (ptrace(PTRACE_SETOPTIONS, target_pid, nullptr, reinterpret_cast<void*>(options)) == -1) {
        logger.error(std::string("PTRACE_SETOPTIONS fallo: ") + std::strerror(errno));
        return 1;
    }

    while (true) {
        if (ptrace(PTRACE_SYSCALL, target_pid, nullptr, nullptr) == -1) {
            if (errno != ESRCH) {
                logger.error(std::string("PTRACE_SYSCALL fallo: ") + std::strerror(errno));
            }
            return 1;
        }

        if (waitpid(target_pid, &status, 0) == -1) {
            logger.error(std::string("waitpid fallo: ") + std::strerror(errno));
            return 1;
        }

        if (WIFEXITED(status)) {
            logger.info("Programa objetivo finalizo con codigo " + std::to_string(WEXITSTATUS(status)));
            return WEXITSTATUS(status);
        }

        if (WIFSIGNALED(status)) {
            logger.info("Programa objetivo termino por senal " + std::to_string(WTERMSIG(status)));
            return 128 + WTERMSIG(status);
        }

        if (WIFSTOPPED(status)) {
            int signal = WSTOPSIG(status);
            if (signal == (SIGTRAP | 0x80)) {
                if (entering_syscall) {
                    analyze_syscall(target_pid);
                }
                entering_syscall = !entering_syscall;
            }
        }
    }
}
