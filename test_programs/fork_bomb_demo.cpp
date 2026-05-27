#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    std::cout << "[fork-demo] Creando varios procesos controlados\n";

    for (int i = 0; i < 7; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            _exit(0);
        }
        if (pid > 0) {
            waitpid(pid, nullptr, 0);
        }
    }

    std::cout << "[fork-demo] Terminado\n";
    return 0;
}
