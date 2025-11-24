#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <errno.h>
 
int main() {
    const char* fifo = "/tmp/myfifo";
    mkfifo(fifo, 0666);
 
    std::cout << "Reader: waiting for writer (open for read will BLOCK)...\n";
    int fd = open(fifo, O_RDONLY);   // BLOCK here until writer opens
 
    if (fd == -1) { perror("open"); return 1; }
 
    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    buf[n] = '\0';
 
    std::cout << "Reader received: " << buf;
 
    close(fd);
    return 0;
}