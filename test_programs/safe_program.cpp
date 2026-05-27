#include <fstream>
#include <iostream>
#include <string>

int main() {
    std::cout << "[safe] Iniciando programa seguro\n";

    const std::string path = "/tmp/sentinel_safe_test.txt";
    std::ofstream out(path);
    out << "archivo temporal seguro\n";
    out.close();

    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        std::cout << "[safe] Leido: " << line << "\n";
    }

    std::cout << "[safe] Terminado correctamente\n";
    return 0;
}
