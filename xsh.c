#include "xsh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ENV_VARS 128
#define MAX_VAR_LEN 64

static char env_vars[MAX_ENV_VARS][2][MAX_VAR_LEN];
static int env_count = 0;

void set_env(const char *key, const char *value) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i][0], key) == 0) {
            strncpy(env_vars[i][1], value, MAX_VAR_LEN);
            return;
        }
    }
    if (env_count < MAX_ENV_VARS) {
        strncpy(env_vars[env_count][0], key, MAX_VAR_LEN);
        strncpy(env_vars[env_count][1], value, MAX_VAR_LEN);
        env_count++;
    }
}

void unset_env(const char *key) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i][0], key) == 0) {
            for (int j = i; j < env_count - 1; j++) {
                strcpy(env_vars[j][0], env_vars[j + 1][0]);
                strcpy(env_vars[j][1], env_vars[j + 1][1]);
            }
            env_count--;
            return;
        }
    }
}

const char *get_env(const char *key) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i][0], key) == 0) {
            return env_vars[i][1];
        }
    }
    return "";
}

char *replace_env_vars(const char *command) {
    static char buffer[MAX_CMD_LEN];
    strncpy(buffer, command, MAX_CMD_LEN);

    for (int i = 0; i < env_count; i++) {
        char var[MAX_VAR_LEN + 2];
        snprintf(var, sizeof(var), "$%s", env_vars[i][0]);

        char *pos = strstr(buffer, var);
        if (pos) {
            char temp[MAX_CMD_LEN];
            strncpy(temp, buffer, pos - buffer);
            temp[pos - buffer] = '\0';
            strncat(temp, env_vars[i][1], MAX_CMD_LEN - strlen(temp));
            strncat(temp, pos + strlen(var), MAX_CMD_LEN - strlen(temp));
            strncpy(buffer, temp, MAX_CMD_LEN);
        }
    }
    return buffer;
}

void execute_command(char *command) {
    char *args[64];
    char *token = strtok(command, " ");
    int i = 0;

    while (token && i < 63) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (strcmp(args[0], "cd") == 0) {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s\n", cwd);
        }
    } else if (strcmp(args[0], "set") == 0) {
        if (args[1] && args[2]) {
            set_env(args[1], args[2]);
        } else {
            fprintf(stderr, "Usage: set <VAR> <value>\n");
        }
    } else if (strcmp(args[0], "unset") == 0) {
        if (args[1]) {
            unset_env(args[1]);
        } else {
            fprintf(stderr, "Usage: unset <VAR>\n");
        }
    } else if (strcmp(args[0], "echo") == 0) {
        for (int j = 1; args[j] != NULL; j++) {
            if (args[j][0] == '$') {
                const char *value = get_env(args[j] + 1); // Skip '$'
                printf("%s", value);
            } else {
                printf("%s", args[j]);
            }
            if (args[j + 1]) {
                printf(" ");
            }
        }
        printf("\n");
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }
    }
}

void xsh_run() {
    char command[MAX_CMD_LEN];
    while (1) {
        printf("xsh# ");
        if (fgets(command, MAX_CMD_LEN, stdin) == NULL) {
            break;
        }
        command[strcspn(command, "\n")] = '\0';
        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
            break;
        }
        execute_command(command);
    }
}