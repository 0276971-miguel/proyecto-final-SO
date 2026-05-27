#include "rules.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {
const std::vector<std::string> CRITICAL_FILES = {
    "/etc/shadow",
    "/etc/sudoers",
    "/root/.ssh/id_rsa"
};

const std::vector<std::string> SENSITIVE_FILES = {
    "/etc/passwd",
    "/etc/group",
    "/etc/hosts"
};

const std::vector<std::string> DANGEROUS_BINARIES = {
    "/bin/bash",
    "/bin/sh",
    "/bin/dash",
    "/usr/bin/python3",
    "/usr/bin/perl",
    "/usr/bin/nc",
    "/usr/bin/curl",
    "/usr/bin/wget"
};

constexpr int FORK_THRESHOLD = 5;

bool path_matches(const std::string& path, const std::vector<std::string>& list) {
    return std::find(list.begin(), list.end(), path) != list.end();
}

Alert make_alert(AlertLevel level, const SyscallInfo& info, const std::string& reason) {
    Alert alert;
    alert.level = level;
    alert.pid = info.pid;
    alert.syscall = info.syscall_name;
    alert.reason = reason;
    return alert;
}
}

Alert Rules::check(const SyscallInfo& info) {
    if ((info.syscall_name == "open" || info.syscall_name == "openat") && !info.path.empty()) {
        if (path_matches(info.path, CRITICAL_FILES)) {
            return make_alert(AlertLevel::CRITICA, info, "Acceso a archivo critico: " + info.path);
        }
        if (path_matches(info.path, SENSITIVE_FILES)) {
            return make_alert(AlertLevel::ALTA, info, "Acceso a archivo sensible: " + info.path);
        }
    }

    if (info.syscall_name == "execve" && path_matches(info.path, DANGEROUS_BINARIES)) {
        return make_alert(AlertLevel::ALTA, info, "Ejecucion de binario sospechoso: " + info.path);
    }

    if ((info.syscall_name == "unlink" || info.syscall_name == "unlinkat") && !info.path.empty()) {
        return make_alert(AlertLevel::MEDIA, info, "Intento de borrar archivo: " + info.path);
    }

    if ((info.syscall_name == "chmod" || info.syscall_name == "fchmodat") && !info.path.empty()) {
        return make_alert(AlertLevel::ALTA, info, "Cambio de permisos en archivo: " + info.path);
    }

    if ((info.syscall_name == "fork" || info.syscall_name == "vfork" || info.syscall_name == "clone") &&
        info.fork_count > FORK_THRESHOLD) {
        return make_alert(AlertLevel::ALTA, info, "Creacion excesiva de procesos: " + std::to_string(info.fork_count));
    }

    return Alert{};
}
