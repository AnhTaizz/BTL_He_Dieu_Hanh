#include <iostream>
#include <unistd.h>     // Thư viện cho pipe, fork, read, write
#include <cstring>      // Thư viện cho strlen
#include <sys/wait.h>   // Thư viện cho wait

using namespace std;

int main() {
    int fd[2]; 
    // fd[0] - Dùng để ĐỌC (Read end)
    // fd[1] - Dùng để GHI (Write end)

    char buffer[100];
    pid_t pid;

    // 1. Tạo Pipe truoc tien trinh fork
    if (pipe(fd) == -1) {
        cerr << "Lỗi tạo Pipe" << endl;
        return 1;
    }

    // 2. Fork tiến trình
    pid = fork();

    if (pid < 0) {
        cerr << "Lỗi Fork" << endl;
        return 1;
    }

    // --- TIẾN TRÌNH CON (Người nhận) ---
    if (pid == 0) {
        close(fd[1]); // Con chỉ đọc, nên đóng đầu Ghi

        cout << "Con: Đang chờ tin nhắn từ Cha..." << endl;

        // Đọc dữ liệu từ ống
        // read() sẽ block (dừng chờ) cho đến khi có dữ liệu
        int nbytes = read(fd[0], buffer, sizeof(buffer));
        
        if (nbytes > 0) {
            buffer[nbytes] = '\0'; 
            cout << "Con: Đã nhận được -> " << buffer << endl;
        }

        // Đọc xong thì đóng nốt đầu Đọc
        close(fd[0]); 
    } 
    
    // --- TIẾN TRÌNH CHA (Người gửi) ---
    else {
        // Quy tắc quan trọng: Đóng đầu ống không dùng đến
        close(fd[0]); // Cha chỉ gửi, nên đóng đầu Đọc

        const char* msg = "loi chao tu Cha!";
        
        cout << "Cha: Đang gửi tin nhắn..." << endl;
        
        // Ghi dữ liệu vào ống
        write(fd[1], msg, strlen(msg));

        // Ghi xong thì đóng đầu Ghi.
        // Việc đóng này gửi tín hiệu EOF cho Child để Child biết là hết dữ liệu.
        close(fd[1]); 

        // Chờ con kết thúc
        wait(nullptr);
    }

    return 0;
}