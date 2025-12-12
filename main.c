
typedef struct { 
    int age;
    char name[128];
} person;



int main() {
    // Open a file
    int fd = open("data.person", O_RDWR | O_CREAT | S_IRWXU);
    // Check if the file exists 
    struct stat st; fstat(fd, &st);
    person *p;
    if (st.st_size != 0) {
        // New file: Create and write initial data
        ftruncate(fd, sizeof(person));
        p = (person) mmap(0, sizeof(person), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
        strcpy(p[0].name, "Hello");
        p[0].age = 10;
        msync(p, sizeof(person), MS_SYNC);
    } else {
        // Existing file: Try to read data
        p = (person *) mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    }
    printf("%s %d\n", p->name, p->age);
    munmap(p, sizeof(person));
    close(fd);

    return 0;
}
