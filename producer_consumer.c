#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 5  // Kích thước buffer

int buf[N], in = 0, out = 0;    // Buffer và chỉ số đọc/ghi
sem_t empty, full, mutex;       // Semaphore đồng bộ hóa

// Hàm in trạng thái buffer
void in_buf() {
    printf("Buf: [");
    for(int i = 0; i < N; i++) {
        if (buf[i] != 0 ) printf("%d", buf[i]);  // In số nếu có dữ liệu
        else printf("_");                        // In gạch nếu trống
        if(i < N-1) printf(" ");
    }
    printf("]\n");
}

// Hàm producer - nhà sản xuất
void* sx(void* arg) {
    int id = *(int*)arg;  // ID của producer
    for(int i = 0; i < 3; i++) {
        int sp = rand() % 100;  // Tạo sản phẩm ngẫu nhiên
        
        sem_wait(&empty);  // Chờ nếu buffer đầy
        sem_wait(&mutex);  // Khóa buffer
        
        // Thêm dữ liệu vào buffer
        buf[in] = sp;
        printf("SX%d: +%d ind: %d - ", id, sp, in);
        in_buf();
        in = (in + 1) % N;  // Di chuyển chỉ số (vòng tròn)
        
        sem_post(&mutex);  // Mở khóa buffer
        sem_post(&full);   // Báo hiệu có dữ liệu mới
        
        sleep(1);  // Giả lập thời gian xử lý
    }
    return NULL;
}

// Hàm consumer - người tiêu thụ
void* td(void* arg) {
    int id = *(int*)arg;  // ID của consumer
    for(int i = 0; i < 3; i++) {
        sem_wait(&full);   // Chờ nếu buffer rỗng
        sem_wait(&mutex);  // Khóa buffer
        
        // Lấy dữ liệu từ buffer
        int sp = buf[out];
        buf[out] = 0;      // Đánh dấu đã xử lý
        printf("TD%d: -%d ind: %d - ", id, sp, out);
        in_buf();
        out = (out + 1) % N;  // Di chuyển chỉ số (vòng tròn)
        
        sem_post(&mutex);  // Mở khóa buffer
        sem_post(&empty);  // Báo hiệu có chỗ trống
        
        sleep(1);  // Giả lập thời gian xử lý
    }
    return NULL;
}

int main() {
    pthread_t p[2], c[2];  // Thread producers và consumers
    int id[2] = {1, 2};    // ID các thread
    
    // Khởi tạo semaphore
    sem_init(&empty, 0, N);  // Ban đầu có N chỗ trống
    sem_init(&full, 0, 0);   // Ban đầu có 0 dữ liệu
    sem_init(&mutex, 0, 1);  // Khóa nhị phân
    
    // Khởi tạo buffer rỗng
    for(int i = 0; i < N; i++) buf[i] = 0;
    
    // Tạo các thread
    pthread_create(&p[0], NULL, sx, &id[0]);  // Producer 1
    pthread_create(&p[1], NULL, sx, &id[1]);  // Producer 2
    pthread_create(&c[0], NULL, td, &id[0]);  // Consumer 1
    pthread_create(&c[1], NULL, td, &id[1]);  // Consumer 2
    
    // Chờ tất cả thread kết thúc
    pthread_join(p[0], NULL);
    pthread_join(p[1], NULL);
    pthread_join(c[0], NULL);
    pthread_join(c[1], NULL);
    
    // Hủy semaphore
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    return 0;
}