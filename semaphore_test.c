//Producteur :
//============
int main() {
    sem_t *empty = sem_open("/empty", O_CREAT, 0644, BUFFER_SIZE);
    sem_t *full = sem_open("/full", O_CREAT, 0644, 0);
    if (empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, sizeof(shared_data_t));

    shared_data_t *shared = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shared->in = 0;

    for (int i = 1; i <= 10; i++) {
        sem_wait(empty);
        shared->buffer[shared->in] = i;
        printf("Producer: Produced %d\n", i);
        shared->in = (shared->in + 1) % BUFFER_SIZE;
        sem_post(full);
        sleep(1);  // Simulate production time
    }

    munmap(shared, sizeof(shared_data_t));
    close(shm_fd);
    sem_close(empty);
    sem_close(full);
    return 0;
}




// ==========
//Consumer  : 
int main() {
    sem_t *empty = sem_open("/empty", 0);
    sem_t *full = sem_open("/full", 0);
    if (empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open("/shared_memory", O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    shared_data_t *shared = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    shared->out = 0;

    for (int i = 1; i <= 10; i++) {
        sem_wait(full);
        int item = shared->buffer[shared->out];
        printf("Consumer: Consumed %d\n", item);
        shared->out = (shared->out + 1) % BUFFER_SIZE;
        sem_post(empty);
        sleep(2);  // Simulate consumption time
    }

    munmap(shared, sizeof(shared_data_t));
    close(shm_fd);
    sem_close(empty);
    sem_close(full);
    return 0;
}

