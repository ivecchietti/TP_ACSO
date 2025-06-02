#define _POSIX_C_SOURCE 200809L  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMANDS 256
#define MAX_ARGS 65  


char *trim_spaces(char *str) {
    while (isspace(*str)) str++;
    if (*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    return str;
}


char **parse_args(const char *command) {
    char **tokens = malloc(MAX_ARGS * sizeof(char *));
    if (!tokens) {
        perror("malloc");
        exit(1);
    }

    char *copy = strdup(command);
    if (!copy) {
        perror("strdup");
        exit(1);
    }

    int pos = 0;
    char *start = copy;
    while (*start) {
        while (isspace(*start)) start++;
        if (*start == '\0') break;

        char *arg;
        if (*start == '"' || *start == '\'') {
            char quote = *start++;
            char *end = strchr(start, quote);
            if (!end) {
                fprintf(stderr, "Error: comilla no cerrada\n");
                free(tokens);
                free(copy);
                return NULL;
            }
            *end = '\0';
            arg = strdup(start);
            start = end + 1;
        } else {
            char *end = start;
            while (*end && !isspace(*end)) end++;
            char temp = *end;
            *end = '\0';
            arg = strdup(start);
            *end = temp;
            start = end;
        }

        if (!arg) {
            perror("strdup");
            free(tokens);
            free(copy);
            return NULL;
        }

        if (pos >= MAX_ARGS - 1) {
            fprintf(stderr, "Error: demasiados argumentos\n");
            for (int i = 0; i < pos; i++) free(tokens[i]);
            free(tokens);
            free(copy);
            return NULL;
        }

        tokens[pos++] = arg;
    }

    tokens[pos] = NULL;
    free(copy);
    return tokens;
}

void free_args(char **args) {
    for (int i = 0; args[i]; i++) free(args[i]);
    free(args);
}

int main() {
    char input[4096]; 

    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) continue;

        int dq = 0, sq = 0;
        for (int i = 0; input[i]; i++) {
            if (input[i] == '"') dq++;
            if (input[i] == '\'') sq++;
        }
        if (dq % 2 != 0 || sq % 2 != 0) {
            fprintf(stderr, "Error: comilla no cerrada\n");
            continue;
        }

        char *trimmed = trim_spaces(input);
        if (trimmed[0] == '|' || trimmed[strlen(trimmed) - 1] == '|') {
            fprintf(stderr, "Error de sintaxis: pipe al inicio o final\n");
            continue;
        }
        if (strstr(trimmed, "||")) {
            fprintf(stderr, "Error de sintaxis: pipes consecutivos\n");
            continue;
        }

        char *commands[MAX_COMMANDS];
        int command_count = 0;
        char *token = strtok(trimmed, "|");
        while (token) {
            char *clean = trim_spaces(token);
            if (strlen(clean) == 0) {
                fprintf(stderr, "Error de sintaxis: comando vacío entre pipes\n");
                command_count = -1;
                break;
            }
            if (command_count >= MAX_COMMANDS) {
                fprintf(stderr, "Error: pipeline demasiado largo\n");
                command_count = -1;
                break;
            }
            commands[command_count++] = clean;
            token = strtok(NULL, "|");
        }

        if (command_count <= 0) continue;

        if (command_count == 1 && strcmp(commands[0], "exit") == 0) break;

        int prev_fd = -1, fd[2];

        for (int i = 0; i < command_count; i++) {
            if (i < command_count - 1 && pipe(fd) == -1) {
                perror("pipe");
                exit(1);
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                if (prev_fd != -1) {
                    dup2(prev_fd, STDIN_FILENO);
                    close(prev_fd);
                }

                if (i < command_count - 1) {
                    close(fd[0]);
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[1]);
                }

                char **args = parse_args(commands[i]);
                if (!args) exit(1);

                execvp(args[0], args);
                fprintf(stderr, "Error: comando no encontrado: %s\n", args[0]);
                perror("execvp");
                free_args(args);
                exit(1);
            } else {
                if (prev_fd != -1) close(prev_fd);
                if (i < command_count - 1) {
                    close(fd[1]);
                    prev_fd = fd[0];
                }
            }
        }

        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }
    }

    return 0;
}
