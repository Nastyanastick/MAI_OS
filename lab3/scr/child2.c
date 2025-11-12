#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/mman.h>

#define BUF_SIZE 1024

typedef struct{
    char data[BUF_SIZE];
    size_t len;
}shm_t;

int main(int argc, char *argv[]){
    if (argc != 7){
        fprintf(stderr, "Error: usage child2 shm2 shm3 sem2_empty sem2_full sem3_empty sem3_full\n");
        exit(1);
    }

    char *shm2_name = argv[1];
    char *shm3_name = argv[2];
    char *sem2_empty_name = argv[3];
    char *sem2_full_name = argv[4];
    char *sem3_empty_name = argv[5];
    char *sem3_full_name = argv[6];

    //открываем shared memory
    int fd2 = shm_open(shm2_name, O_RDWR, 0666);
    int fd3 = shm_open(shm3_name, O_RDWR, 0666);

    shm_t *shm2 = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    shm_t *shm3 = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);

    //открываем семафоры
    sem_t *sem2_empty = sem_open(sem2_empty_name, 0);
    sem_t *sem2_full = sem_open(sem2_full_name, 0);
    sem_t *sem3_empty = sem_open(sem3_empty_name, 0);
    sem_t *sem3_full = sem_open(sem3_full_name, 0);

    while(1){
        sem_wait(sem2_full);
        if (shm2->len == 0) break;
        for (size_t i = 0; i < shm2->len; i++){
            if (shm2->data[i] == ' ' || shm2->data[i] == '\t') shm3->data[i] = '_';
            else shm3->data[i] = shm2->data[i];
        }

        shm3->len = shm2->len;

        sem_post(sem2_empty);
        sem_post(sem3_full);
    }
    shm3->len = 0;
    sem_post(sem3_full);

    munmap(shm2, sizeof(shm_t));
    munmap(shm3, sizeof(shm_t));

    return 0;
}