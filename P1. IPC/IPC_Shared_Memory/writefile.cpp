#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <fstream>

const char* shm_name = "/my_shared_memory";
const int SIZE = 1024 * 1024;  // Kích thước shared memory, ví dụ 1MB

int main() {
    // 1. Mở file để đọc dữ liệu
    std::ifstream file("file_10MB.txt", std::ios::binary);
    if (!file) {
        std::cerr << "Không thể mở file" << std::endl;
        return 1;
    }

    // 2. Tạo hoặc mở shared memory
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    // 3. Đặt kích thước shared memory
    if (ftruncate(shm_fd, SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // 4. Ánh xạ shared memory vào bộ nhớ
    void* ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // 5. Đọc dữ liệu từ file và ghi vào shared memory
    file.read(reinterpret_cast<char*>(ptr), SIZE);
    if (!file) {
        std::cerr << "Không thể đọc hết file" << std::endl;
    }

    std::cout << "File đã được ghi vào shared memory!" << std::endl;

    // Đóng file và giải phóng tài nguyên
    file.close();
    msync(ptr, SIZE, MS_SYNC);  // Đồng bộ dữ liệu vào bộ nhớ
    munmap(ptr, SIZE);          // Giải phóng vùng nhớ
    close(shm_fd);              // Đóng shared memory

    return 0;
}
