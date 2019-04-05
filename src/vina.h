#ifndef ___VINA_H___
#define ___VINA_H___

#include "recovery.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef VINA_WORK_DIR
#define VINA_WORK_DIR "/dev/shm"
#endif // !VINA_WORK_DIR
#ifndef VINA_ORIGIN_DIR
#define VINA_ORIGIN_DIR VINA_WORK_DIR "/software"
#endif // !VINA_ORIGIN_DIR

static char* _work_dir = NULL;

int make_work_dir() {
    char template_dir[] = VINA_WORK_DIR "/vina.XXXXXX";
    const char* dir = mkdtemp(template_dir);
    if (dir == NULL) {
        fprintf(stderr, "[Error] mkdtemp failed: %s\n", strerror(errno));
        return 0;
    }

    _work_dir = strdup(dir);
    if (chdir(dir) == -1) {
        fprintf(stderr, "[Error] chdir failed: %s\n", strerror(errno));
        return 0;
    }
    fprintf(stdout, "[Info] PWD: %s\n", _work_dir);
    return 1;
}

int destory_work_dir() {
    if (!_work_dir) {
        return 1;
    }
    char command[512];
    snprintf(command, sizeof(command), "rm -rf %s", _work_dir);
    fprintf(stdout, "[Info] Running %s\n", command);
    if (system(command)) {
        fprintf(stderr, "[Error] rm -rf failed: %s\n", strerror(errno));
        return 0;
    }
    free(_work_dir);
    return 1;
}

int vina_version() {
    return system(VINA_ORIGIN_DIR "/vina --version") == 0
        && system(VINA_ORIGIN_DIR "/obabel -V") == 0;
}

int vina_init() {
#ifndef NO_COPY_VINA
    if (vina_version()) {
        return 1;
    }
    if (system("cp -rp ./software/ " VINA_WORK_DIR)) {
        sleep(1);
    }
#endif // !NO_COPY_VINA
    return vina_version();
}

int redirect_null() {
    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null == -1) {
        fprintf(stderr, "[Error] open '/dev/null' failed\n");
        return 0;
    }

    if (dup2(dev_null, STDOUT_FILENO) == -1) {
        fprintf(stderr, "[Error] dup2 /dev/null to STDOUT_FILENO failed\n");
        return 0;
    }
    return 1;
}

int get_child_process_status(const char* process) {
    int status;
    do {
        if (wait(&status) != -1) {
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status)) {
                    fprintf(stderr, "[Error] %s return %d\n",
                        process, WEXITSTATUS(status));
                }
                return 0;
            }
#ifdef DEBUG_OUTPUT
            fprintf(stdout, "[Info] %s return %d %d %d %d %d %d %d %d\n",
                process, status, WIFSIGNALED(status), WTERMSIG(status),
                WIFEXITED(status), WEXITSTATUS(status),
                WCOREDUMP(status), WIFSTOPPED(status), WIFCONTINUED(status));
#endif // !DEBUG_OUTPUT
            return 1;
        }
    } while (errno == EINTR);

    fprintf(stderr, "[Error] wait %s failed: %s\n", process, strerror(errno));
    return -1;
}

int vina_run(const char* vina_file, const char* pdbqt_file) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "[Error] fork error for %d\n", errno);
        return 1;
    } else if (pid > 0) {
        check_interrupt(pid);
        int status = get_child_process_status("vina");
        set_child_pid(0);
        return status;
    }
    redirect_null();
    if (execl(VINA_ORIGIN_DIR "/vina", "vina", "--cpu", "1",
        "--config", VINA_ORIGIN_DIR "/target.txt",
        "--receptor", VINA_ORIGIN_DIR "/target.pdbqt",
        "--ligand", vina_file, "--out", pdbqt_file, NULL) < 0) {
        fprintf(stderr, "[Error] exec failed: %s\n", strerror(errno));
        exit(errno);
    }
    return 1;
}

double vina_score(const char* pdbqt_file) {
    const size_t SCORE_ROW = 2;
    double score = DBL_MAX;
    char* line = file_line(pdbqt_file, SCORE_ROW);
    if (!line || sscanf(line, "%*s%*s%*s%lf", &score) != 1) {
        fprintf(stderr, "[Error] get score failed, set score: %lf\n", score);
    }
    return score;
}

int obabel_run(const char* pdbqt_file, const char* mol_file) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "[Error] fork error for %d\n", errno);
        return 1;
    } else if (pid > 0) {
        check_interrupt(pid);
        int status = get_child_process_status("obabel");
        set_child_pid(0);
        return status;
    }
    if (execl(VINA_ORIGIN_DIR "/obabel", "obabel",
        "-ipdbqt", pdbqt_file, "-omol", "-O", mol_file, NULL) < 0) {
        fprintf(stderr, "[Error] exec failed: %s\n", strerror(errno));
        exit(errno);
    }
    return 1;
}

#endif // ! ___VINA_H___
