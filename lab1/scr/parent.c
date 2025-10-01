#include <unistd.h>
#include <string.h>
#include <sys/wait.h>



int main(){
    int fd1[2];
    int fd2[2];
    int fd3[2];

    if (pipe(fd1) == -1){ //создаем канал
        const char *err1 = "Error: failed to create pipe1\n";
        write(2, err1, strlen(err1));
        return 1;
    }

    if (pipe(fd2) == -1){
        const char *err2 = "Error: failed to creat pipe2\n";
        write(2, err2, strlen(err2));
        return 1;
    }

    if (pipe(fd3) == -1){
        const char *err3 = "Error: failed to create pipe3\n";
        write (2, err3, strlen(err3));
        return 1;
    }


    
    pid_t pid1 = fork(); //создаем дочерний процесс 1
    if (pid1 == -1){
        const char *error1 = "Error: failed to spawn new process1\n";
        write (2, error1, strlen(error1));
        return 1;
    } else if (pid1 == 0){
        if (dup2(fd1[0], STDIN_FILENO) == -1){
            const char *er5 = "Error: failed to dup2 fd1 STDIN\n";
            write(2, er5, strlen(er5));
            return 1;
        }

        if (dup2(fd2[1], STDOUT_FILENO) == -1){
            const char *er6 = "Error: failed to dup2 fd2 STDOUT\n";
            write(2, er6, strlen(er6));
            return 1;
        }

        if (close(fd1[1]) == -1){
            const char *e1 = "Error: failed to close fd1[1] in child1\n";
            write(2, e1, strlen(e1));
            return 1;

        }
        if (close(fd2[0]) == -1){
            const char *e2 = "Error: failed to close fd2[0] in child1\n";
            write(2, e2, strlen(e2));
            return 1;

        }
        if (close(fd3[0]) == -1){
            const char *e3 = "Error: failed to close fd3[0] in child1\n";
            write(2, e3, strlen(e3));
            return 1;

        }
        if (close(fd3[1]) == -1){
            const char *e4 = "Error: failed to close fd3[1] in child1\n";
            write(2, e4, strlen(e4));
            return 1;

        }

        if (execl("./child1", "child1", (char *)NULL) == -1){
            const char *erro1 = "Error: failed to execl child1\n";
            write(2, erro1, strlen(erro1));
            return 1;
        }

    }



    pid_t pid2 = fork(); //создаем дочерний процесс 2
    if (pid2 == -1){
        const char *error2 = "Error: failed to spawn new process2\n";
        write(2, error2, strlen(error2));
        return 1;
    }
    if (pid2 == 0){
        if (dup2(fd2[0], STDIN_FILENO) == -1){
            const char *er3 = "Error: failed to dup2 fd2 STDIN\n";
            write(2, er3, strlen(er3));
            return 1;
        }

        if (dup2(fd3[1], STDOUT_FILENO) == -1){
            const char *er4 = "Error: failed to dup2 fd3 STDOUT\n";
            write(2, er4, strlen(er4));
            return 1;
        }

        if (close(fd1[0]) == -1){
            const char *e5 = "Error: failed to close fd1[0] in child2\n";
            write(2, e5, strlen(e5));
            return 1;

        }
        if (close(fd1[1]) == -1){
            const char *e6 = "Error: failed to close fd1[1] in child2\n";
            write(2, e6, strlen(e6));
            return 1;

        }
        if (close(fd2[1]) == -1){
            const char *e7 = "Error: failed to close fd2[1] in child2\n";
            write(2, e7, strlen(e7));
            return 1;

        }
        if (close(fd3[0]) == -1){
            const char *e8 = "Error: failed to close fd3[0] in child2\n";
            write(2, e8, strlen(e8));
            return 1;

        }

        if (execl("./child2", "child2", (char *)NULL) == -1) {
            const char *erro2 = "Error: failed to execl child2\n";
            write(2, erro2, strlen(erro2));
            return 1;
        }
    }

    if (close(fd1[0]) == -1){
        const char *e9 = "Error: failed to close fd1[0] in parent\n";
        write(2, e9, strlen(e9));
        return 1;
    }

    if (close(fd2[0]) == -1){
        const char *e10 = "Error: failed to close fd2[0] in parent\n";
        write(2, e10, strlen(e10));
        return 1;
    }
    if (close(fd2[1]) == -1){
        const char *e11 = "Error: failed to close fd2[1] in parent\n";
        write(2, e11, strlen(e11));
        return 1;
    }
    if (close(fd3[1]) == -1){
        const char *e12 = "Error: failed to close fd3[1] in parent\n";
        write(2, e12, strlen(e12));
        return 1;
    }


    char input[1024];
    ssize_t r;
    char buf2[1024];
    ssize_t w1;
    

    while ((r = read(STDIN_FILENO, input, sizeof(input))) > 0){ //читаем пользователя
        if (write(fd1[1], input, r) == -1){ //отправляем ребенку 1
            const char *er1 = "Error: failed to write to fd1 in parent\n";
            write(2, er1, strlen(er1));
            return 1;

        }
        //читаем от ребенка 2
        w1 = read(fd3[0], buf2, sizeof(buf2));
        if (w1 == -1){
            const char *error4 = "Error: failed to read from fd3\n";
            write(2, error4, strlen(error4));
            return 1;
        }
        if (w1 > 0) {
            if (write (1, buf2, w1) == -1){ //выводим итог
                const char *er2 = "Error: failed to write to stdout in parent\n";
                write(2, er2, strlen(er2));
                return 1;

            }
        }
    }


    if (r == -1){
        const char *error3 = "Error: failed to read from stdin\n";
        write(2, error3, strlen(error3));
        return 1;
    }

    if (close(fd1[1]) == -1){
        const char *e13 = "Error: failed to close fd1[1] in parent\n";
        write(2, e13, strlen(e13));
        return 1;
    }
    if (close(fd3[0]) == -1){
        const char *e14 = "Error: failed to close fd3[0] in parent\n";
        write(2, e14, strlen(e14));
        return 1;
    }


    if (wait(NULL) == -1) {
        const char *erro3 = "Error: failed to wait child\n";
        write (2, erro3, strlen(erro3));
        return 1;
    }

    if (wait(NULL) == -1) {
        const char *erro3 = "Error: failed to wait child\n";
        write (2, erro3, strlen(erro3));
        return 1;
    }

    return 0;

}