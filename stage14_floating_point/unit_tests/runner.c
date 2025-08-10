//
// Created by scott on 6/13/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define TEST_DIR "./unit_tests/build"
#define PREFIX "test_"

int ends_with(const char *str, const char *suffix) {
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    return len_str >= len_suffix &&
           strcmp(str + len_str - len_suffix, suffix) == 0;
}

int main(void) {
    DIR *dir = opendir(TEST_DIR);
    if (!dir) {
        perror("Could not open test directory");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG && entry->d_type != DT_LNK)
            continue;

        if (strncmp(entry->d_name, PREFIX, strlen(PREFIX)) != 0)
            continue;

        // Build full path
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", TEST_DIR, entry->d_name);

        printf("Running %s...\n", full_path);
        fflush(stdout);

        pid_t pid = fork();
        if (pid == 0) {
            // Child: run test
            execl(full_path, full_path, NULL);
            perror("Failed to exec test");
            exit(127);  // Common exec failure code
        } else if (pid > 0) {
            // Parent: wait
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                int code = WEXITSTATUS(status);
                if (code != 0) {
                    printf("❌ %s failed with exit code %d\n", entry->d_name, code);
                    closedir(dir);
                    return code;
                } else {
                    printf("✅ %s passed\n", entry->d_name);
                }
            } else {
                printf("❌ %s exited abnormally\n", entry->d_name);
                closedir(dir);
                return 1;
            }
        } else {
            perror("fork failed");
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    printf("🎉 All tests passed\n");
    return 0;
}
