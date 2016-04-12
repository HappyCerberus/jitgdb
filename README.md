# Just in time debugging support library for GDB

When preloaded, GDB will be auto-attached to the running process when a specific signal is raised.

## Compilation

```
cmake .
make
```

## Usage

Before running your program, preload the compiled library.

```
export LD_PRELOAD=/path/to/lib/libjitgdb.so
```

By default the debugger will be spawned for `SIGSEGV` and `SIGABRT`, for other signals, the environment variables `JITGDB_SIGNAL` have to be set to `True`.

Full list of supported signals:

```
JITGDB_SIGSEGV
JITGDB_SIGABRT
JITGDB_SIGHUP
JITGDB_SIGINT
JITGDB_SIGQUIT
JITGDB_SIGILL
JITGDB_SIGFPE
JITGDB_SIGPIPE
JITGDB_SIGALRM
JITGDB_SIGTERM
JITGDB_SIGUSR1
JITGDB_SIGUSR2
```
