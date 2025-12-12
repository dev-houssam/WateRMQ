#ifndef _H_WATERMQ_H
#define _H_WATERMQ_H


//#include "Semaphore.h"
#include <cstring>
#include <cstdio>
#include <thread>
#include <cstddef>
#include <string>


#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>  // For O_* constants
#include <sys/stat.h>  // For mode constants
#include <stdlib.h>

#include "SemaphoreCPP.hpp"

#include <map>


using namespace std;


#define MAX_SIZE 4096*10

typedef struct {
    char*  items[MAX_SIZE];
    int front;
    int rear;
} Queue;


typedef struct {
    string filename;
    size_t length;
    void * addr; 
    int prot; 
    int flags;
    int fd; 
    off_t offset;
    struct stat st;
    Queue * ptr_queue; /* Pointe sur la queue utilisée */ 
} QueueMmap;



class WateRMQ {
    private:
	    QueueMmap *     _qm; 
        SemaphoreCPP    _sem;
        Queue *         _ptr_queue;
        string          _filename;
        map<std::string, std::string> command;

        bool _isEmpty(Queue* q);

        char * _peek(Queue* q);
        
        string _readMessage();

        void _init();

        static void _taskConsumer(map<std::string, std::string> command_);


	public:
        WateRMQ();
        WateRMQ(const string & filename);
        WateRMQ(const WateRMQ&);

        void setExceptionMessage(
    		    const string & loginExceptionMessage, 
    		    const string & openingChannelExceptionMessage, 
    		    const string & queueExceptionMessage, 
    		    const string & bindingExceptionMessage, 
    		    const string & consumingExceptionMessage, 
    		    const string & closingChannelExceptionMessage, 
    		    const string & closingConnectionExceptionMessage, 
    		    const string & endingConnectionExceptionMessage);

        void declareExchange(const string & exchange_name, const string & bindingKey, const string & queuename);

        void connection(const string & hostname, int port);

        void login(const string & v_host, int channel_max, int frame_max, int heartbeat, const string & username, const string & password);

        void start();

        void destroy();

        string consumingMessage();
};

#endif
