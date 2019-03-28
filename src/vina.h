#ifndef ___VINA_H___
#define ___VINA_H___

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <sys/types.h>

#ifndef VINA_WORK_DIR
#define VINA_WORK_DIR "/dev/shm"
#endif // !VINA_WORK_DIR
#ifndef VINA_ORIGIN_DIR
#define VINA_ORIGIN_DIR VINA_WORK_DIR "/software"
#endif // !VINA_ORIGIN_DIR

static char* _work_dir;

int make_work_dir() {
    char template_dir[] = VINA_WORK_DIR "/vina.XXXXXX";
    const char* dir = mkdtemp(template_dir);
    if (dir == NULL) {
        perror("[Error] mkdtemp failed: ");
        return 0;
    }

    _work_dir = strdup(dir);
    if (chdir(dir) == -1) {
        perror("[Error] chdir failed: ");
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
        perror("[Error] rm -rf failed: ");
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
    if (vina_version()) {
        return 1;
    }
    int status = system("cp -rp ./software/ " VINA_WORK_DIR);
    (void)status;
    return vina_version();
}

int vina_run(const char* vina_file, const char* pdbqt_file) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "[Error] fork error for %d\n", errno);
        return 1;
    } else if (pid > 0) {
        int status = -1;
        wait(&status);
        return status;
    }
    if (execl(VINA_ORIGIN_DIR "/vina", "vina", "--cpu", "1",
        "--config", VINA_ORIGIN_DIR "/target.txt",
        "--receptor", VINA_ORIGIN_DIR "/target.pdbqt",
        "--ligand", vina_file, "--out", pdbqt_file, NULL) < 0) {
        perror("error on exec");
        exit(errno);
    }
    return 1;
}

int obabel_run(const char* pdbqt_file, const char* mol_file) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "[Error] fork error for %d\n", errno);
        return 1;
    } else if (pid > 0) {
        int status = -1;
        wait(&status);
        return status;
    }
    if (execl(VINA_ORIGIN_DIR "/obabel", "obabel",
        "-ipdbqt", pdbqt_file, "-omol", "-O", mol_file, NULL) < 0) {
        perror("error on exec");
        exit(errno);
    }
    return 1;
}

#endif // ! ___VINA_H___
