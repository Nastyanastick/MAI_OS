#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <ctype.h>


#define BUF_SIZE 1024

typedef struct{
    char data[BUF_SIZE];
    size_t len;
}shm_t;

int main(int argc, char *argv[]){
    if (argc != 7){
        fprintf(stderr, "Error: usage child1 shm1 shm2 sem1_empty sem1_full sem2_empty sem2_full\n");
        exit(1);
    }

    char *shm1_name = argv[1];
    char *shm2_name = argv[2];
    char *sem1_empty_name = argv[3];
    char *sem1_full_name = argv[4];
    char *sem2_empty_name = argv[5];
    char *sem2_full_name = argv[6];

    //открываем shared memory
    int fd1 = shm_open(shm1_name, O_RDWR, 0666);
    int fd2 = shm_open(shm2_name, O_RDWR, 0666);

    shm_t *shm1 = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    shm_t *shm2 = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

    //открываем семафоры
    sem_t *sem1_empty = sem_open(sem1_empty_name, 0);
    sem_t *sem1_full = sem_open(sem1_full_name, 0);
    sem_t *sem2_empty = sem_open(sem2_empty_name, 0);
    sem_t *sem2_full = sem_open(sem2_full_name, 0);

    while(1){

        //ждем пока родитель запишет
        sem_wait(sem1_full);

        if (shm1->len == 0) break;

        for (size_t i = 0; i < shm1->len; i++) shm2->data[i] = tolower((unsigned char)shm1->data[i]);
        shm2->len = shm1->len;

        //освобождаем shm1 (можно снова писать)
        sem_post(sem1_empty);
        sem_post(sem2_full); //сообщаем, что в shm2 появились новые данные

    }

    shm2->len = 0;
    sem_post(sem2_full);

    munmap(shm1, sizeof(shm_t));
    munmap(shm2, sizeof(shm_t));

    return 0;

}