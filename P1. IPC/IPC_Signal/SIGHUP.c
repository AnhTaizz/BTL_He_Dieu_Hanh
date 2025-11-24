#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

volatile sig_atomic_t reload_config = 0;

// Handler cho SIGHUP
void handle_sighup(int sig) {
    const char* msg = "\nNhan SIGHUP! Tai lai cau hinh...\n";
    write(1, msg, strlen(msg));
    reload_config = 1;
}

int main() {
    signal(SIGHUP, handle_sighup);  // Dang ky handler

    printf("Demo Web Server (gia) PID: %d\n", getpid());
    printf("Gui SIGHUP de reload config. Ctrl+C de thoat.\n");

    int counter = 0;
    while(1) {
        printf("[Server] Dang phuc vu yeu cau... Lan thu %d\n", ++counter);
        fflush(stdout);

        if(reload_config) {
            printf("[Server] Tai lai cau hinh xong.\n");
            reload_config = 0;
        }

        sleep(2);
    }

    return 0;
}
