#include <stdio.h>
#include <unistd.h>
#include <signal.h>

// Co bao hieu SIGINT
volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    write(1, "\nNhan Ctrl+C, thoat an toan...\n", 31);
    stop = 1; // Thoat loop
}

int main() {
    // Dang ki handle cho SIGINT
    signal(SIGINT, handle_sigint);
    int counter = 0;
    while (!stop) {
        printf("Hello World! Giay thu %d\n", ++counter);
        fflush(stdout);    // Chac chan in
        sleep(1);        
    }
    printf("[Main] Thoat chuong trinh.\n");
    return 0;
}
