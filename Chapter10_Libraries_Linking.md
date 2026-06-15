# Chương 10: Libraries & Linking

## Mục tiêu
- Hiểu pipeline: Compile → Assemble → Link
- Tạo và sử dụng static library (.a)
- Tạo và sử dụng shared library (.so)
- Đọc và debug lỗi linker

---

## 10.1 Pipeline biên dịch đầy đủ

```
file.c
  │  (Preprocessor: xử lý #include, #define)
  ▼
file.i   (đã expand macro, include)
  │  (Compiler: C → Assembly)
  ▼
file.s   (assembly code)
  │  (Assembler: Assembly → machine code)
  ▼
file.o   (object file — machine code, chưa link)
  │  (Linker: kết hợp các .o + library)
  ▼
executable   (file chạy được)
```

```bash
gcc -E file.c -o file.i     # chỉ preprocess
gcc -S file.c -o file.s     # chỉ compile → assembly
gcc -c file.c -o file.o     # chỉ assemble → object file
gcc file.o -o program       # chỉ link
```

---

## 10.2 Object File (.o) — chứa gì?

```bash
nm file.o    # xem symbol table
```

```
Symbol types:
T  — defined trong Text (function)
D  — defined trong Data (global variable đã init)
B  — defined trong BSS (global variable chưa init)
U  — Undefined — cần linker tìm ở file khác
```

```c
// file.c
int global_var = 10;       // → D (data)
int uninit_var;             // → B (bss)
int add(int a, int b) {...} // → T (text)
extern int external_func();  // → U (undefined, cần link)
```

---

## 10.3 Multiple Files — Compile riêng, Link sau

```c
// math_utils.c
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

// math_utils.h
#ifndef MATH_UTILS_H
#define MATH_UTILS_H
int add(int a, int b);
int sub(int a, int b);
#endif

// main.c
#include "math_utils.h"
#include <stdio.h>
int main() {
    printf("%d\n", add(3, 4));
    return 0;
}
```

```bash
gcc -c math_utils.c -o math_utils.o
gcc -c main.c -o main.o
gcc main.o math_utils.o -o program
```

> **Lợi ích:** chỉ compile lại file đã sửa, không cần compile lại toàn bộ project → dùng `make` để tự động hóa.

---

## 10.4 Static Library (.a)

### Tạo static library
```bash
gcc -c math_utils.c -o math_utils.o
gcc -c string_utils.c -o string_utils.o

ar rcs libmymath.a math_utils.o string_utils.o
#  r = insert/replace files
#  c = create archive nếu chưa có
#  s = tạo index (symbol table) cho linker
```

### Dùng static library
```bash
gcc main.c -L. -lmymath -o program
#  -L.        = tìm library trong thư mục hiện tại
#  -lmymath   = link với libmymath.a (bỏ "lib" và ".a")
```

### Đặc điểm Static Library
```
- Code của library được COPY vào executable lúc link
- Executable lớn hơn, nhưng KHÔNG cần library lúc chạy
- Nếu library update → phải LINK LẠI toàn bộ
```

---

## 10.5 Shared Library (.so)

### Tạo shared library
```bash
gcc -fPIC -c math_utils.c -o math_utils.o
#  -fPIC = Position Independent Code, cần cho shared lib

gcc -shared -o libmymath.so math_utils.o
```

### Dùng shared library
```bash
gcc main.c -L. -lmymath -o program

# Chạy — cần chỉ đường dẫn tìm .so lúc runtime
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./program
```

### Đặc điểm Shared Library
```
- Code KHÔNG copy vào executable — chỉ tham chiếu
- Executable nhỏ hơn
- NHIỀU chương trình có thể dùng CHUNG 1 bản .so trong RAM
- Library update → các chương trình dùng nó tự động dùng bản mới
  (không cần recompile)
- Cần .so tồn tại lúc CHẠY, không chỉ lúc compile
```

---

## 10.6 So sánh Static vs Shared

| | Static (.a) | Shared (.so) |
|---|---|---|
| Khi nào copy code | Lúc link | Lúc chạy (load) |
| Kích thước executable | Lớn hơn | Nhỏ hơn |
| Cần file lib lúc chạy | Không | Có |
| Update library | Phải link lại | Tự động (không cần recompile) |
| Nhiều process dùng chung RAM | Không | Có |
| Tốc độ load | Nhanh hơn | Có overhead load library |

---

## 10.7 Kiểm tra Dependencies

```bash
ldd program           # xem chương trình cần .so nào
```
```
linux-vdso.so.1
libmymath.so => not found    ← lỗi: thiếu library lúc chạy
libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6
```

```bash
nm -D libmymath.so     # xem symbol được export bởi shared lib
file program           # kiểm tra loại file, dynamically/statically linked
```

---

## 10.8 Lỗi Linker thường gặp

### 1. Undefined reference
```
undefined reference to 'add'
```
**Nguyên nhân:** dùng hàm nhưng chưa link file/library chứa định nghĩa hàm đó.
```bash
# SAI — quên link math_utils.o
gcc main.o -o program

# ĐÚNG
gcc main.o math_utils.o -o program
```

### 2. Multiple definition
```
multiple definition of 'global_var'
```
**Nguyên nhân:** định nghĩa biến global trong header, include ở nhiều file `.c`.
```c
// SAI — trong header.h
int global_var = 10;   // mỗi file include sẽ có 1 bản → multiple definition

// ĐÚNG — trong header.h
extern int global_var;   // chỉ khai báo, không định nghĩa

// trong 1 file .c duy nhất
int global_var = 10;     // định nghĩa thật ở đây
```

### 3. Cannot find -lxxx
```
/usr/bin/ld: cannot find -lmymath
```
**Nguyên nhân:** thiếu `-L` chỉ đường dẫn, hoặc sai tên library.
```bash
gcc main.c -L/path/to/lib -lmymath -o program
```

### 4. error while loading shared libraries
```
error while loading shared libraries: libmymath.so: cannot open shared object file
```
**Nguyên nhân:** `.so` không nằm trong đường dẫn linker tìm lúc chạy.
```bash
export LD_LIBRARY_PATH=/path/to/lib:$LD_LIBRARY_PATH
# hoặc copy .so vào /usr/lib, /usr/local/lib rồi chạy ldconfig
```

---

## 10.9 extern và static — Phạm vi liên kết

```c
// file1.c
int global_x = 10;          // external linkage — file khác thấy được
static int file_local = 5;  // internal linkage — chỉ file1.c thấy

void public_func() {...}            // external — gọi được từ file khác
static void private_func() {...}    // internal — chỉ dùng trong file1.c

// file2.c
extern int global_x;   // khai báo: "biến này định nghĩa ở file khác"
extern void public_func();

int main() {
    global_x = 20;     // OK
    public_func();     // OK
    // file_local = 1;     // LỖI — không thấy được
    // private_func();     // LỖI — không thấy được
}
```

---

## 10.10 Header File — Best Practices

```c
// myheader.h
#ifndef MYHEADER_H    // include guard — tránh include 2 lần
#define MYHEADER_H

// Chỉ khai báo (declaration), KHÔNG định nghĩa (definition)
extern int global_var;            // khai báo biến
int add(int a, int b);             // khai báo hàm (prototype)

typedef struct {                   // OK — định nghĩa type không gây lỗi
    int x, y;
} Point;

#endif
```

```
Quy tắc:
- Header (.h): khai báo (prototype, extern, struct, typedef, #define)
- Source (.c): định nghĩa (function body, biến global thật)
```

---

## 10.11 Makefile cơ bản

```makefile
CC = gcc
CFLAGS = -Wall -g

program: main.o math_utils.o
	$(CC) main.o math_utils.o -o program

main.o: main.c math_utils.h
	$(CC) $(CFLAGS) -c main.c

math_utils.o: math_utils.c math_utils.h
	$(CC) $(CFLAGS) -c math_utils.c

clean:
	rm -f *.o program
```

```bash
make           # build
make clean     # xóa file build
```

---

## 10.12 Bài tập

1. Tách 1 chương trình thành 3 file: `main.c`, `utils.c`, `utils.h`. Compile riêng từng file rồi link.
2. Tạo static library `.a` từ 2 file `.c`, link vào `main.c`, chạy thử.
3. Tạo shared library `.so`, dùng `ldd` kiểm tra dependencies, cố tình xóa LD_LIBRARY_PATH để tái tạo lỗi "cannot open shared object file" rồi fix.
4. Viết Makefile cho project gồm 3 file `.c`, hỗ trợ `make` và `make clean`.
