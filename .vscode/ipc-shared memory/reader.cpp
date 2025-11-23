#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

int main() {
   const char* shm_name = "/my_shared_memory";
   const int SIZE = 1024;
   // 1. Mở shared memory đã được tạo trước đó
   int shm_fd = shm_open(shm_name, O_RDONLY, 0666);
   // 2. Ánh xạ shared memory vào RAM
   void* ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
   // 3. Mở semaphore có sẵn
   sem_t* sem = sem_open("/my_sem", 0);
   // 4. Chờ writer gửi tín hiệu
   sem_wait(sem);
   // 5. Đọc dữ liệu
   std::cout << "Reader nhận: " << (char*)ptr << std::endl;
   return 0;
}
