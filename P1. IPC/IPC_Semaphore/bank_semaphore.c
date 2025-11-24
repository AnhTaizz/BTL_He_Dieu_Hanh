#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

int balance = 1000;  // Số dư tài khoản ban đầu
sem_t lock;          // Semaphore dùng để đồng bộ hóa

typedef struct {
    int id;          // ID của thread
    int amount;      // Số tiền muốn rút
} thread_data;

// Hàm rút tiền KHÔNG an toàn (có race condition)
void* unsafe_withdraw(void* arg) {
    thread_data* data = (thread_data*)arg;
    int current = balance;  // Đọc số dư hiện tại
    printf("Luong %d: doc %d, rut %d\n", data->id, current, data->amount);
    
    // Giả lập thời gian xử lý
    usleep(rand() % 300000 + 200000);
    
    // Kiểm tra và cập nhật số dư
    if(current >= data->amount) {
        balance = current - data->amount;
        printf("Luong %d rut %d → con %d\n", data->id, data->amount, balance);
    }
    
    return NULL;
}

// Hàm rút tiền an toàn (có đồng bộ hóa)
void* safe_withdraw(void* arg) {
    thread_data* data = (thread_data*)arg;
    
    sem_wait(&lock);  // Khóa - chờ đến lượt
    int current = balance;
    printf("Luong %d: doc %d, rut %d\n", data->id, current, data->amount);
    
    usleep(rand() % 300000 + 200000);
    
    if(current >= data->amount) {
        balance = current - data->amount;
        printf("Luong %d rut %d → con %d\n", data->id, data->amount, balance);
    }
    sem_post(&lock);  // Mở khóa - cho thread khác vào
    
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t t1, t2;
    
    // Phần 1: Không đồng bộ - có race condition
    printf("--Khong dong bo--\n");
    balance = 1000;
    
    thread_data data1 = {1, rand() % 300 + 200};  // Tạo số tiền ngẫu nhiên
    thread_data data2 = {2, rand() % 300 + 200};
    
    pthread_create(&t1, NULL, unsafe_withdraw, &data1);
    pthread_create(&t2, NULL, unsafe_withdraw, &data2);
    
    pthread_join(t1, NULL);  // Chờ thread kết thúc
    pthread_join(t2, NULL);
    printf("So du: %d\n\n", balance);
    
    // Phần 2: Có đồng bộ - sử dụng semaphore
    printf("--Co dong bo--\n");
    balance = 1000;
    sem_init(&lock, 0, 1);  // Khởi tạo semaphore = 1
    
    thread_data data3 = {1, rand() % 300 + 200};
    thread_data data4 = {2, rand() % 300 + 200};
    
    pthread_create(&t1, NULL, safe_withdraw, &data3);
    pthread_create(&t2, NULL, safe_withdraw, &data4);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    sem_destroy(&lock);  // Hủy semaphore
    
    printf("So du: %d\n", balance);
    
    return 0;
}