#ifndef _H_SEMAPHORE_H
#define _H_SEMAPHORE_H

#include <semaphore.h>
#include <string>

class SemaphoreCPP {
private:
    sem_t* _sem;
    std::string _sem_name;
    bool _is_named;
    
public:
    // For named semaphore (system semaphore)
    SemaphoreCPP(const std::string& sem_name, bool create = true);
    
    // For unnamed semaphore (shared memory semaphore)
    SemaphoreCPP(sem_t* sem);
    
    ~SemaphoreCPP();
    
    void wait();
    void post();
    void close();
    void unlink();
    
    // Disable copy
    SemaphoreCPP(const SemaphoreCPP&) = delete;
    SemaphoreCPP& operator=(const SemaphoreCPP&) = delete;
};

#endif
