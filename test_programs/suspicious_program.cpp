#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    std::cout << "[susp] Iniciando programa sospechoso\n";

    std::cout << "[susp] Leyendo /etc/passwd\n";
    std::ifstream passwd("/etc/passwd");
    if (passwd.is_open()) {
        std::string line;
        std::getline(passwd, line);
        std::cout << "[susp] Primera linea: " << line.substr(0, 30) << "...\n";
    }

    const char* temp_file = "/tmp/sentinel_demo.txt";
    std::cout << "[susp] Creando archivo temporal\n";
    std::ofstream tmp(temp_file);
    tmp << "archivo de prueba\n";
    tmp.close();

    std::cout << "[susp] Cambiando permisos\n";
    chmod(temp_file, 0777);

    std::cout << "[susp] Borrando archivo\n";
    unlink(temp_file);

    std::cout << "[susp] Ejecutando /bin/sh\n";
    char* args[] = {const_cast<char*>("/bin/sh"), const_cast<char*>("-c"), const_cast<char*>("echo shell ejecutado"), nullptr};
    execve("/bin/sh", args, nullptr);

    return 0;
}
