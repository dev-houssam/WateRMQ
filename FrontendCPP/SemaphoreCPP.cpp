#include <semaphore.h>
#include <cstdlib>
#include <string>

#include <fcntl.h>  // For O_* constants
#include <sys/stat.h>  // For mode constants

#include "SemaphoreCPP.hpp"

using namespace std;

SemaphoreCPP::SemaphoreCPP(){}

SemaphoreCPP::SemaphoreCPP(const std::string & sem_name){
    this->_sem_name = sem_name;
    semOpen();
    semInit();
}

void SemaphoreCPP::semOpen(){
    char * sem_name = this->_sem_name.data();
    _sem = sem_open(sem_name, O_RDWR, 0644);
    
    if (_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

void SemaphoreCPP::semInit(){
    sem_init(_sem, _p1, _p2);
}

void SemaphoreCPP::semWait(){
    sem_wait(_sem);
}

void SemaphoreCPP::semPost(){
    sem_post(_sem);
}
void SemaphoreCPP::semClose(){
    // This method must be executed before sem_unlink
    sem_close(_sem);
}

void SemaphoreCPP::semUnlink(){
    std::string str;
    char * sem_name = this->_sem_name.data();
    sem_unlink(sem_name);
}
void SemaphoreCPP::semDestroy(){
    sem_destroy(_sem);
}