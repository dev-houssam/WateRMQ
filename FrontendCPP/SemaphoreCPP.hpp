#ifndef _H_SEMAPHORE_H
#define _H_SEMAPHORE_H


#include <semaphore.h>
#include <cstdlib>
#include <string>

#include <fcntl.h>  // For O_* constants
#include <sys/stat.h>  // For mode constants


using namespace std;

typedef struct {
    char * _sem_name;
    int oflag;
    mode_t mode;
    unsigned int value; 
    sem_t * semaphore_self;
} SemaphoreMmap;



class SemaphoreCPP {
    private:
        sem_t * _sem;
        int _p1 = 0;    
        int _p2 = 1;
        string _sem_name;
    public :
    SemaphoreCPP();
    SemaphoreCPP(const string & sem_name);
    
    void semOpen();

    void semInit();

    void semWait();

    void semPost();
    void semClose();

    void semUnlink();
    void semDestroy();

};

#endif