/* (c) Simon Toth 2016
 * Licensed under the MIT license.
 *
 * Just-in-time debugging support for Linux
 * ----------------------------------------
 *
 * LD_PRELOAD library that will launch a new terminal
 * with a debugger attached when a program crashes
 * with the specified signals.
 *
 * Based on:
https://stackoverflow.com/questions/22509088/is-it-possible-to-attach-gdb-to-a-crashed-process-a-k-a-just-in-time-debuggin
*/


#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/prctl.h>

void gdb(int sig);

const char *signals[] = {
  "JITGDB_SIGSEGV",
  "JITGDB_SIGABRT",
  "JITGDB_SIGHUP",
  "JITGDB_SIGINT",
  "JITGDB_SIGQUIT",
  "JITGDB_SIGILL",
  "JITGDB_SIGFPE",
  "JITGDB_SIGPIPE",
  "JITGDB_SIGALRM",
  "JITGDB_SIGTERM",
  "JITGDB_SIGUSR1",
  "JITGDB_SIGUSR2",
  NULL
};

int signals_opt[12][2] = {
	{ 1, SIGSEGV },
	{ 1, SIGABRT },
	{ 0, SIGHUP },
	{ 0, SIGINT },
	{ 0, SIGQUIT },
	{ 0, SIGILL },
	{ 0, SIGFPE },
	{ 0, SIGPIPE },
	{ 0, SIGALRM },
	{ 0, SIGTERM },
	{ 0, SIGUSR1 },
	{ 0, SIGUSR2 }
};

void _init() {

  // parse environment configuration
  size_t id = 0;
  while (signals[id] != NULL) {
    // use the signal name part of the env variables
    char *sig = getenv(signals[id]);
    if (sig == NULL) {
      ++id;
      continue;
    }

    if (toupper(sig[0]) == 'T') {
      signals_opt[id][0] = 1;
      fprintf(stderr,"Setting %s to watched.\n",signals[id]);
    } else if (toupper(sig[0]) == 'F') {
      signals_opt[id][0] = 0;
      fprintf(stderr,"Setting %s to unwatched.\n",signals[id]);
    } else {
      fprintf(stderr,"Unrecognized option for environment variable %s=%s. "
                     "Only accepts True/False.\n",signals[id],sig);
    }

  ++id;
  }

  // setup signal handlers
  do {
    --id;
    if (signals_opt[id][0] == 1) {
    signal(signals_opt[id][1],gdb);
    }
  } while (id > 0);

}

void gdb(int sig) {

  size_t id = 0;
  while (id <= 12 && signals_opt[id][1] != sig) {
    if (signals_opt[id][0] == 1)
      break;
    ++id;
  }

  // this isn't a signal we react to
  if (id > 12)
    return;


  pid_t cpid = fork();
  // if we can't fork, just bail out, nothing we can do
  if (cpid == -1)
    return;

  if (cpid != 0) {
    // parent
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);  // allow any process to ptrace us
    raise(SIGSTOP);  // wait for child's gdb invocation to pick us up
    return;
  }

  // we are in the child now

  // Child - now try to exec gdb in our place attached to the parent
  // Avoiding using libc since that may already have been stomped, so building the
  // gdb args the hard way ("gdb dummy PID"), first copy
  char cmd[100];
  const char* stem = "gdb _dummy_process_name_                   ";  // 18 trailing spaces to allow for a 64 bit proc id
  const char*s = stem;
  char* d = cmd;

  while(*s) {
    *d++ = *s++;
  }
  *d-- = '\0';

  char* hexppid = d;

  // now backfill the trailing space with the hex parent PID - not
  // using decimal for fear of libc maths helper functions being dragged in
  pid_t ppid = getppid();
  while(ppid) {
    *hexppid = ((ppid & 0xF) + '0');

    if (*hexppid > '9')
       *hexppid += 'a' - '0' - 10;

    --hexppid;
    ppid >>= 4;
  }

  *hexppid-- = 'x';   // prefix with 0x
  *hexppid = '0';

  // system() isn't listed as safe under async signals, nor is execlp, 
  // or getenv. So ideally we'd already have cached the gdb location, or we
  // hardcode the gdb path, or we accept the risk of re-entrancy/library woes
  // around the environment fetch...
  execlp("xterm", "xterm", "-e", cmd, (char*) NULL);
}

