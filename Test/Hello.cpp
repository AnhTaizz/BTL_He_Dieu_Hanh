// demo.c
// Demo nhiều cơ chế IPC: pipe, signal, shared memory + semaphore (producer/consumer),
// message queue, FIFO (named pipe).
// Biên dịch: gcc TenDemo.c -o demo -pthread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/wait.h>

#define SHM_KEY 1234
#define MSG_KEY 5678
#define SHM_SIZE 1024
#define FIFO_PATH "myfifo"
#define SEM_NAME "/mysem_demo"

// Global flag for signal demo
volatile sig_atomic_t sigint_received = 0;

/* ---------------- SIGNAL ---------------- */
void handle_sigint(int signum) {
    (void)signum;
    sigint_received = 1;
}

void demo_signal() {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("[SIGNAL] sigaction");
        return;
    }

    printf("[SIGNAL] Handler đã được cài đặt. Nhấn Ctrl+C để test.\n");
    while (!sigint_received) {
        printf("[SIGNAL] Đang chạy...\n");
        sleep(1);
    }
    printf("[SIGNAL] Nhận SIGINT → Thoát.\n");
}

/* ---------------- PIPE ---------------- */
void demo_pipe() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("[PIPE] pipe");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("[PIPE] fork");
        return;
    } else if (pid == 0) { // child = consumer
        close(pipefd[1]);
        char buffer[128] = {0};
        ssize_t r = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (r < 0) perror("[PIPE - CHILD] read");
        else printf("[PIPE - CHILD] Nhận: %s\n", buffer);
        close(pipefd[0]);
        _exit(0);
    } else { // parent = producer
        close(pipefd[0]);
        const char msg[] = "Hello từ PIPE!";
        if (write(pipefd[1], msg, sizeof(msg)) == -1) perror("[PIPE - PARENT] write");
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
    }
}

/* ---------------- SHM + SEM (Producer / Consumer) ----------------
   - Producer: tạo shm và semaphore (khởi tạo sem value = 0), ghi dữ liệu, sem_post
   - Consumer: mở shm và semaphore, sem_wait, đọc, dọn dẹp (shmctl IPC_RMID, sem_unlink)
*/
void demo_shm_producer() {
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("[SHM-PROD] shmget");
        return;
    }

    void *shm = shmat(shm_id, NULL, 0);
    if (shm == (void *) -1) {
        perror("[SHM-PROD] shmat");
        // nếu muốn xóa shm (không chắc chắn), có thể shmctl ở đây, nhưng để consumer xóa an toàn hơn
        return;
    }

    // Tạo semaphore với giá trị 0 (consumer sẽ block đến khi producer post)
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("[SHM-PROD] sem_open");
        shmdt(shm);
        return;
    }

    // Ghi dữ liệu vào SHM
    snprintf((char *)shm, SHM_SIZE, "Xin chào từ shared memory (PID: %d)!", getpid());
    printf("[SHM-PROD] Ghi vào shared memory: \"%s\"\n", (char *)shm);

    // Thông báo consumer
    if (sem_post(sem) == -1) perror("[SHM-PROD] sem_post");

    // Dọn dẹp local handles (consumer sẽ xóa shm và unlink sem sau khi đọc)
    if (sem_close(sem) == -1) perror("[SHM-PROD] sem_close");
    if (shmdt(shm) == -1) perror("[SHM-PROD] shmdt");

    printf("[SHM-PROD] Hoàn tất. Consumer nên chạy sau để đọc.\n");
}

void demo_shm_consumer() {
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id == -1) {
        perror("[SHM-CONS] shmget (moi co khi shm chua duoc tao)");
        return;
    }

    void *shm = shmat(shm_id, NULL, 0);
    if (shm == (void *) -1) {
        perror("[SHM-CONS] shmat");
        return;
    }

    // Mở semaphore (nếu chưa tồn tại, retry)
    sem_t *sem = sem_open(SEM_NAME, 0);
    int retries = 0;
    while (sem == SEM_FAILED && retries < 50) { // chờ tối đa ~5s
        if (errno != ENOENT) break;
        usleep(100000);
        sem = sem_open(SEM_NAME, 0);
        retries++;
    }
    if (sem == SEM_FAILED) {
        perror("[SHM-CONS] sem_open");
        shmdt(shm);
        return;
    }

    // Chờ producer post
    if (sem_wait(sem) == -1) {
        perror("[SHM-CONS] sem_wait");
        sem_close(sem);
        shmdt(shm);
        return;
    }

    printf("[SHM-CONS] Đọc từ SHM: \"%s\"\n", (char *)shm);

    // Dọn dẹp: detach, xóa SHM, close/unlink semaphore
    if (shmdt(shm) == -1) perror("[SHM-CONS] shmdt");
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) perror("[SHM-CONS] shmctl(IPC_RMID)");
    if (sem_close(sem) == -1) perror("[SHM-CONS] sem_close");
    if (sem_unlink(SEM_NAME) == -1) perror("[SHM-CONS] sem_unlink");

    printf("[SHM-CONS] Hoàn tất và đã xóa SHM & SEM.\n");
}

/* ---------------- MESSAGE QUEUE ---------------- */
struct mymsg {
    long mtype;
    char mtext[256];
};

void demo_mq_producer() {
    int msqid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("[MQ-PROD] msgget");
        return;
    }

    struct mymsg msg;
    msg.mtype = 1;
    strncpy(msg.mtext, "Hello từ Message Queue!", sizeof(msg.mtext)-1);
    msg.mtext[sizeof(msg.mtext)-1] = '\0';

    if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
        perror("[MQ-PROD] msgsnd");
    } else {
        printf("[MQ-PROD] Đã gửi message.\n");
    }
}

void demo_mq_consumer() {
    int msqid = msgget(MSG_KEY, 0666);
    if (msqid == -1) {
        perror("[MQ-CONS] msgget");
        return;
    }

    struct mymsg msg;
    if (msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
        perror("[MQ-CONS] msgrcv");
    } else {
        printf("[MQ-CONS] Nhận: %s\n", msg.mtext);
    }

    if (msgctl(msqid, IPC_RMID, NULL) == -1) perror("[MQ-CONS] msgctl(IPC_RMID)");
}

/* ---------------- FIFO (named pipe) ---------------- */
void demo_fifo_writer() {
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("[FIFO-W] mkfifo");
        return;
    }

    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("[FIFO-W] open");
        return;
    }

    const char *msg = "Hello từ FIFO!";
    if (write(fd, msg, strlen(msg) + 1) == -1) perror("[FIFO-W] write");
    close(fd);
    printf("[FIFO-W] Đã ghi vào FIFO.\n");
}

void demo_fifo_reader() {
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("[FIFO-R] mkfifo");
        return;
    }

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("[FIFO-R] open");
        return;
    }

    char buf[256] = {0};
    ssize_t r = read(fd, buf, sizeof(buf) - 1);
    if (r == -1) perror("[FIFO-R] read");
    else printf("[FIFO-R] Nhận: %s\n", buf);

    close(fd);
    // Xóa FIFO (nếu muốn)
    if (unlink(FIFO_PATH) == -1) perror("[FIFO-R] unlink");
}

/* ---------------- MAIN ---------------- */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Cách dùng:\n");
        printf("  ./demo pipe\n");
        printf("  ./demo signal\n");
        printf("  ./demo shm_p   # producer (tạo shm+sem, ghi, sem_post)\n");
        printf("  ./demo shm_c   # consumer (chờ sem, đọc, delete shm+sem)\n");
        printf("  ./demo mq_p\n");
        printf("  ./demo mq_c\n");
        printf("  ./demo fifo_w\n");
        printf("  ./demo fifo_r\n");
        return 0;
    }

    if (strcmp(argv[1], "pipe") == 0) demo_pipe();
    else if (strcmp(argv[1], "signal") == 0) demo_signal();
    else if (strcmp(argv[1], "shm_p") == 0) demo_shm_producer();
    else if (strcmp(argv[1], "shm_c") == 0) demo_shm_consumer();
    else if (strcmp(argv[1], "mq_p") == 0) demo_mq_producer();
    else if (strcmp(argv[1], "mq_c") == 0) demo_mq_consumer();
    else if (strcmp(argv[1], "fifo_w") == 0) demo_fifo_writer();
    else if (strcmp(argv[1], "fifo_r") == 0) demo_fifo_reader();
    else printf("Lệnh không hợp lệ.\n");

    return 0;
}
