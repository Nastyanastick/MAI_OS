#include <pthread.h> //для PROXI Threads
#include <stdlib.h> //для malloc
#include <semaphore.h> //для sem_t
#include <unistd.h> //для write()
#include <sys/time.h> //для gettimeofday
#include <time.h> //для time()/srand()


void write_int(long v){
    char buf[32];
    int i = 0;
    if (v == 0) {write(1, "0", 1); return;}
    if (v < 0) {write(1, "-", 1); v = -v;}
    while ( v > 0){
        buf[i++] = '0' + (v % 10);
        v/=10;
    }
    for (int j = i - 1; j >= 0; --j) write(1, &buf[j], 1);
}

void merge(int *arr, int left, int mid, int right){//слияние двух отсортированных подмассивов
    int i = left, j = mid + 1, k = 0;
    int len = right - left + 1; //количество элементов в подмассиве
    int *tmp = (int*) malloc(len * sizeof(int)); //просим ядро выделить память
    if (!tmp) return; //ошибка выделения

    while (i <= mid && j <= right){
        if (arr[i] <= arr[j])
            tmp[k++] = arr[i++];
        else
            tmp[k++] = arr[j++];
    }

    while (i <= mid) tmp[k++] = arr[i++]; //остатки левой части
    while (j <= right) tmp[k++] = arr[j++]; //остатки правой части

    for (i = 0; i < k; i++) arr[left + i] = tmp[i]; //записываем отсортированное в исходный массив
    free(tmp);

}

void merge_sort_recurs(int *arr, int left, int right){ //обычная рекурсивная сортировка без потоков (последовательная)
    if (left >= right) return;
    int mid = (left + right) / 2;
    merge_sort_recurs(arr, left, mid);
    merge_sort_recurs(arr, mid + 1, right);
    merge(arr, left, mid, right);
}



//структура аргументов потоков
typedef struct{
    int *arr;
    int left;
    int right;
    sem_t *sem; //указатель на семафор (ограничивает число одновременно работающих потоков)
}thread_arg_t;


void* merge_sort_thread(void *arg){
    thread_arg_t *data = (thread_arg_t*) arg; //преобразуем указатель void к типу thread_arg_t

    int left = data->left;
    int right = data->right;
    int *arr = data->arr;

    if (left >= right){
        sem_post(data->sem);//освобождаем место (один поток завершился)
        free(data);
        return NULL;
    }

    int mid = (left + right) / 2;

        //идентификаторы потоков
    pthread_t left_thread, right_thread;
    int created_left = 0, created_right = 0; 

    if (sem_trywait(data->sem) == 0){
        thread_arg_t *left_arg = malloc(sizeof(thread_arg_t));
        if (left_arg){
            left_arg->arr = arr;
            left_arg->left = left;
            left_arg->right = mid;
            left_arg->sem = data->sem;

            if (pthread_create(&left_thread, NULL, merge_sort_thread, left_arg) == 0)
    //**куда сохранить идентификатор, атибуты потока, функция которую будет выполнять, аргумент который передаем в строку
                created_left = 1;
            else{
                sem_post(data->sem);
                free(left_arg);
                merge_sort_recurs(arr, left, mid);
            }
        }else{
            sem_post(data->sem);
            merge_sort_recurs(arr, left, mid); //если нет место, сортируем
        }
    } else{
        merge_sort_recurs(arr, left, mid);
    }

    //для правой:
    if (sem_trywait(data->sem) == 0){
        thread_arg_t *right_arg = malloc(sizeof(thread_arg_t));
        if (right_arg){
            right_arg->arr = arr;
            right_arg->left = mid + 1;
            right_arg->right = right;
            right_arg->sem = data->sem;

            if (pthread_create(&right_thread, NULL, merge_sort_thread, right_arg) == 0)
                created_right = 1;
            else{
                sem_post(data->sem);
                free(right_arg);
                merge_sort_recurs(arr, mid + 1, right);
            }
        }else{
            sem_post(data->sem);
            merge_sort_recurs(arr, mid + 1, right); //если нет место, сортируем
        }
    } else{
        merge_sort_recurs(arr, mid + 1, right);
    }


    //ждем завершения
    if (created_left) pthread_join(left_thread, NULL);
    if (created_right) pthread_join(right_thread, NULL);

    merge(arr, left, mid, right); //слияение отсортированных половин

    sem_post(data->sem); //освобождаем разрешение
    free(data);
    return NULL;

}




int main(int argc, char* argv[]){
    if (argc != 2){
        write(2, "Usage: Enter 2 arguments\n", sizeof("Usage: Enter 2 arguments\n") - 1);
        return 1;
    }

    int MAX_THREADS = atoi(argv[1]); //максимум потоков одновременно (преобразуем строку в число)
    int N = 10000; //размер массива

    int *arr = (int*) malloc(N * sizeof(int));
    int* arr_copy = (int*) malloc(N * sizeof(int));
    if (!arr || !arr_copy){
        write(2, "Memory allocation error\n", sizeof("Memory allocation error\n") - 1);
        return 1;
    }

    srand(time(NULL)); //генератор случайных чисел
    for (int i = 0; i < N; i++){
        arr[i] = rand() % 10000; //случайное число от 0 до 9999
        arr_copy[i] = arr[i]; //сохраняем копию для второй версии сортировки
    }

    struct timeval start, end; //для измерения времени (миллисекунды)

    gettimeofday(&start, NULL);
    merge_sort_recurs(arr_copy, 0, N - 1);
    gettimeofday(&end, NULL);

    double seq_time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    write(1, "Sequential sort done\n", 21);


    sem_t sem;
    sem_init(&sem, 0, MAX_THREADS); //инициализируем семафор

    thread_arg_t *arg = malloc(sizeof(thread_arg_t)); //структура аргументов для главного потока сортировки
    if (!arg){
        write(2, "Memory allocation error\n", sizeof("Memory allocation error\n") - 1);
        return 1;
    }
    arg->arr = arr;
    arg->left = 0;
    arg->right = N - 1;
    arg->sem = &sem;

    gettimeofday(&start, NULL);
    sem_wait(&sem);
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, merge_sort_thread, arg);
    pthread_join(main_thread, NULL);

    gettimeofday(&end, NULL);

    double par_time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    write(1, "Parallel sort done\n", 19);

    char buffer[128];
    write(1, "Sequential: ", sizeof("Sequential: ") - 1);
    write_int((long) (seq_time)); //миллисекунды
    write(1, " ms\n", 4);

    write(1, "Parallel: ", sizeof("Parallel: ") - 1);
    write_int((long) (par_time));
    write(1, " ms\n", 4);

    write(1, "Sorted array:\n", 14);
    for (int i = 0; i < N; i++) {
        write_int(arr[i]);
        write(1, " ", 1);
    }
    write(1, "\n", 1);

    sem_destroy(&sem);
    free(arr);
    free(arr_copy);

    return 0;
}