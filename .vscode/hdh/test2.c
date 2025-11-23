#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t pid;

    // 1. Tạo pipe
    // pipefd[0] là đầu đọc (read end)
    // pipefd[1] là đầu ghi (write end)
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 2. Fork process
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // --- CHILD PROCESS (Đóng vai trò là 'ls') ---
        
        // Đóng đầu đọc vì Child chỉ cần ghi dữ liệu
        close(pipefd[0]);
        // Redirect (Chuyển hướng) STDOUT sang đầu ghi của Pipe
        // Mọi thứ lệnh 'ls' in ra màn hình giờ sẽ chui vào ống pipe
        dup2(pipefd[1], STDOUT_FILENO);
        
        // Đóng đầu ghi gốc sau khi đã duplicate (quy tắc an toàn)
        close(pipefd[1]);

        // Thực thi lệnh 'ls'
        execlp("ls", "ls", NULL);
        perror("execlp ls");
        exit(EXIT_FAILURE);
    } else {
        // --- PARENT PROCESS (Đóng vai trò là 'grep') ---

        // Đóng đầu ghi vì Parent chỉ cần đọc dữ liệu
        close(pipefd[1]);

        // Redirect (Chuyển hướng) STDIN lấy từ đầu đọc của Pipe
        // Lệnh 'grep' sẽ đọc dữ liệu từ ống pipe thay vì bàn phím
        dup2(pipefd[0], STDIN_FILENO);

        // Đóng đầu đọc gốc
        close(pipefd[0]);

        // Thực thi lệnh 'grep' tìm chuỗi ".c"
        execlp("grep", "grep", ".c", NULL);
        perror("execlp grep");
        exit(EXIT_FAILURE);
    }

    return 0;
}