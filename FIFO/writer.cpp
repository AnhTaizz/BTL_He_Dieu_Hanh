#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
 
int main() {
    const char* fifo = "/tmp/myfifo";
    mkfifo(fifo, 0666);
 
    std::cout << "Writer: waiting for reader (open for write will BLOCK)...\n";
    int fd = open(fifo, O_WRONLY);   // BLOCK here until reader opens
 
    if (fd == -1) { perror("open"); return 1; }
 
    const char* msg = "Hello from writer\n";
    write(fd, msg, strlen(msg));
 
    close(fd);
    return 0;
}
 