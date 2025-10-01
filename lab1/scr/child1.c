#include <unistd.h>
#include <ctype.h> //для нижнего регистра tolower
#include <string.h>



int main(){

    char buf[1024];
    ssize_t r1;

    while ((r1 = read(STDIN_FILENO, buf, sizeof(buf))) > 0){
        for (ssize_t i = 0; i < r1; i++){
            buf[i] = tolower((unsigned char)buf[i]);

        }
        if (write(STDOUT_FILENO, buf, r1) == -1){
            const char *err = "Error: failed to write to fd2 in child1\n";
            write(2, err, strlen(err));
            return 1;
        }
    }

    if (r1 == -1){
        const char *err2 = "Error: failed to read from fd1\n";
        write(2, err2, strlen(err2));
        return 1;
    }

    return 0;
}