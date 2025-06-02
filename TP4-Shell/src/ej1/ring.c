#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int start, pid, n;
    int val;

    if (argc != 4) {printf("Uso: anillo <n> <c> <s> \n"); exit(1);}

    n = atoi(argv[1]); 
    val = atoi(argv[2]);  
    start = atoi(argv[3]); 

    printf("Se crearán %d procesos, se enviará el valor %d desde proceso %d\n", n, val, start);

    int pipes[n][2];       
    int parent_pipe[2];   

    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    if (pipe(parent_pipe) == -1) {
        perror("pipe padre");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } 
		else if (pid == 0) {
            for (int j = 0; j < n; j++) {
                if (j != i) close(pipes[j][0]);            
                if (j != (i + 1) % n) close(pipes[j][1]);     
            }

            close(parent_pipe[0]);
            if (i != (start + n - 1) % n) close(parent_pipe[1]); 

            int num;
            if (read(pipes[i][0], &num, sizeof(int)) != sizeof(int)) {
                perror("read hijo");
                exit(1);
            }
            close(pipes[i][0]);

            printf("Proceso %d recibió %d, lo incrementa a %d\n", i, num, num + 1);
            fflush(stdout);
            num++;

            if (i == (start + n - 1) % n) {
                if (write(parent_pipe[1], &num, sizeof(int)) != sizeof(int)) {
                    perror("write al padre");
                    exit(1);
                }
                close(parent_pipe[1]);
            } else {
                if (write(pipes[(i + 1) % n][1], &num, sizeof(int)) != sizeof(int)) {
                    perror("write al siguiente");
                    exit(1);
                }
                close(pipes[(i + 1) % n][1]);
            }

            exit(0);
        }
    }

    if (write(pipes[start][1], &val, sizeof(int)) != sizeof(int)) {
        perror("write padre");
        exit(1);
    }
    close(pipes[start][1]);

    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        if (i != start) close(pipes[i][1]);
    }

    close(parent_pipe[1]);

    if (read(parent_pipe[0], &val, sizeof(int)) != sizeof(int)) {
        perror("read padre");
        exit(1);
    }
    close(parent_pipe[0]);

    printf("Valor final recibido en el proceso padre: %d\n", val);

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    return 0;
}
