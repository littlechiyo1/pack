// 停车场 128 个车位，2入口，2出口

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_NUM 128 // 总车位
#define IN_NUM 2    // 入口数
#define OUT_NUM 2   // 出口数

int cars = 0;       // 记录已停车数
int wait_num = 0;   // 待停车数
int empty_num;      // 总空车位数

sem_t empty_sem;  // 空车位信号量
sem_t fill_sem;  // 非空车位信号量

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   // 车位锁

int main(int argc, char const *argv[])
{
    void *pack_in(void *arg);
    void *pack_out(void *arg);

    int wait_cars = argc > 1 ? atoi(argv[1]) : 200;    // 等待停车数量(默认200)
    empty_num = wait_cars;  // 总空车位数

    pthread_t in_p[IN_NUM], out_p[OUT_NUM];

    // 信号量初始值为 2
    sem_init(&empty_sem, 0, MAX_NUM);
    sem_init(&fill_sem, 0, 0);

    pthread_mutex_init(&mutex, NULL);

    for (size_t i = 0; i < OUT_NUM; i++)
        pthread_create(&out_p[i], NULL, pack_out, &wait_cars);
    for (size_t i = 0; i < IN_NUM; i++)
        pthread_create(&in_p[i], NULL, pack_in, &wait_cars);

    // 等待所有线程结束
    for (int i = 0; i < IN_NUM; i++) {
        pthread_join(in_p[i], NULL);
    }

    for (int i = 0; i < OUT_NUM; i++) {
        pthread_join(out_p[i], NULL);
    }

    sem_destroy(&empty_sem);
    sem_destroy(&fill_sem);

    pthread_mutex_destroy(&mutex);

    return 0;
}

void *pack_in(void *arg)
{
    while (wait_num < *(int *)arg)     // 循环次数---总等待停车数
    {
        sem_wait(&empty_sem); // 等待有空闲车位
        pthread_mutex_lock(&mutex); // 加锁保护停车场状态

        cars++;
        printf("%d 进入---当前占用车位数: %d\n", wait_num, cars);
        wait_num++;

        // usleep(10000); // 入场花费时间

        pthread_mutex_unlock(&mutex); // 解锁
        sem_post(&fill_sem); // 增加已占车位信号量
    }

    return NULL;
}

void *pack_out(void *arg)
{
    while (empty_num > 0)
    {
        sem_wait(&fill_sem);
        pthread_mutex_lock(&mutex);

        cars--;
        printf("%d 离开---当前占用车位数: %d\n", *(int *)arg - empty_num, cars);
        empty_num--;

        // usleep(10000); // 出场花费时间

        pthread_mutex_unlock(&mutex);
        sem_post(&empty_sem);
    }

    return NULL;
}