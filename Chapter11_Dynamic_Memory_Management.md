# Chương 11: Dynamic Memory Management

## Mục tiêu
- Hiểu cách malloc/free hoạt động bên trong
- Hiểu fragmentation và các chiến lược cấp phát
- Hiểu sbrk/mmap — cách OS cấp bộ nhớ cho heap
- Tự implement một malloc đơn giản

---

## 11.1 Ôn lại API malloc/free

```c
#include <stdlib.h>

void *malloc(size_t size);             // cấp phát, không khởi tạo
void *calloc(size_t n, size_t size);   // cấp phát n*size bytes, khởi tạo 0
void *realloc(void *ptr, size_t size); // thay đổi kích thước
void free(void *ptr);                   // giải phóng
```

```c
int *arr = malloc(10 * sizeof(int));
if (arr == NULL) exit(1);   // luôn kiểm tra
// ... dùng arr ...
free(arr);
arr = NULL;
```

---

## 11.2 Heap được cấp phát từ OS như thế nào

```
Process memory:
┌─────────────┐
│   Stack     │
│     ↓       │
│  (trống)    │
│     ↑       │
│   Heap      │ ← malloc lấy bộ nhớ từ đây
├─────────────┤
│ Data/BSS    │
│   Text      │
└─────────────┘
```

### sbrk() — mở rộng heap (cách cũ)
```c
#include <unistd.h>

void *old_top = sbrk(0);        // lấy con trỏ hiện tại của đỉnh heap
void *new_mem = sbrk(4096);     // mở rộng heap thêm 4096 bytes
// new_mem = địa chỉ TRƯỚC khi mở rộng (= old_top)
```

### mmap() — cấp phát vùng nhớ lớn (cách hiện đại)
```c
#include <sys/mman.h>

void *mem = mmap(NULL, size, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
// dùng cho cấp phát lớn (thường > 128KB)
munmap(mem, size);
```

```
glibc malloc thường:
- Request nhỏ → mở rộng heap bằng sbrk()
- Request lớn (> ~128KB) → dùng mmap() trực tiếp, trả về OS ngay khi free
```

---

## 11.3 Free List — Cấu trúc bên trong malloc

malloc quản lý các vùng nhớ trống bằng **free list** — danh sách liên kết các block chưa dùng.

```
Heap layout (mỗi block có header chứa size + trạng thái):

┌────────┬──────────┬────────┬──────────┬────────┬──────────┐
│ header │   data   │ header │   data   │ header │   data   │
│ size=32│ (used)   │ size=64│ (free)   │ size=16│ (used)   │
└────────┴──────────┴────────┴──────────┴────────┴──────────┘
                          ↑
                    free list trỏ tới đây
```

```c
// Header điển hình của 1 block
typedef struct block_header {
    size_t size;              // kích thước block (không tính header)
    int free;                  // 1 = free, 0 = đang dùng
    struct block_header *next; // block free tiếp theo (nếu free)
} BlockHeader;
```

---

## 11.4 Allocation Strategies

Khi malloc(n) được gọi, cần tìm 1 block free đủ lớn trong free list:

### First Fit
```
Chọn block FREE ĐẦU TIÊN đủ lớn.

Free list: [50] → [200] → [80] → [120]
malloc(100) → chọn [200] (đầu tiên >= 100)

Ưu: nhanh
Nhược: dễ tạo fragment nhỏ ở đầu list
```

### Best Fit
```
Chọn block FREE NHỎ NHẤT nhưng vẫn đủ lớn.

Free list: [50] → [200] → [80] → [120]
malloc(100) → chọn [120] (nhỏ nhất >= 100)

Ưu: ít lãng phí không gian mỗi block
Nhược: chậm (phải scan toàn bộ list), tạo nhiều fragment rất nhỏ
```

### Worst Fit
```
Chọn block FREE LỚN NHẤT.

Free list: [50] → [200] → [80] → [120]
malloc(100) → chọn [200] (lớn nhất)

Ưu: phần còn lại sau khi cắt thường vẫn đủ lớn để dùng tiếp
Nhược: làm cạn nhanh các block lớn
```

---

## 11.5 Fragmentation

### Internal Fragmentation
```
Lãng phí bên TRONG 1 block đã cấp — do alignment hoặc cấp dư.

malloc(20) nhưng block nhỏ nhất có sẵn = 32 bytes
→ 12 bytes bị lãng phí bên trong block đó
```

### External Fragmentation
```
Tổng bộ nhớ trống ĐỦ nhưng KHÔNG LIỀN MẠCH → không cấp được request lớn.

Free list: [10] → [10] → [10] → [10]   (tổng = 40 bytes free)
malloc(35)  →  THẤT BẠI vì không có block liền nào >= 35
```

---

## 11.6 Splitting & Coalescing

### Splitting — khi block free lớn hơn cần
```
Block free = 100 bytes, malloc(20)

Trước: [free: 100]
Sau:   [used: 20][free: 80]   ← cắt block thành 2
```

### Coalescing — gộp các block free liền kề khi free()
```
Trước free(B): [used: A][free: B][used: C][free: D]

free(B):
  Nếu C cũng free → gộp B+C thành 1 block free lớn hơn
  Nếu A cũng free → gộp A+B thành 1 block free lớn hơn

→ Giảm external fragmentation
```

```c
// Pseudocode coalescing khi free
void my_free(void *ptr) {
    BlockHeader *block = get_header(ptr);
    block->free = 1;

    // Gộp với block kế tiếp nếu free
    BlockHeader *next = get_next_block(block);
    if (next && next->free) {
        block->size += next->size + HEADER_SIZE;
    }

    // Gộp với block trước nếu free
    BlockHeader *prev = get_prev_block(block);
    if (prev && prev->free) {
        prev->size += block->size + HEADER_SIZE;
    }
}
```

---

## 11.7 Implement Malloc đơn giản

```c
#include <unistd.h>
#include <string.h>

#define ALIGN 8
#define ALIGN_SIZE(size) (((size) + ALIGN - 1) & ~(ALIGN - 1))

typedef struct block_header {
    size_t size;
    int free;
    struct block_header *next;
} BlockHeader;

#define HEADER_SIZE sizeof(BlockHeader)

static BlockHeader *free_list = NULL;

void *my_malloc(size_t size) {
    size = ALIGN_SIZE(size);
    BlockHeader *curr = free_list;
    BlockHeader *prev = NULL;

    // First fit: tìm block free đủ lớn
    while (curr != NULL) {
        if (curr->free && curr->size >= size) {
            curr->free = 0;
            return (char *)curr + HEADER_SIZE;
        }
        prev = curr;
        curr = curr->next;
    }

    // Không có block phù hợp → mở rộng heap bằng sbrk
    BlockHeader *block = sbrk(HEADER_SIZE + size);
    if (block == (void *)-1) return NULL;

    block->size = size;
    block->free = 0;
    block->next = NULL;

    if (prev) prev->next = block;
    else free_list = block;

    return (char *)block + HEADER_SIZE;
}

void my_free(void *ptr) {
    if (ptr == NULL) return;
    BlockHeader *block = (BlockHeader *)((char *)ptr - HEADER_SIZE);
    block->free = 1;
    // (coalescing có thể thêm ở đây)
}
```

---

## 11.8 Memory Alignment

```c
// Hầu hết hệ thống yêu cầu dữ liệu align theo địa chỉ chia hết cho 4, 8, hoặc 16
// malloc luôn trả về địa chỉ đã align (thường 8 hoặc 16 bytes trên 64-bit)

#define ALIGN_SIZE(size) (((size) + 7) & ~7)
// Ví dụ: ALIGN_SIZE(13) = 16, ALIGN_SIZE(16) = 16, ALIGN_SIZE(17) = 24
```

```c
// Struct cũng bị align — padding tự động chèn vào
struct Example {
    char  a;     // 1 byte
    // 3 bytes padding
    int   b;     // 4 bytes
    char  c;     // 1 byte
    // 3 bytes padding (để total chia hết cho 4)
};
// sizeof(struct Example) = 12, không phải 6
```

---

## 11.9 So sánh với Garbage Collection (Java)

| | C (malloc/free) | Java (GC) |
|---|---|---|
| Cấp phát | Thủ công | Tự động |
| Giải phóng | Thủ công — `free()` | Tự động — Garbage Collector |
| Lỗi có thể xảy ra | Leak, dangling pointer, double free | Không (nhưng GC tốn CPU) |
| Hiệu năng | Dự đoán được, không có "GC pause" | Có thể bị pause khi GC chạy |
| Kiểm soát | Toàn quyền kiểm soát | Ít kiểm soát hơn |

---

## 11.10 Công cụ kiểm tra Memory

```bash
# Valgrind — phát hiện leak, invalid access (đã học ở Chương 5)
valgrind --leak-check=full ./program

# Đo memory usage thực tế
/usr/bin/time -v ./program
# Maximum resident set size: ... kB

# heaptrack — phân tích chi tiết heap usage theo thời gian
heaptrack ./program
heaptrack_gui heaptrack.program.*.gz
```

---

## 11.11 Bài tập

1. Implement `my_malloc`/`my_free` với First Fit, test với nhiều lần malloc/free liên tiếp.
2. Thêm coalescing vào `my_free`, viết test case chứng minh giảm fragmentation.
3. So sánh First Fit vs Best Fit: viết script malloc/free ngẫu nhiên 1000 lần, đo tổng heap size cuối cùng của mỗi chiến lược.
4. Viết chương trình minh họa internal fragmentation: malloc nhiều block kích thước lẻ (vd: 13, 17, 25 bytes), tính tổng bytes lãng phí do alignment.
