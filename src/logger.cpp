#include "logger.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

Logger::Logger(const std::string& filename) {
    log_file.open(filename, std::ios::app);
}

Logger::~Logger() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

std::string Logger::current_time() const {
    std::time_t now = std::time(nullptr);
    std::tm local_tm{};
    localtime_r(&now, &local_tm);

    std::ostringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::level_to_string(AlertLevel level) const {
    switch (level) {
        case AlertLevel::MEDIA: return "MEDIA";
        case AlertLevel::ALTA: return "ALTA";
        case AlertLevel::CRITICA: return "CRITICA";
        default: return "NONE";
    }
}

void Logger::write_line(const std::string& line, bool stderr_output) {
    if (stderr_output) {
        std::cerr << line << std::endl;
    } else {
        std::cout << line << std::endl;
    }

    if (log_file.is_open()) {
        log_file << line << std::endl;
    }
}

void Logger::info(const std::string& msg) {
    write_line("[" + current_time() + "] [INFO] " + msg);
}

void Logger::error(const std::string& msg) {
    write_line("[" + current_time() + "] [ERROR] " + msg, true);
}

void Logger::alert(const Alert& a) {
    std::ostringstream ss;
    ss << "[ALERTA " << level_to_string(a.level) << "] "
       << "PID=" << a.pid
       << " Syscall=" << a.syscall
       << " Motivo=" << a.reason;
    write_line(ss.str());
}
