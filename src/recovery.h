#ifndef ___RECOVERY_H___
#define ___RECOVERY_H___


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

typedef struct flow_control {
    sig_atomic_t interrupt;
    pid_t child_pid;
} flow_control;

volatile flow_control _flow_control_info;
static struct sigaction _sigterm_action;

int term_process(pid_t pid) {
    if (kill(pid, SIGTERM)) {
        fprintf(stderr, "[Error] Kill %d failed: %s\n", pid, strerror(errno));
        return 0;
    }
    return 1;
}

void term_signal(int sig) {
    _flow_control_info.interrupt = 1;
    fprintf(stdout, "[Info] Capture signal %d\n", sig);
    if (_flow_control_info.child_pid > 0) {
        term_process(_flow_control_info.child_pid);
    }
}

void set_sigaction(void (*handler) (int)) {
    _sigterm_action.sa_handler = handler;
    sigfillset(&(_sigterm_action.sa_mask));
    _sigterm_action.sa_flags = 0;
}

int register_signal(int signum) {
    if (sigaction(signum, &_sigterm_action, NULL)) {
        fprintf(stderr, "[Error] sigaction %d failed: %s\n", signum, strerror(errno));
        return 0;
    }
    return 1;
}

sig_atomic_t is_interrupt() {
    return _flow_control_info.interrupt;
}

void set_child_pid(pid_t pid) {
    _flow_control_info.child_pid = pid;
}

int check_interrupt(pid_t pid) {
    _flow_control_info.child_pid = pid;
    if (is_interrupt()) {
        term_process(pid);
        return 1;
    }
    return 0;
}

int flow_control_init() {
    _flow_control_info.interrupt = 0;
    _flow_control_info.child_pid = 0;
    set_sigaction(term_signal);
    return register_signal(SIGTERM) && register_signal(SIGINT);
}

int flow_control_destroy() {
    return 0;
}

#endif // ! ___RECOVERY_H___
