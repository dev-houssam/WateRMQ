#include "WateRMQ.hpp"
#include <cstring>
#include <cstdio>
#include <thread>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>

using namespace std;

WateRMQ::WateRMQ(const string& filename) : _filename(filename), _running(false) {
    _sem = nullptr;
    _ptr_queue = nullptr;
    _init();
}

WateRMQ::~WateRMQ() {
    _cleanup();
}

void WateRMQ::_cleanup() {
    _running = false;
    
    if (_ptr_queue) {
        // Free all items in queue
        for (int i = 0; i < MAX_SIZE; i++) {
            if (_ptr_queue->items[i]) {
                free(_ptr_queue->items[i]);
                _ptr_queue->items[i] = nullptr;
            }
        }
        munmap(_ptr_queue, sizeof(Queue));
        _ptr_queue = nullptr;
    }
    
    if (_sem) {
        delete _sem;
        _sem = nullptr;
    }
    
    // Unlink shared memory
    shm_unlink(_filename.c_str());
}

void WateRMQ::_init() {
    int shm_fd;
    
    // Create or open shared memory
    shm_fd = shm_open(_filename.c_str(), O_CREAT | O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    
    // Set size
    if (ftruncate(shm_fd, sizeof(Queue)) < 0) {
        perror("ftruncate");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    // Map memory
    _ptr_queue = (Queue*)mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, 
                               MAP_SHARED, shm_fd, 0);
    if (_ptr_queue == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    // Initialize queue if it's a new one
    struct stat st;
    if (fstat(shm_fd, &st) == 0 && st.st_size == sizeof(Queue)) {
        // Check if queue is uninitialized (front == 0 and rear == 0 and count == 0)
        if (_ptr_queue->front == 0 && _ptr_queue->rear == 0 && _ptr_queue->count == 0) {
            _ptr_queue->front = 0;
            _ptr_queue->rear = 0;
            _ptr_queue->count = 0;
            memset(_ptr_queue->items, 0, sizeof(_ptr_queue->items));
        }
    }
    
    close(shm_fd);
    
    // Initialize semaphore
    _sem = new SemaphoreCPP(_filename + "_sem");
}

bool WateRMQ::_isEmpty(Queue* q) {
    return q->count == 0;
}

bool WateRMQ::_isFull(Queue* q) {
    return q->count == MAX_SIZE;
}

char* WateRMQ::_peek(Queue* q) {
    if (_isEmpty(q)) {
        return nullptr;
    }
    return q->items[q->front];
}

string WateRMQ::_readMessage() {
    if (!_sem) return "";
    
    _sem->wait();
    char* msg = _peek(_ptr_queue);
    string result;
    if (msg) {
        result = string(msg);
        // Remove from queue
        free(_ptr_queue->items[_ptr_queue->front]);
        _ptr_queue->items[_ptr_queue->front] = nullptr;
        _ptr_queue->front = (_ptr_queue->front + 1) % MAX_SIZE;
        _ptr_queue->count--;
    }
    _sem->post();
    
    return result;
}

void* WateRMQ::_taskConsumer(void* arg) {
    // This function would call the backend C consumer
    // For now, it's a placeholder
    cout << "Consumer thread started" << endl;
    
    // In a real implementation, you would call the backend binary here
    // using execvp or fork + exec
    
    return nullptr;
}

void WateRMQ::setExceptionMessage(const string& loginExceptionMessage, 
                                  const string& openingChannelExceptionMessage,
                                  const string& queueExceptionMessage,
                                  const string& bindingExceptionMessage,
                                  const string& consumingExceptionMessage,
                                  const string& closingChannelExceptionMessage,
                                  const string& closingConnectionExceptionMessage,
                                  const string& endingConnectionExceptionMessage) {
    _command["loginExceptionMessage"] = loginExceptionMessage;
    _command["openingChannelExceptionMessage"] = openingChannelExceptionMessage;
    _command["queueExceptionMessage"] = queueExceptionMessage;
    _command["bindingExceptionMessage"] = bindingExceptionMessage;
    _command["consumingExceptionMessage"] = consumingExceptionMessage;
    _command["closingChannelExceptionMessage"] = closingChannelExceptionMessage;
    _command["closingConnectionExceptionMessage"] = closingConnectionExceptionMessage;
    _command["endingConnectionExceptionMessage"] = endingConnectionExceptionMessage;
}

void WateRMQ::declareExchange(const string& exchange_name, 
                              const string& bindingKey, 
                              const string& queuename) {
    _command["exchange_name"] = exchange_name;
    _command["bindingKey"] = bindingKey;
    _command["queuename"] = queuename;
}

void WateRMQ::connection(const string& hostname, int port) {
    _command["hostname"] = hostname;
    _command["port"] = to_string(port);
}

void WateRMQ::login(const string& v_host, int channel_max, 
                    int frame_max, int heartbeat,
                    const string& username, const string& password) {
    _command["v_host"] = v_host;
    _command["channel_max"] = to_string(channel_max);
    _command["frame_max"] = to_string(frame_max);
    _command["heartbeat"] = to_string(heartbeat);
    _command["username"] = username;
    _command["password"] = password;
}

void WateRMQ::start() {
    _running = true;
    
    // Create consumer thread
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, nullptr, _taskConsumer, this);
    pthread_detach(consumer_thread);
}

void WateRMQ::destroy() {
    _cleanup();
}

string WateRMQ::consumingMessage() {
    return _readMessage();
}
