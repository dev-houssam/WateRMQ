#include "SemaphoreCPP.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>

SemaphoreCPP::SemaphoreCPP(const std::string& sem_name, bool create) 
    : _sem_name(sem_name), _is_named(true) {
    if (create) {
        _sem = sem_open(sem_name.c_str(), O_CREAT | O_RDWR, 0644, 0);
    } else {
        _sem = sem_open(sem_name.c_str(), O_RDWR);
    }
    
    if (_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

SemaphoreCPP::SemaphoreCPP(sem_t* sem) : _sem(sem), _is_named(false) {}

SemaphoreCPP::~SemaphoreCPP() {
    if (_is_named && _sem != SEM_FAILED) {
        sem_close(_sem);
    }
}

void SemaphoreCPP::wait() {
    sem_wait(_sem);
}

void SemaphoreCPP::post() {
    sem_post(_sem);
}

void SemaphoreCPP::close() {
    if (_is_named) {
        sem_close(_sem);
    }
}

void SemaphoreCPP::unlink() {
    if (_is_named) {
        sem_unlink(_sem_name.c_str());
    }
}
