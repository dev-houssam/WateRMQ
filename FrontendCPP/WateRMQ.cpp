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

#include "WateRMQ.hpp"

using namespace std;


#define MAX_SIZE 4096*10

WateRMQ::WateRMQ(){}
WateRMQ::WateRMQ(const WateRMQ&){}

WateRMQ::WateRMQ(const string & filename){
	std::string str;
    //char * filename_ = ;
	this->_filename = filename;
    // Nous n'avons  pas besoin de confirmer que je  l'ai lu
    this->_init();
}



void WateRMQ::setExceptionMessage(
	    const string & loginExceptionMessage, 
	    const string & openingChannelExceptionMessage, 
	    const string & queueExceptionMessage, 
	    const string & bindingExceptionMessage, 
	    const string & consumingExceptionMessage, 
	    const string & closingChannelExceptionMessage, 
	    const string & closingConnectionExceptionMessage, 
	    const string & endingConnectionExceptionMessage){
		this->command["loginExceptionMessage"] 				= loginExceptionMessage;
        this->command["openingChannelExceptionMessage"] 	= openingChannelExceptionMessage;
        this->command["queueExceptionMessage"] 				= queueExceptionMessage;
        this->command["bindingExceptionMessage"] 			= bindingExceptionMessage; 
        this->command["consumingExceptionMessage"] 			= consumingExceptionMessage; 
        this->command["closingChannelExceptionMessage"] 	= closingChannelExceptionMessage; 
        this->command["closingConnectionExceptionMessage"] 	= closingConnectionExceptionMessage; 
        this->command["endingConnectionExceptionMessage"] 	= endingConnectionExceptionMessage;

        // Exception ::

        /*
		--except_message 
		em::this->command["loginExceptionMessage"]
		em::this->command["openingChannelExceptionMessage"]
		em::this->command["queueExceptionMessage"]
		em::this->command["bindingExceptionMessage"]
		em::this->command["consumingExceptionMessage"]
		em::this->command["closingChannelExceptionMessage"]
		em::this->command["closingConnectionExceptionMessage"]
		em::this->command["endingConnectionExceptionMessage"]
		*/
}



void WateRMQ::declareExchange(const string & exchange_name, const string & bindingKey, const string & queuename){
	this->command["exchange_name"] = exchange_name;
    this->command["bindingKey"]    = bindingKey;
    this->command["queuename"] 	   = queuename;

    /*
	--exchange_bind_queue
	ebq::this->command["exchange_name"]
	ebq::this->command["bindingKey"]
	ebq::this->command["queuename"]
	*/
}




void WateRMQ::connection(const string & hostname, int port){
	//configurationConnection_WateRQM_public :  Host : example : localhost 15672
	this->command["hostname"] = hostname;
    this->command["port"] 	  = port;

    /*
	--connection
	chp::this->command["hostname"]
	chp::this->command["port"]
	*/
}



void WateRMQ::login(const string & v_host, int channel_max, int frame_max, int heartbeat, const string & username, const string & password){
	//configurationLogin_WateRQM_public(stub, v_host, channel_max, frame_max, heartbeat, username, password);
	this->command["v_host"] 	= v_host; 
    this->command["channel_max"]= channel_max; 
    this->command["frame_max"] 	= frame_max;
    this->command["heartbeat"] 	= heartbeat;
    this->command["username"] 	= username; 
    this->command["password"] 	= password;

    /*
	--login_system
	ls::this->command["v_host"]
	ls::this->command["channel_max"]
	ls::this->command["frame_max"]
	ls::this->command["heartbeat"]
	ls::this->command["username"]
	ls::this->command["password"]
	*/
}



void WateRMQ::_taskConsumer(map<std::string, std::string> command_){
	//zenity --info --text="I am a Consumer"
     execl("/usr/bin/zenity", 
     	"--connection", 
     		"chp::" + command_["hostname"],
     		"chp::" + command_["port"],
     	"--exchange_bind_queue",
     		"ebq::" + command_["exchange_name"],
			"ebq::" + command_["bindingKey"],
			"ebq::" + command_["queuename"],
		"--login_system",
			"ls::"  + command_["v_host"],
			"ls::"  + command_["channel_max"],
			"ls::"  + command_["frame_max"],
			"ls::"  + command_["heartbeat"],
			"ls::"  + command_["username"],
			"ls::"  + command_["password"],
		"--except_message",
			"em::"  + command_["loginExceptionMessage"],
			"em::"  + command_["openingChannelExceptionMessage"],
			"em::"  + command_["queueExceptionMessage"],
			"em::"  + command_["bindingExceptionMessage"],
			"em::"  + command_["consumingExceptionMessage"],
			"em::"  + command_["closingChannelExceptionMessage"],
			"em::"  + command_["closingConnectionExceptionMessage"],
			"em::"  + command_["endingConnectionExceptionMessage"],
     	(char *) 0);
}

/*

void WateRMQ::_taskConsumer(map<std::string, std::string> command_){
	//zenity --info --text="I am a Consumer"
     execl("/usr/bin/zenity", 
     	"--connection", 
     		"chp::", this->command["hostname"],
     		"chp::", this->command["port"],
     	"--exchange_bind_queue",
     		"ebq::", this->command["exchange_name"],
			"ebq::", this->command["bindingKey"],
			"ebq::", this->command["queuename"],
		"--login_system",
			"ls::",  this->command["v_host"],
			"ls::",  this->command["channel_max"],
			"ls::",  this->command["frame_max"],
			"ls::",  this->command["heartbeat"],
			"ls::",  this->command["username"],
			"ls::",  this->command["password"],
		"--except_message",
			"em::",  this->command["loginExceptionMessage"],
			"em::",  this->command["openingChannelExceptionMessage"],
			"em::",  this->command["queueExceptionMessage"],
			"em::",  this->command["bindingExceptionMessage"],
			"em::",  this->command["consumingExceptionMessage"],
			"em::",  this->command["closingChannelExceptionMessage"],
			"em::",  this->command["closingConnectionExceptionMessage"],
			"em::",  this->command["endingConnectionExceptionMessage"],
     	(char *) 0);
}

*/


void WateRMQ::_init(){
	cout << "Debut de test"<<endl;
	int shm_fd;
	int cpt = 0;
	while(true){
		//stub = _init_WateRQM_public();
		shm_fd = shm_open(this->_filename.data(), O_RDWR, 0644);
	    if (shm_fd < 0) {
	        perror("shm_open : on init");
	        
	    }
	    if (cpt >= 10)
	    {
	    	cout << "Tentative : ECHEC" <<endl;
	    	exit(EXIT_FAILURE);
	    }
	    cpt += 1;
	    cout << "Tentative : " << cpt <<endl;
	    sleep(1);
    }
    this->_ptr_queue = (Queue *) mmap(NULL, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (this->_ptr_queue == MAP_FAILED) {
        perror("mmap : failed on init ");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
	cout << "init" << endl;
}




void WateRMQ::start(){
	cout << "START de test"<<endl;
	std::thread t(this->_taskConsumer, this->command);
    //t.join(); // Wait for the thread to finish
    t.detach();
}

void WateRMQ::destroy(){
	cout << "DESTROY de test"<<endl;

    //munmap(mapped, length);
    //close(fd);
    printf("--- Test Finished ---\n");
}

bool WateRMQ::_isEmpty(Queue* q) { 
	return (q->front == q->rear - 1); 
}

char * WateRMQ::_peek(Queue* q)
{
    if (this->_isEmpty(q)) {
        printf("Queue is empty\n");
        return nullptr; // return some default value or handle
                   // error differently
    }
    return q->items[q->front + 1];
}

string WateRMQ::_readMessage(){
    //sem->wait();
        string messageIn = this->_peek(this->_ptr_queue);
    //sem->post();
    return messageIn;
}

string WateRMQ::consumingMessage(){
	return this->_readMessage();
}