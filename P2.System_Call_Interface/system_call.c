#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>


int main() {

    int fd = 1;
    const char *message = "Hello, World!\n";
    write(fd, message, strlen(message));
    return 0;
}
