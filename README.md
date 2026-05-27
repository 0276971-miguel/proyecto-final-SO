# Syscall Sentinel

Syscall Sentinel es una herramienta educativa de ciberseguridad defensiva. Ejecuta un programa objetivo en modo monitoreado, intercepta llamadas al sistema con `ptrace` y genera alertas cuando detecta acciones sospechosas.

## Flujo del programa

```text
Usuario ejecuta syscall-sentinel
        ↓
El programa crea un proceso hijo con fork()
        ↓
El hijo activa PTRACE_TRACEME y ejecuta el programa objetivo con execvp()
        ↓
El proceso padre monitorea syscalls con PTRACE_SYSCALL
        ↓
El monitor extrae número de syscall y argumentos relevantes
        ↓
Rules evalúa si la acción es sospechosa
        ↓
Logger imprime alertas y guarda logs en logs/sentinel.log
```

## Qué detecta

- Acceso a archivos sensibles: `/etc/passwd`, `/etc/group`, `/etc/hosts`.
- Acceso a archivos críticos: `/etc/shadow`, `/etc/sudoers`, `/root/.ssh/id_rsa`.
- Ejecución de binarios sospechosos: `/bin/sh`, `/bin/bash`, `/usr/bin/python3`, `curl`, `wget`, `nc`.
- Borrado de archivos mediante `unlink` o `unlinkat`.
- Cambio de permisos mediante `chmod` o `fchmodat`.
- Creación excesiva de procesos mediante `fork`, `vfork` o `clone`.

## Estructura limpia

```text
syscall-sentinel/
├── Dockerfile
├── docker-compose.yml
├── Makefile
├── README.md
├── logs/
├── src/
│   ├── main.cpp
│   ├── monitor.cpp
│   ├── monitor.h
│   ├── rules.cpp
│   ├── rules.h
│   ├── logger.cpp
│   └── logger.h
└── test_programs/
    ├── safe_program.cpp
    ├── suspicious_program.cpp
    └── fork_bomb_demo.cpp
```

## Compilar en Linux

```bash
make clean
make all
```

## Ejecutar pruebas en Linux

Programa seguro:

```bash
make run-safe
```

Programa sospechoso:

```bash
make run-suspicious
```

Prueba de creación de procesos:

```bash
make run-fork
```

## Ejecutar con Docker en Linux o macOS

`ptrace` necesita permisos especiales dentro del contenedor. Por eso se usa `SYS_PTRACE` y `seccomp=unconfined`.

Con Docker Compose:

```bash
docker compose up --build
```

Con Docker directo:

```bash
docker build -t syscall-sentinel .
docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "$(pwd)/logs:/app/logs" syscall-sentinel
```

Para entrar al contenedor:

```bash
make docker-shell
```

## Nota sobre macOS

El código monitorea syscalls de Linux con `ptrace`. En macOS no se ejecuta de forma nativa porque macOS no usa las mismas syscalls ni la misma API de `ptrace`. En Mac debe correrse con Docker Desktop, que ejecuta un entorno Linux interno.

## Ejemplo de salida esperada

```text
[ALERTA ALTA] PID=17 Syscall=openat Motivo=Acceso a archivo sensible: /etc/passwd
[ALERTA ALTA] PID=17 Syscall=chmod Motivo=Cambio de permisos en archivo: /tmp/sentinel_demo.txt
[ALERTA MEDIA] PID=17 Syscall=unlink Motivo=Intento de borrar archivo: /tmp/sentinel_demo.txt
[ALERTA ALTA] PID=17 Syscall=execve Motivo=Ejecucion de binario sospechoso: /bin/sh
```

## Limitaciones

- Es un proyecto educativo, no un EDR real.
- Monitorea principalmente el proceso objetivo inicial.
- No bloquea acciones; solo detecta y registra alertas.
- La compatibilidad real está enfocada en Linux x86_64 y Linux aarch64.
