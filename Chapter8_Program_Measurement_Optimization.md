# Chương 8: Program Measurement & Optimization

## Mục tiêu
- Đo thời gian thực thi chương trình chính xác
- Hiểu cache hierarchy và locality
- Profile chương trình để tìm bottleneck
- Tối ưu code dựa trên hiểu biết về hardware

---

## 8.1 Đo thời gian thực thi

### Dùng `time` command
```bash
gcc -O2 -o program program.c
time ./program
```
```
real    0m1.234s   # thời gian thực tế (wall clock)
user    0m1.100s   # thời gian CPU dùng cho code
sys     0m0.050s   # thời gian CPU dùng cho kernel/syscalls
```

### Đo trong code — `clock()`
```c
#include <time.h>

clock_t start = clock();
// ... đoạn code cần đo ...
clock_t end = clock();

double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
printf("Time: %f seconds\n", elapsed);
```

### Đo chính xác hơn — `clock_gettime()`
```c
#include <time.h>

struct timespec start, end;
clock_gettime(CLOCK_MONOTONIC, &start);

// ... code ...

clock_gettime(CLOCK_MONOTONIC, &end);

double elapsed = (end.tv_sec - start.tv_sec)
               + (end.tv_nsec - start.tv_nsec) / 1e9;
printf("Time: %f seconds\n", elapsed);
```

> `CLOCK_MONOTONIC` không bị ảnh hưởng khi hệ thống chỉnh giờ — chính xác hơn `clock()`.

---

## 8.2 Compiler Optimization Flags

```bash
gcc -O0 program.c    # không optimize — dễ debug
gcc -O1 program.c    # optimize cơ bản
gcc -O2 program.c    # optimize chuẩn — thường dùng
gcc -O3 program.c    # optimize mạnh — có thể tăng kích thước code
gcc -Os program.c    # optimize cho kích thước
```

### Compiler có thể làm gì
```c
// Code gốc
for (int i = 0; i < 100; i++) {
    sum += arr[i] * 2;
}

// Compiler -O2 có thể:
// - Loop unrolling: xử lý nhiều phần tử mỗi vòng
// - Strength reduction: x*2 → x<<1
// - Vectorization: dùng SIMD instructions
// - Constant folding: tính trước giá trị hằng số
```

---

## 8.3 Cache Hierarchy

```
┌─────────────────────────────────────┐
│  CPU Registers   — vài ns, vài chục bytes │
├─────────────────────────────────────┤
│  L1 Cache  — ~1ns,   32-64 KB         │
├─────────────────────────────────────┤
│  L2 Cache  — ~4ns,   256KB-1MB        │
├─────────────────────────────────────┤
│  L3 Cache  — ~15ns,  8-32MB (shared)  │
├─────────────────────────────────────┤
│  RAM       — ~100ns, GBs              │
├─────────────────────────────────────┤
│  Disk/SSD  — ~100,000ns (0.1ms), TBs  │
└─────────────────────────────────────┘
```

Mỗi tầng càng xa CPU → càng lớn nhưng càng chậm. Mục tiêu tối ưu: **tận dụng cache, giảm truy cập RAM**.

---

## 8.4 Locality — Nguyên lý quan trọng nhất

### Temporal Locality (cục bộ thời gian)
Dữ liệu vừa truy cập có khả năng cao sẽ được truy cập lại sớm.
```c
for (int i = 0; i < 1000000; i++) {
    sum += x;   // x được truy cập lặp lại → ở trong cache
}
```

### Spatial Locality (cục bộ không gian)
Dữ liệu gần nhau trong bộ nhớ có khả năng cao sẽ được truy cập gần nhau về thời gian.
```c
// TỐT — truy cập tuần tự, cache load theo block
for (int i = 0; i < n; i++)
    sum += arr[i];

// XẤU — nhảy cách → cache miss liên tục
for (int i = 0; i < n; i += 16)
    sum += arr[i];
```

---

## 8.5 Ví dụ kinh điển — Matrix Multiplication

```c
#define N 1000
double A[N][N], B[N][N], C[N][N];

// CHẬM — truy cập B theo cột (không liền nhau trong memory)
void multiply_slow() {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                C[i][j] += A[i][k] * B[k][j];  // B[k][j] nhảy N*8 bytes mỗi lần
}

// NHANH — đổi thứ tự loop, truy cập theo hàng (liền nhau)
void multiply_fast() {
    for (int i = 0; i < N; i++)
        for (int k = 0; k < N; k++)
            for (int j = 0; j < N; j++)
                C[i][j] += A[i][k] * B[k][j];  // B[k][j] truy cập tuần tự
}
```

> `multiply_fast` có thể nhanh hơn **5-10 lần** chỉ bằng đổi thứ tự vòng lặp — không đổi logic, chỉ tận dụng cache tốt hơn.

---

## 8.6 Array of Structs vs Struct of Arrays

```c
#define N 1000000

// AoS — Array of Structs
typedef struct {
    float x, y, z;
    int id;
} Particle;
Particle particles[N];

// Nếu chỉ cần xử lý x của tất cả particle:
for (int i = 0; i < N; i++)
    particles[i].x *= 2;   // mỗi lần load cả struct (16 bytes) chỉ dùng 4 bytes


// SoA — Struct of Arrays
typedef struct {
    float x[N], y[N], z[N];
    int id[N];
} ParticleSystem;
ParticleSystem ps;

for (int i = 0; i < N; i++)
    ps.x[i] *= 2;   // load liên tục toàn x → cache hiệu quả hơn
```

> SoA thường nhanh hơn khi xử lý từng field riêng lẻ trên dữ liệu lớn.

---

## 8.7 Profiling với `gprof`

```bash
gcc -pg -O2 -o program program.c   # -pg để sinh profiling info
./program                           # tạo file gmon.out
gprof program gmon.out > report.txt
```

```
report.txt sẽ cho biết:
- Mỗi hàm chiếm bao nhiêu % thời gian
- Hàm nào được gọi bao nhiêu lần
- Call graph: hàm nào gọi hàm nào
```

---

## 8.8 Profiling với `perf` (Linux)

```bash
perf stat ./program          # thống kê tổng quan: cache miss, branch miss...
perf record ./program        # ghi lại profile chi tiết
perf report                   # xem báo cáo, dòng nào tốn nhiều thời gian
```

```
Output mẫu của perf stat:
  1,234,567   cache-misses
  9,876,543   cache-references
  12.5%       cache miss rate
```

---

## 8.9 Các kỹ thuật tối ưu thường gặp

```c
// 1. Tránh tính toán lặp lại
// XẤU
for (int i = 0; i < strlen(s); i++)   // strlen gọi lại MỖI vòng lặp!
    ...

// TỐT
int len = strlen(s);
for (int i = 0; i < len; i++)
    ...


// 2. Truyền struct lớn bằng pointer thay vì copy
// XẤU — copy toàn bộ struct mỗi lần gọi
void process(BigStruct s) { ... }

// TỐT
void process(const BigStruct *s) { ... }


// 3. Dùng đúng kiểu dữ liệu
// int thường nhanh hơn long trên hệ 32-bit
// double thường nhanh hơn float trên hầu hết CPU hiện đại (FPU optimize cho double)


// 4. Loop-invariant code motion
// XẤU
for (int i = 0; i < n; i++)
    arr[i] = x * y + i;   // x*y không đổi nhưng tính lại mỗi vòng

// TỐT
int xy = x * y;
for (int i = 0; i < n; i++)
    arr[i] = xy + i;
```

---

## 8.10 Big-O lý thuyết vs thực tế

```
Big-O cho biết xu hướng khi n → ∞, nhưng:

- O(n) với constant lớn có thể CHẬM HƠN O(n log n) với n nhỏ
- Cache locality có thể làm O(n²) "cache-friendly" nhanh hơn
  O(n log n) "cache-unfriendly" với n không quá lớn
- Luôn benchmark thực tế, đừng chỉ dựa vào Big-O
```

---

## 8.11 Bài tập

1. Viết 2 phiên bản matrix multiply (loop order khác nhau), đo thời gian bằng `clock_gettime`, so sánh.
2. Dùng `perf stat` đo cache-miss rate của 2 cách truy cập mảng: tuần tự vs stride 64.
3. Viết chương trình tính tổng mảng 10 triệu phần tử, compile với `-O0` và `-O2`, so sánh thời gian.
4. Dùng `gprof` profile một chương trình có nhiều hàm, xác định hàm nào chiếm thời gian nhiều nhất.
