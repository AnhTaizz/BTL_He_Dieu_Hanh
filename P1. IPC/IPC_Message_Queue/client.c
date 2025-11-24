#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define LOG_INFO     1
#define LOG_WARNING  2
#define LOG_ERROR    3
#define LOG_DEBUG    4

struct logMessage {
    long msg_type;
    char text[256];
};

void send_log(int mqid, long type, const char* msg) {
    struct logMessage log;
    log.msg_type = type;
    strcpy(log.text, msg);
    msgsnd(mqid, &log, sizeof(log) - sizeof(long), 0);
}

int main() {
    key_t key = ftok("mqfile", 65);
    int mqid = msgget(key, 0666 | IPC_CREAT);

    srand(time(NULL));

    printf("Nhấn Ctrl + C để dừng.\n\n");

    while (1) {
        int r = rand() % 100;   // random số 0–99
        int type;
        char msg[200];

        if (r < 5) {  
            // 5% ERROR
            type = LOG_ERROR;
            strcpy(msg, "ERROR: Lỗi kết nối database, retry failed");
        }
        else if (r < 20) {  
            // 15% WARNING
            type = LOG_WARNING;
            strcpy(msg, "WARNING: CPU usage vượt 80%");
        }
        else if (r < 60) {  
            // 40% INFO
            type = LOG_INFO;
            strcpy(msg, "INFO: Người dùng truy cập hệ thống");
        }
        else {  
            // 40% DEBUG
            type = LOG_DEBUG;
            strcpy(msg, "DEBUG: Giá trị biến counter đang tăng");
        }

        send_log(mqid, type, msg);

        printf("Gửi -> %s\n", msg);

        // Random thời gian gửi giống hệ thống 
        usleep((rand() % 1500 + 500) * 1000); // 0.5s - 2.0s
    }

    return 0;
}
