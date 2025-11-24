#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <fstream>
#include <chrono>  // Thư viện cho đo thời gian

const char* shm_name = "/my_shared_memory";
const int SIZE = 1024 * 1024;  // Kích thước shared memory (phải giống với tiến trình ghi)

int main() {
    // 1. Mở shared memory đã tồn tại
    int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    // 2. Ánh xạ shared memory vào bộ nhớ
    void* ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // 3. Bắt đầu đo thời gian
    auto start = std::chrono::high_resolution_clock::now();

    // Hoặc bạn có thể ghi vào file
    std::ofstream outfile("received_file.txt", std::ios::binary);
    if (!outfile) {
        std::cerr << "Không thể mở file để ghi" << std::endl;
        return 1;
    }
    outfile.write(reinterpret_cast<char*>(ptr), SIZE);
    outfile.close();

    std::cout << "Dữ liệu đã được ghi vào file 'received_file_by_shared_memory.txt'" << std::endl;

    // 5. Kết thúc đo thời gian
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // 6. In ra thời gian thực thi
    std::cout << "Thời gian truyền dữ liệu: " << duration.count() << " giây" << std::endl;

    // Giải phóng tài nguyên
    munmap(ptr, SIZE);
    close(shm_fd);

    return 0;
}
