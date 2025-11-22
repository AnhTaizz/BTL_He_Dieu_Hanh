#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define LOG_ERROR 3

struct logMessage {
    long msg_type;
    char text[256];
};

int main() {
    key_t key = ftok("mqfile", 65);
    int mqid = msgget(key, 0666 | IPC_CREAT);

    struct logMessage msg;

    printf("Server đang chờ ERROR...\n");

    while (1) {
        if (msgrcv(mqid, &msg, sizeof(msg) - sizeof(long), LOG_ERROR, 0) >= 0) {
            printf("ERROR: %s\n", msg.text);
        }
    }
}
