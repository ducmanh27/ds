# Chương 5: Testing & Debugging

## Mục tiêu
- Sử dụng GDB để debug chương trình C
- Dùng Valgrind để phát hiện lỗi bộ nhớ
- Viết unit test cơ bản trong C

---

## 5.1 GDB — GNU Debugger

### Compile với debug info
```bash
gcc -g -o program program.c   # bắt buộc có -g
```

### Khởi động GDB
```bash
gdb ./program
```

### Các lệnh cơ bản

| Lệnh | Viết tắt | Chức năng |
|------|----------|-----------|
| `run` | `r` | Chạy chương trình |
| `break main` | `b main` | Đặt breakpoint tại hàm |
| `break file.c:25` | `b file.c:25` | Đặt breakpoint tại dòng 25 |
| `next` | `n` | Chạy dòng tiếp (không vào hàm) |
| `step` | `s` | Chạy dòng tiếp (vào trong hàm) |
| `continue` | `c` | Chạy tiếp đến breakpoint kế |
| `finish` | | Chạy hết hàm hiện tại rồi dừng |
| `print x` | `p x` | In giá trị biến x |
| `backtrace` | `bt` | In call stack |
| `info locals` | | In tất cả biến local |
| `quit` | `q` | Thoát GDB |

### In giá trị nâng cao
```bash
print *p              # dereference pointer
print arr[0]@5        # in 5 phần tử từ arr[0]
print list->head->data
display x             # tự động in x mỗi lần dừng
watch x               # dừng khi giá trị x thay đổi
x/10x $rsp            # xem 10 words tại địa chỉ rsp (hex)
```

---

## 5.2 Debug Segfault bằng GDB

```c
// Chương trình bị segfault
int main() {
    int *p = NULL;
    *p = 5;           // CRASH
    return 0;
}
```

```bash
gdb ./program
run
# Program received signal SIGSEGV, Segmentation fault.

backtrace
# #0  main () at program.c:3  → crash ở dòng 3

print p
# $1 = 0x0   → p là NULL → rõ nguyên nhân
```

---

## 5.3 Core Dump

```bash
ulimit -c unlimited     # cho phép tạo core dump
./program               # chạy → crash → sinh file core

gdb ./program core      # mở core dump
backtrace               # xem call stack lúc crash
```

---

## 5.4 Valgrind — Phát hiện lỗi bộ nhớ

### Chạy Valgrind
```bash
valgrind --leak-check=full --track-origins=yes ./program
```

### Các loại lỗi Valgrind phát hiện

**1. Memory Leak — quên free:**
```c
int *p = malloc(100);
// quên free
```
```
LEAK SUMMARY:
definitely lost: 100 bytes in 1 blocks
```

**2. Invalid Read/Write — truy cập ngoài mảng:**
```c
int arr[5];
arr[5] = 99;
```
```
Invalid write of size 4
Address 0x... is 0 bytes after a block of size 20
```

**3. Use After Free:**
```c
free(p);
*p = 5;
```
```
Invalid write of size 4
Address 0x... is 0 bytes inside a block of size 4 free'd
```

**4. Uninitialized Value:**
```c
int x;
if (x > 0) { ... }   // x chưa khởi tạo
```
```
Conditional jump depends on uninitialised value(s)
```

---

## 5.5 Unit Testing trong C

```c
#include <stdio.h>
#include <assert.h>

// Hàm cần test
int add(int a, int b) { return a + b; }

// Macro test đơn giản
#define TEST(condition, name) \
    if (condition) printf("PASS: %s\n", name); \
    else           printf("FAIL: %s\n", name);

void test_add() {
    TEST(add(2, 3) == 5,  "add positive");
    TEST(add(-1, 1) == 0, "add negative");
    TEST(add(0, 0) == 0,  "add zeros");
}

// assert — crash nếu sai
void test_with_assert() {
    assert(add(2, 3) == 5);
}

int main() {
    test_add();
    return 0;
}
```

---

## 5.6 Checklist Debug khi gặp lỗi

```
Bước 1: Đọc thông báo lỗi — segfault, bus error, assertion failed?
Bước 2: Compile với -g
Bước 3: Chạy GDB → backtrace → xác định dòng crash
Bước 4: print các biến xung quanh → tìm giá trị bất thường
Bước 5: Chạy Valgrind → tìm memory error nếu có
Bước 6: Kiểm tra NULL pointer trước khi dereference
Bước 7: Kiểm tra bounds của mảng
```

---

## 5.7 Bài tập

1. Viết chương trình có segfault, dùng GDB tìm đúng dòng gây crash.
2. Cố tình tạo memory leak + use-after-free, chạy Valgrind đọc output.
3. Viết unit test đầy đủ cho linked list (push, pop, search, free).
4. Dùng `watch` trong GDB để bắt thời điểm một biến bị thay đổi bất ngờ.
