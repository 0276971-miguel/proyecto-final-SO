#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include "rules.h"

class Logger {
public:
    explicit Logger(const std::string& filename);
    ~Logger();

    void info(const std::string& msg);
    void error(const std::string& msg);
    void alert(const Alert& alert);

private:
    std::ofstream log_file;

    std::string current_time() const;
    std::string level_to_string(AlertLevel level) const;
    void write_line(const std::string& line, bool stderr_output = false);
};

#endif
