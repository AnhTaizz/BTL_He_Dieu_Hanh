#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <chrono>  // Thư viện để đo thời gian

const int BUFFER_SIZE = 1024;  // Kích thước buffer để truyền qua pipe

int main() {
    int pipe_fd[2];  // Pipe file descriptors (pipe_fd[0] là đầu đọc, pipe_fd[1] là đầu ghi)
    
    // Tạo pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        return 1;
    }

    // Tiến trình con
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {  // Tiến trình con (Reader)
        close(pipe_fd[1]);  // Đóng đầu ghi của pipe trong tiến trình con

        // Bắt đầu đo thời gian trong tiến trình con
        auto start = std::chrono::high_resolution_clock::now();

        // Đọc dữ liệu từ pipe và ghi vào file
        std::ofstream outfile("received_file_by_pipe.txt", std::ios::binary);
        if (!outfile) {
            std::cerr << "Không thể mở file để ghi" << std::endl;
            return 1;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
            outfile.write(buffer, bytes_read);
        }

        std::cout << "Dữ liệu đã được ghi vào file 'received_file_by_pipe.txt'" << std::endl;

        // Kết thúc đo thời gian
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Thời gian truyền dữ liệu qua pipe (tiến trình con): " 
                  << duration.count() << " giây" << std::endl;

        outfile.close();
        close(pipe_fd[0]);
        exit(0);
    } else {  // Tiến trình cha (Writer)
        close(pipe_fd[0]);  // Đóng đầu đọc của pipe trong tiến trình cha

        // Mở file cần truyền
        std::ifstream infile("file_10MB.txt", std::ios::binary);
        if (!infile) {
            std::cerr << "Không thể mở file để đọc" << std::endl;
            return 1;
        }

        // Bắt đầu đo thời gian trong tiến trình cha
        auto start = std::chrono::high_resolution_clock::now();

        // Đọc từ file và truyền qua pipe
        char buffer[BUFFER_SIZE];
        while (infile.read(buffer, sizeof(buffer)) || infile.gcount() > 0) {
            write(pipe_fd[1], buffer, infile.gcount());
        }

        std::cout << "Dữ liệu đã được truyền qua pipe" << std::endl;

        // Kết thúc đo thời gian
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Thời gian truyền dữ liệu qua pipe (tiến trình cha): " 
                  << duration.count() << " giây" << std::endl;

        infile.close();
        close(pipe_fd[1]);

        // Chờ tiến trình con kết thúc
        wait(nullptr);
    }

    return 0;
}
