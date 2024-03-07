#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

int shared_value = 0;

void *incrementer_thread(void *arg)
{
    while(shared_value < 5)
    {
        sleep(1);
        shared_value++;
    }
    return NULL;
}

sem_t* eventopen(const char *name)
{
    if (name == NULL)
    {
        fprintf(stderr, "Eroare: Numele semaforului este NULL\n");

        return NULL;
    }

    sem_t *sem = sem_open(name, O_CREAT, 0644, 0);

    if (sem == SEM_FAILED)
    {
        perror("sem_open failed");
        exit(1);
    }

    return sem;
}

void eventclose(sem_t *sem)
{
    if (sem != NULL)
        sem_close(sem);
    else
    {
        fprintf(stderr, "Semaforul este NULL\n");

        return;
    }

}

void eventwait(sem_t *sem)
{
    int ret;

    if (sem == NULL)
    {
        fprintf(stderr, "Semaforul este NULL\n");

        return;
    }

    while (1)
    {
        ret = sem_wait(sem);

        if (ret == 0)
            break;
        else
        if (errno == EINTR)
            continue;
        else
        {
            perror("sem_wait failed");
            break;
        }

    }

}

void eventsignal(sem_t *sem, int num_signals, int* shr_val, int target_value)
{
    if (sem == NULL)
    {
        fprintf(stderr, "Eroare: Semaforul este NULL\n");

        return;
    }

    if (num_signals <= 0)
    {
        fprintf(stderr, "Eroare: Numărul de semnale trebuie să fie pozitiv\n");

        return;
    }

    while (1) {
        if (*shr_val >= target_value) {
            break;
        }
    }

    clock_t start, end;
    double time_spent;



    if (sem != NULL)
    {
        start = clock();

        for (int i = 0; i < num_signals; ++i)
            sem_post(sem);

        end = clock();

        time_spent = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Timpul de rulare: %f secunde\n", time_spent);
    }
    else
    {
        fprintf(stderr, "Semaforul este NULL\n");

        return;
    }
}


const int NUM_CHILDREN = 500;
const char *eventName = "/test";

void childProcess(sem_t *event)
{
    printf("Procesul copil %d așteaptă evenimentul...\n", getpid());

    eventwait(event);

    printf("Procesul copil %d a primit semnalul evenimentului.\n", getpid());
}

int main()
{
    pid_t pids[NUM_CHILDREN];
    sem_t *event = eventopen(eventName);

    for (int i = 0; i < NUM_CHILDREN; ++i)
    {
        pids[i] = fork();

        if (pids[i] < 0)
        {
            perror("fork failed");
            exit(1);
        }

        if (pids[i] == 0)
        {
            childProcess(event);
            eventclose(event);
            exit(0);
        }
    }


    printf("Procesul părinte semnalează evenimentul.\n");

    int target_value = 5;
    pthread_t inc_thread;

    if (pthread_create(&inc_thread, NULL, incrementer_thread, NULL) != 0) {
        printf("Thread creation failed\n");
        return 1;
    }

    eventsignal(event, NUM_CHILDREN, &shared_value, target_value);

    for (int i = 0; i < NUM_CHILDREN; ++i)
        waitpid(pids[i], NULL, 0);

    pthread_join(inc_thread, NULL);

    eventclose(event);
    sem_unlink(eventName);

    return 0;
}