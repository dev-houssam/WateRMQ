// SemaphoreCPP.hpp
#ifndef _H_SEMAPHORE_H
#define _H_SEMAPHORE_H

#include <semaphore.h>
#include <string>
#include <cstdio>
#include <cstdlib>

class SemaphoreCPP {
private:
    sem_t* _sem;
    std::string _sem_name;
    bool _is_named;  // named vs unnamed semaphore
    
public:
    // Pour named semaphore (sémaphores système)
    SemaphoreCPP(const std::string& sem_name, bool create = true);
    
    // Pour unnamed semaphore (sémaphores en mémoire partagée)
    SemaphoreCPP(sem_t* sem);
    
    ~SemaphoreCPP();
    
    void wait();
    void post();
    void close();
    void unlink();
};

#endif
