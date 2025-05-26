#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_WAITING_CHAIRS 5
#define NUM_STUDENTS 10

int waiting_students = 0;

pthread_mutex_t mutex;
sem_t ta_sleep;
sem_t student_sem;

void* ta(void* arg) {
    while (1) {
        // TA sleeps if no students
        sem_wait(&ta_sleep);

        pthread_mutex_lock(&mutex);
        if (waiting_students == 0) {
            pthread_mutex_unlock(&mutex);
            continue;
        }
        waiting_students--;
        printf("TA is helping a student. Students waiting: %d\n", waiting_students);
        pthread_mutex_unlock(&mutex);

        // Help student (random time between 1 and 2 seconds)
        sleep(1 + rand() % 2);

        // Notify student
        sem_post(&student_sem);
        printf("TA finished helping a student.\n");
    }
    return NULL;
}

void* student(void* id_ptr) {
    int student_id = *((int*)id_ptr);
    while (1) {
        // Random arrival between 0.5 and 3 seconds
        usleep((500000 + rand() % 2500000));  // in microseconds

        pthread_mutex_lock(&mutex);
        if (waiting_students < MAX_WAITING_CHAIRS) {
            waiting_students++;
            printf("Student %d is waiting. Students waiting: %d\n", student_id, waiting_students);
            sem_post(&ta_sleep); // Wake TA if sleeping
            pthread_mutex_unlock(&mutex);

            // Wait for TA to help
            sem_wait(&student_sem);
            printf("Student %d got help and leaves.\n", student_id);
        } else {
            pthread_mutex_unlock(&mutex);
            printf("Student %d found no empty chair and will come back later.\n", student_id);
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t ta_thread;
    pthread_t student_threads[NUM_STUDENTS];
    int student_ids[NUM_STUDENTS];

    pthread_mutex_init(&mutex, NULL);
    sem_init(&ta_sleep, 0, 0);
    sem_init(&student_sem, 0, 0);

    pthread_create(&ta_thread, NULL, ta, NULL);

    for (int i = 0; i < NUM_STUDENTS; i++) {
        student_ids[i] = i + 1;
        pthread_create(&student_threads[i], NULL, student, &student_ids[i]);
    }

    // Run simulation for 12 seconds
    sleep(12);
    printf("Simulation finished.\n");

    return 0;
}
