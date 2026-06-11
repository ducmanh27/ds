# Chương 7: Process Control

## Mục tiêu
- Hiểu sự khác biệt giữa Program và Process
- Sử dụng fork(), exec(), wait() để tạo và quản lý process
- Giao tiếp giữa processes qua pipe
- Xử lý signals

---

## 7.1 Process là gì?

```
Program  = file thực thi trên disk (tĩnh)
Process  = program đang chạy (động)

Mỗi process có:
  - PID  (Process ID) — số định danh duy nhất
  - PPID (Parent PID) — PID của process cha
  - Bộ nhớ riêng: text, data, heap, stack
  - File descriptors riêng
  - Trạng thái: running, sleeping, zombie, stopped
```

```c
#include <unistd.h>
#include <sys/types.h>

pid_t pid  = getpid();    // PID của process hiện tại
pid_t ppid = getppid();   // PID của process cha
printf("PID=%d PPID=%d\n", pid, ppid);
```

---

## 7.2 fork() — Tạo Process Con

```c
#include <unistd.h>

pid_t pid = fork();
```

### Điều xảy ra sau fork()
```
Trước fork():
  Process A (PID=100)

Sau fork():
  Process A (PID=100) — parent → fork() trả về PID con (101)
  Process B (PID=101) — child  → fork() trả về 0
```

### Cấu trúc chuẩn
```c
pid_t pid = fork();

if (pid < 0) {
    perror("fork failed");
    exit(1);
} else if (pid == 0) {
    // === CHILD PROCESS ===
    printf("Child: PID=%d\n", getpid());
    exit(0);
} else {
    // === PARENT PROCESS ===
    printf("Parent: child PID=%d\n", pid);
}
```

### Bộ nhớ sau fork() — độc lập
```c
int x = 10;
pid_t pid = fork();

if (pid == 0) {
    x = 99;
    printf("Child:  x=%d\n", x);   // 99
} else {
    wait(NULL);
    printf("Parent: x=%d\n", x);   // vẫn là 10
}
```

> Child nhận **bản copy** bộ nhớ của parent — thay đổi ở child không ảnh hưởng parent.

---

## 7.3 wait() & waitpid()

```c
#include <sys/wait.h>

// Đợi BẤT KỲ child nào kết thúc
wait(NULL);

// Đợi với status
int status;
wait(&status);
if (WIFEXITED(status))
    printf("Exit code: %d\n", WEXITSTATUS(status));

// Đợi child cụ thể theo PID
waitpid(pid, &status, 0);

// Không block — kiểm tra ngay, không đợi
waitpid(pid, &status, WNOHANG);
```

### Zombie Process
```
Child kết thúc nhưng parent chưa gọi wait()
→ Entry trong process table vẫn còn
→ Chiếm tài nguyên hệ thống

Fix: LUÔN gọi wait() hoặc waitpid() sau fork()
```

---

## 7.4 exec() — Thay thế Process

```c
#include <unistd.h>

// exec thay thế image của process hiện tại
// Nếu thành công → code SAU exec KHÔNG BAO GIỜ chạy

execl("/bin/ls", "ls", "-la", NULL);
perror("exec");   // chỉ đến đây nếu exec thất bại
```

### Các biến thể exec

| Hàm | Truyền args | Tìm PATH |
|-----|-------------|----------|
| `execl(path, arg0, ..., NULL)` | list | không |
| `execv(path, argv[])` | array | không |
| `execvp(file, argv[])` | array | có |
| `execle(path, arg0, ..., NULL, envp[])` | list | không |

---

## 7.5 fork + exec — Pattern chuẩn

```c
// Cách shell thực thi lệnh
pid_t pid = fork();

if (pid == 0) {
    // Child: thay bằng chương trình mới
    execvp("ls", (char*[]){"ls", "-la", NULL});
    perror("exec");   // chỉ đến đây nếu exec thất bại
    exit(1);
} else {
    // Parent: đợi child xong
    int status;
    waitpid(pid, &status, 0);
    printf("Exited with: %d\n", WEXITSTATUS(status));
}
```

---

## 7.6 Pipe — Giao tiếp giữa Processes

```
ls | grep .c
     ↓
stdout(ls) → [pipe] → stdin(grep)
```

```c
#include <unistd.h>

int fd[2];
pipe(fd);
// fd[0] = đầu đọc  (read end)
// fd[1] = đầu ghi  (write end)

pid_t pid = fork();

if (pid == 0) {
    // Child: ghi vào pipe
    close(fd[0]);                           // đóng đầu không dùng
    write(fd[1], "Hello from child", 16);
    close(fd[1]);
    exit(0);
} else {
    // Parent: đọc từ pipe
    close(fd[1]);                           // đóng đầu không dùng
    char buf[100];
    int n = read(fd[0], buf, sizeof(buf));
    buf[n] = '\0';
    printf("Got: %s\n", buf);
    close(fd[0]);
    wait(NULL);
}
```

### Pipe giữa 2 lệnh (dùng dup2)
```c
// ls | grep .c
int fd[2];
pipe(fd);

if (fork() == 0) {
    // Child 1: chạy ls, stdout → pipe
    dup2(fd[1], STDOUT_FILENO);   // stdout = fd[1]
    close(fd[0]); close(fd[1]);
    execlp("ls", "ls", NULL);
}

if (fork() == 0) {
    // Child 2: chạy grep, stdin ← pipe
    dup2(fd[0], STDIN_FILENO);    // stdin = fd[0]
    close(fd[0]); close(fd[1]);
    execlp("grep", "grep", ".c", NULL);
}

close(fd[0]); close(fd[1]);
wait(NULL); wait(NULL);
```

---

## 7.7 Signals

### Các signal phổ biến

| Signal | Số | Nguyên nhân | Mặc định |
|--------|-----|-------------|----------|
| SIGINT | 2 | Ctrl+C | Terminate |
| SIGTERM | 15 | `kill <pid>` | Terminate |
| SIGKILL | 9 | `kill -9 <pid>` | Terminate (không chặn được) |
| SIGSEGV | 11 | Segmentation fault | Core dump |
| SIGCHLD | 17 | Child kết thúc | Ignore |
| SIGALRM | 14 | Timer hết | Terminate |

### Xử lý signal
```c
#include <signal.h>

void handler(int sig) {
    printf("Caught signal %d\n", sig);
}

signal(SIGINT, handler);    // đăng ký handler
signal(SIGINT, SIG_IGN);    // bỏ qua signal
signal(SIGINT, SIG_DFL);    // khôi phục về mặc định

kill(pid, SIGTERM);         // gửi signal tới process khác
kill(getpid(), SIGINT);     // gửi cho chính mình
```

### Ứng dụng — cleanup trước khi thoát
```c
volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
    printf("\nCleaning up...\n");
    running = 0;
}

int main() {
    signal(SIGINT, handle_sigint);

    while (running) {
        printf("Working...\n");
        sleep(1);
    }

    free_resources();
    printf("Bye!\n");
    return 0;
}
```

---

## 7.8 Ví dụ tổng hợp — Mini Shell

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_ARGS 64

void parse(char *line, char **argv) {
    int i = 0;
    argv[i] = strtok(line, " \n");
    while (argv[i] != NULL)
        argv[++i] = strtok(NULL, " \n");
}

int main() {
    char line[256];
    char *argv[MAX_ARGS];

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;
        if (line[0] == '\n') continue;

        parse(line, argv);
        if (strcmp(argv[0], "exit") == 0) break;

        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            fprintf(stderr, "Command not found: %s\n", argv[0]);
            exit(1);
        } else {
            wait(NULL);
        }
    }
    return 0;
}
```

---

## 7.9 Tóm tắt luồng hoạt động

```
fork()
  ├── child (pid == 0)
  │     └── exec() → chạy chương trình mới
  │           └── exit() → kết thúc
  └── parent (pid > 0)
        └── wait() / waitpid() → đợi child
              └── tiếp tục công việc
```

---

## 7.10 Bài tập

1. Viết chương trình fork 3 child, mỗi child in PID rồi thoát, parent đợi tất cả rồi in "All done".
2. Dùng pipe: parent gửi mảng số nguyên, child tính tổng rồi gửi kết quả lại.
3. Mở rộng mini shell hỗ trợ pipe: `cmd1 | cmd2` bằng `dup2()`.
4. Viết chương trình bắt SIGTERM để lưu state ra file trước khi thoát.
