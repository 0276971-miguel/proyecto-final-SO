#include "logger.h"
#include "monitor.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
void print_usage(const char* program_name) {
    std::cerr << "Uso:\n"
              << "  " << program_name << " <programa_objetivo> [argumentos...]\n\n"
              << "Ejemplos:\n"
              << "  " << program_name << " ./test_programs/safe_program\n"
              << "  " << program_name << " ./test_programs/suspicious_program\n";
}
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    Logger logger("logs/sentinel.log");
    logger.info("=== Iniciando Syscall Sentinel ===");
    logger.info(std::string("Programa objetivo: ") + argv[1]);

    pid_t pid = fork();
    if (pid == -1) {
        logger.error(std::string("fork fallo: ") + std::strerror(errno));
        return 1;
    }

    if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
            std::cerr << "ptrace fallo: " << std::strerror(errno) << "\n";
            return 1;
        }

        raise(SIGSTOP);
        execvp(argv[1], &argv[1]);
        std::cerr << "execvp fallo: " << std::strerror(errno) << "\n";
        return 1;
    }

    Monitor monitor(pid, logger);
    int exit_code = monitor.run();
    logger.info("=== Syscall Sentinel finalizado ===");
    return exit_code;
}
