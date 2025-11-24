#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// Du Child co Handler nay, SIGKILL se bo qua no
    // Nếu Parent gửi SIGINT (tín hiệu 2), hàm này sẽ được gọi.
void signal_handle(int sig) {
    printf("[Child] Nhan duoc SIGINT (%d) -> Dang thoat...\n", sig);
    exit(0);
}

int main() {
    pid_t pid = fork();

    if (pid == 0) {  
        // CHILD
        printf("[Child] PID = %d\n", getpid());

        signal(SIGINT, signal_handle);   // Chỉ handle SIGINT

        while (1) {
            printf("[Child] Dang chay...\n");
            sleep(1);
        }
    } 
    else {  
        // PARENT
        
        printf("[Parent] Child PID = %d\n", pid);
        printf("[Parent] Cho 4 giay truoc khi gui SIGKILL...\n");
        sleep(4);

        // Gui SIGKILL, lenh nay khong the bi chan du co signal_handle cua Child
        printf("[Parent] Gui SIGKILL toi Child...\n");
        kill(pid, SIGKILL);
        
        // wait() là System Call để Parent chờ Child và tránh Zombie process.
        wait(NULL);
        printf("[Parent] Child da bi kill. Parent thoat.\n");
    }

    return 0;
}
