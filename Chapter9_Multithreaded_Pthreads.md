# Chương 9: Multithreaded Programming with pthreads

## Mục tiêu
- Hiểu sự khác biệt giữa Thread và Process
- Tạo, chạy, đợi thread bằng pthread
- Hiểu race condition và cách dùng mutex
- Hiểu deadlock và cách tránh

---

## 9.1 Thread vs Process

```
Process:
  - Bộ nhớ riêng (heap, data, stack riêng)
  - Tốn tài nguyên để tạo/chuyển đổi

Thread:
  - CHIA SẺ bộ nhớ với các thread khác trong cùng process
    (heap, global/static data, code)
  - Mỗi thread có STACK RIÊNG
  - Nhẹ hơn process, tạo nhanh hơn
```

```
┌─────────────── Process ───────────────┐
│  Code (shared)                          │
│  Heap (shared)                          │
│  Global/Static data (shared)            │
│                                          │
│  Thread 1        Thread 2        Thread 3│
│  ┌────────┐     ┌────────┐     ┌────────┐│
│  │ Stack 1│     │ Stack 2│     │ Stack 3││
│  │ regs   │     │ regs   │     │ regs   ││
│  └────────┘     └────────┘     └────────┘│
└──────────────────────────────────────────┘
```

> Vì chia sẻ bộ nhớ → nhanh, dễ giao tiếp, nhưng dễ xảy ra **race condition**.

---

## 9.2 Tạo Thread cơ bản

```c
#include <pthread.h>
#include <stdio.h>

void *task(void *arg) {
    int id = *(int *)arg;
    printf("Thread %d running\n", id);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    pthread_create(&t1, NULL, task, &id1);
    pthread_create(&t2, NULL, task, &id2);

    pthread_join(t1, NULL);   // đợi t1 xong
    pthread_join(t2, NULL);   // đợi t2 xong

    printf("Done\n");
    return 0;
}
```

```bash
gcc -pthread -o program program.c   # bắt buộc -pthread
```

### Giải thích các hàm

| Hàm | Chức năng |
|-----|-----------|
| `pthread_create(&t, attr, func, arg)` | Tạo thread mới, chạy `func(arg)` |
| `pthread_join(t, &retval)` | Đợi thread `t` kết thúc, lấy giá trị trả về |
| `pthread_exit(retval)` | Thread tự kết thúc, trả về `retval` |
| `pthread_self()` | Lấy ID của thread hiện tại |

---

## 9.3 Truyền nhiều dữ liệu vào thread

```c
typedef struct {
    int *arr;
    int start, end;
    int sum;
} ThreadData;

void *sum_range(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->sum = 0;
    for (int i = data->start; i < data->end; i++)
        data->sum += data->arr[i];
    return NULL;
}

int main() {
    int arr[1000];
    for (int i = 0; i < 1000; i++) arr[i] = i;

    pthread_t t1, t2;
    ThreadData d1 = {arr, 0, 500, 0};
    ThreadData d2 = {arr, 500, 1000, 0};

    pthread_create(&t1, NULL, sum_range, &d1);
    pthread_create(&t2, NULL, sum_range, &d2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Total: %d\n", d1.sum + d2.sum);
    return 0;
}
```

---

## 9.4 Race Condition

```c
int counter = 0;   // shared giữa các thread

void *increment(void *arg) {
    for (int i = 0; i < 100000; i++)
        counter++;     // KHÔNG AN TOÀN — nhiều thread cùng đọc/ghi
    return NULL;
}
```

### Tại sao `counter++` không an toàn?

`counter++` thực ra là 3 bước (xem lại assembly chương 6):
```asm
mov  counter, %eax    # 1. đọc counter vào register
inc  %eax              # 2. tăng 1
mov  %eax, counter    # 3. ghi lại vào counter
```

```
Thread A: đọc counter = 5
Thread B: đọc counter = 5     ← cùng đọc giá trị 5
Thread A: tăng → 6, ghi lại counter = 6
Thread B: tăng → 6, ghi lại counter = 6   ← mất 1 lần tăng!

Kết quả: counter = 6, đáng lẽ phải = 7
```

> Chạy nhiều lần, kết quả `counter` sẽ KHÁC NHAU mỗi lần — đây là dấu hiệu race condition.

---

## 9.5 Mutex — Giải quyết Race Condition

```c
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void *increment(void *arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&lock);     // chỉ 1 thread vào được
        counter++;                      // critical section
        pthread_mutex_unlock(&lock);   // mở khóa cho thread khác
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Counter: %d\n", counter);  // luôn = 200000
    pthread_mutex_destroy(&lock);
    return 0;
}
```

### Quy tắc Mutex
```
- Critical section CÀNG NGẮN CÀNG TỐT (chỉ lock đúng phần cần)
- LUÔN unlock sau khi lock — kể cả khi có early return / error
- Không lock 2 lần liên tiếp cùng 1 mutex (deadlock với chính nó)
```

---

## 9.6 Deadlock

```c
pthread_mutex_t lockA, lockB;

// Thread 1
void *thread1(void *arg) {
    pthread_mutex_lock(&lockA);
    sleep(1);
    pthread_mutex_lock(&lockB);   // chờ lockB
    // ...
    pthread_mutex_unlock(&lockB);
    pthread_mutex_unlock(&lockA);
}

// Thread 2
void *thread2(void *arg) {
    pthread_mutex_lock(&lockB);
    sleep(1);
    pthread_mutex_lock(&lockA);   // chờ lockA
    // ...
    pthread_mutex_unlock(&lockA);
    pthread_mutex_unlock(&lockB);
}
```

```
Thread 1 giữ lockA, chờ lockB
Thread 2 giữ lockB, chờ lockA
→ Cả 2 chờ nhau VĨNH VIỄN = DEADLOCK
```

### Cách tránh Deadlock
```
1. Luôn lock các mutex theo THỨ TỰ CỐ ĐỊNH (vd: luôn lockA trước lockB)
2. Tránh giữ lock khi gọi hàm khác có thể lock
3. Dùng pthread_mutex_trylock() để thử lock không block
4. Giảm số lượng lock cần giữ đồng thời
```

---

## 9.7 Condition Variable — Đợi một điều kiện

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int data_ready = 0;

// Thread tiêu thụ — đợi dữ liệu
void *consumer(void *arg) {
    pthread_mutex_lock(&lock);
    while (!data_ready)
        pthread_cond_wait(&cond, &lock);   // ngủ, tự unlock; thức dậy tự lock lại
    printf("Data is ready!\n");
    pthread_mutex_unlock(&lock);
    return NULL;
}

// Thread sản xuất — báo dữ liệu sẵn sàng
void *producer(void *arg) {
    pthread_mutex_lock(&lock);
    data_ready = 1;
    pthread_cond_signal(&cond);   // đánh thức 1 thread đang wait
    pthread_mutex_unlock(&lock);
    return NULL;
}
```

---

## 9.8 Producer-Consumer Pattern (tổng hợp)

```c
#define BUF_SIZE 10

int buffer[BUF_SIZE];
int count = 0, in = 0, out = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    for (int i = 0; i < 20; i++) {
        pthread_mutex_lock(&lock);
        while (count == BUF_SIZE)
            pthread_cond_wait(&not_full, &lock);

        buffer[in] = i;
        in = (in + 1) % BUF_SIZE;
        count++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 0; i < 20; i++) {
        pthread_mutex_lock(&lock);
        while (count == 0)
            pthread_cond_wait(&not_empty, &lock);

        int val = buffer[out];
        out = (out + 1) % BUF_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&lock);
        printf("Consumed: %d\n", val);
    }
    return NULL;
}
```

---

## 9.9 Các lỗi thường gặp

```c
// 1. Quên pthread_join → main kết thúc trước thread → thread bị hủy
pthread_create(&t, NULL, task, NULL);
return 0;   // thread có thể chưa chạy xong!

// 2. Trả về địa chỉ local variable từ thread
void *task(void *arg) {
    int result = 42;
    return &result;   // NGUY HIỂM — result bị hủy khi thread kết thúc
}

// 3. Quên lock khi truy cập shared data
int shared = 0;
void *task(void *arg) {
    shared++;   // race condition nếu nhiều thread cùng chạy
}

// 4. Lock nhưng quên unlock khi có early return
void *task(void *arg) {
    pthread_mutex_lock(&lock);
    if (error_condition)
        return NULL;   // QUÊN unlock → deadlock cho thread khác!
    pthread_mutex_unlock(&lock);
    return NULL;
}
```

---

## 9.10 Bài tập

1. Viết chương trình tính tổng mảng lớn bằng N thread, mỗi thread xử lý 1 phần, parent tổng hợp kết quả.
2. Tạo race condition với `counter++` không mutex, chạy nhiều lần quan sát kết quả khác nhau, sau đó fix bằng mutex.
3. Implement Producer-Consumer với buffer giới hạn, dùng mutex + condition variable.
4. Tạo deadlock có chủ đích (2 thread, 2 mutex, lock ngược thứ tự), dùng GDB để quan sát cả 2 thread bị block.
