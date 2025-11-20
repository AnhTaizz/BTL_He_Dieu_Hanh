#include <iostream>
#include <fcntl.h>         
#include <sys/mman.h>      
#include <sys/stat.h>      
#include <unistd.h>        
#include <semaphore.h>     
#include <cstring>

int main() {
   const char* shm_name = "/my_shared_memory";   // tên shared memory
   const int SIZE = 1024;                        // kích thước vùng nhớ
   // 1. Tạo file shared memory
   int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
   // 2. Cấu hình kích thước
   ftruncate(shm_fd, SIZE);
   // 3. Ánh xạ shared memory vào địa chỉ RAM
   void* ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
   // 4. Tạo semaphore
   sem_t* sem = sem_open("/my_sem", O_CREAT, 0666, 0);
   // 5. Ghi dữ liệu
   const char* message = "Xin chao \nDay la tin nhan duoc ghi vao shared memory";
   memcpy(ptr, message, strlen(message) + 1);
   // 6. Báo cho reader biết dữ liệu đã sẵn sàng
   sem_post(sem);
   std::cout << "Writer: Da ghi vào shared memory thanh cong.\n";
   return 0;
}
