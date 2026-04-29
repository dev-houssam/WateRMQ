#ifndef _H_WATERMQ_H
#define _H_WATERMQ_H

#include "SemaphoreCPP.hpp"
#include <map>
#include <string>
#include <vector>

#define MAX_SIZE 4096 * 10

typedef struct Queue {
    char* items[MAX_SIZE];
    int front;
    int rear;
    int count;
} Queue;

class WateRMQ {
private:
    SemaphoreCPP* _sem;
    Queue* _ptr_queue;
    std::string _filename;
    std::map<std::string, std::string> _command;
    bool _running;
    
    bool _isEmpty(Queue* q);
    bool _isFull(Queue* q);
    char* _peek(Queue* q);
    std::string _readMessage();
    void _init();
    static void* _taskConsumer(void* arg);
    void _cleanup();

public:
    WateRMQ(const std::string& filename);
    ~WateRMQ();
    
    // Disable copy
    WateRMQ(const WateRMQ&) = delete;
    WateRMQ& operator=(const WateRMQ&) = delete;
    
    void setExceptionMessage(const std::string& loginExceptionMessage,
                            const std::string& openingChannelExceptionMessage,
                            const std::string& queueExceptionMessage,
                            const std::string& bindingExceptionMessage,
                            const std::string& consumingExceptionMessage,
                            const std::string& closingChannelExceptionMessage,
                            const std::string& closingConnectionExceptionMessage,
                            const std::string& endingConnectionExceptionMessage);
    
    void declareExchange(const std::string& exchange_name, 
                        const std::string& bindingKey, 
                        const std::string& queuename);
    
    void connection(const std::string& hostname, int port);
    
    void login(const std::string& v_host, int channel_max, 
               int frame_max, int heartbeat,
               const std::string& username, const std::string& password);
    
    void start();
    void destroy();
    std::string consumingMessage();
};

#endif
