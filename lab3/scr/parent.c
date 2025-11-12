#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <semaphore.h>



#define BUF_SIZE 1024

typedef struct {
    char data[BUF_SIZE];
    size_t len;
} shm_t;

int main(){
    pid_t pid = getpid(); //сохраняем идентификатор PID текущего процесса
    char shm1_name[64], shm2_name[64], shm3_name[64]; 
    char sem1_empty_name[64], sem1_full_name[64];
    char sem2_empty_name[64], sem2_full_name[64]; //создаем массивы символов, где будут храниться имена наших объектов POSIX
    char sem3_empty_name[64], sem3_full_name[64]; //три shared memory и шесть семафоров

    sprintf(shm1_name, "/shm1_%d", pid); //имена для объектов shared memory
    sprintf(shm2_name, "/shm2_%d", pid);
    sprintf(shm3_name, "/shm3_%d", pid);

    sprintf(sem1_empty_name, "/sem1_empty_%d", pid); //имена для семафоров
    sprintf(sem1_full_name,  "/sem1_full_%d", pid); // empty отвечатает за пустоту; =1 значит можно писать
    sprintf(sem2_empty_name, "/sem2_empty_%d", pid); // full отвечает за заполненность; =0 значит нечего читать
    sprintf(sem2_full_name,  "/sem2_full_%d", pid); 
    sprintf(sem3_empty_name, "/sem3_empty_%d", pid);
    sprintf(sem3_full_name,  "/sem3_full_%d", pid);

    // создаем три объекта shared memory (возвращает дескриптор файла)
    int fd1 = shm_open(shm1_name, O_CREAT | O_RDWR, 0666); 
    int fd2 = shm_open(shm2_name, O_CREAT | O_RDWR, 0666); 
    int fd3 = shm_open(shm3_name, O_CREAT | O_RDWR, 0666);

    ftruncate(fd1, sizeof(shm_t));
    ftruncate(fd2, sizeof(shm_t));
    ftruncate(fd3, sizeof(shm_t));

    shm_t *shm1 = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    shm_t *shm2 = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    shm_t *shm3 = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);

        //создаем семафоры для каждой пары связей (empty и full)
    sem_t *sem1_empty = sem_open(sem1_empty_name, O_CREAT, 0666, 1);
    sem_t *sem1_full = sem_open(sem1_full_name, O_CREAT, 0666, 0);
    sem_t *sem2_empty = sem_open(sem2_empty_name, O_CREAT, 0666, 1);
    sem_t *sem2_full = sem_open(sem2_full_name, O_CREAT, 0666, 0);
    sem_t *sem3_empty = sem_open(sem3_empty_name, O_CREAT, 0666, 1);
    sem_t *sem3_full = sem_open(sem3_full_name, O_CREAT, 0666, 0);


    if (fork() == 0){
        execl ("./child1", "child1", shm1_name, shm2_name, sem1_empty_name, 
            sem1_full_name, sem2_empty_name, sem2_full_name, (char*)NULL);
        perror("execl child1");
        exit(1);
    }

    if (fork() == 0){
        execl("./child2", "child2", shm2_name, shm3_name, sem2_empty_name, 
            sem2_full_name, sem3_empty_name, sem3_full_name, (char*)NULL);
        perror("execl child2");
        exit(1);
    }

    char input[BUF_SIZE];

    while(fgets(input, BUF_SIZE, stdin)){
        size_t len = strlen(input);
        sem_wait(sem1_empty);
        memcpy(shm1->data, input, len);
        shm1->len = len;

        //можно читать child1
        sem_post(sem1_full);
        
        sem_wait(sem3_full);

        write(STDOUT_FILENO, shm3->data, shm3->len);
        sem_post(sem3_empty);
    }

    wait(NULL);
    wait(NULL);

    //освобождаем ресурсы памяти
    munmap(shm1, sizeof(shm_t));
    munmap(shm2, sizeof(shm_t));
    munmap(shm3, sizeof(shm_t));

    //удаляем объекты shared memory
    shm_unlink(shm1_name);
    shm_unlink(shm2_name);
    shm_unlink(shm3_name);

    //закрываем семафоры
    sem_close(sem1_empty);
    sem_close(sem1_full);
    sem_close(sem2_empty);
    sem_close(sem2_full);
    sem_close(sem3_empty);
    sem_close(sem3_full);

    //удаляем именнованные семафоры и системы
    sem_unlink(sem1_empty_name);
    sem_unlink(sem1_full_name);
    sem_unlink(sem2_empty_name);
    sem_unlink(sem2_full_name);
    sem_unlink(sem3_empty_name);
    sem_unlink(sem3_full_name);

    return 0;

}