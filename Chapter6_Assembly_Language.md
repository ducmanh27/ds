# Chương 6: Assembly Language

## Mục tiêu
- Hiểu register x86-64 và vai trò của từng register
- Đọc và viết lệnh assembly cơ bản (AT&T syntax)
- Hiểu calling convention và stack frame
- Dùng `gcc -S` và GDB để xem assembly từ C

---

## 6.1 Tại sao cần học Assembly?

- Hiểu C compile ra gì → debug sâu hơn
- Hiểu tại sao đoạn code này nhanh/chậm
- Đọc được disassembly trong GDB
- Hiểu buffer overflow, security vulnerabilities

---

## 6.2 Registers x86-64

```
┌──────────┬────────────────────────────────────────┐
│ Register │ Vai trò                                 │
├──────────┼────────────────────────────────────────┤
│ rax      │ Return value, accumulator               │
│ rbx      │ Callee-saved                            │
│ rcx      │ 4th argument                            │
│ rdx      │ 3rd argument                            │
│ rsi      │ 2nd argument                            │
│ rdi      │ 1st argument                            │
│ r8       │ 5th argument                            │
│ r9       │ 6th argument                            │
│ rsp      │ Stack pointer (đỉnh stack)              │
│ rbp      │ Base pointer (đáy stack frame)          │
│ rip      │ Instruction pointer (Program Counter)   │
│ rflags   │ Flags: ZF, SF, CF, OF...               │
└──────────┴────────────────────────────────────────┘
```

### Một register có nhiều kích thước
```
rax   → 64-bit (toàn bộ)
eax   → 32-bit (phần thấp)
 ax   → 16-bit (phần thấp)
 ah   →  8-bit (byte cao của ax)
 al   →  8-bit (byte thấp của ax)
```

---

## 6.3 Lệnh Assembly (AT&T Syntax)

> **Lưu ý AT&T:** `nguồn → đích` (ngược với Intel syntax)

### Data Movement
```asm
movq $5, %rax           # rax = 5           ($ = immediate value)
movq %rax, %rbx         # rbx = rax
movq (%rax), %rbx       # rbx = *rax        (dereference)
movq %rax, (%rbx)       # *rbx = rax
movq 8(%rsp), %rax      # rax = *(rsp + 8)  (offset)

# Suffix kích thước:
# b=byte(1)  w=word(2)  l=long(4)  q=quad(8)
movb, movw, movl, movq
```

### Arithmetic
```asm
addq %rbx, %rax         # rax = rax + rbx
subq %rbx, %rax         # rax = rax - rbx
imulq %rbx, %rax        # rax = rax * rbx
idivq %rbx              # rax = rdx:rax / rbx
incq %rax               # rax++
decq %rax               # rax--
negq %rax               # rax = -rax
```

### Bitwise
```asm
andq %rbx, %rax         # rax &= rbx
orq  %rbx, %rax         # rax |= rbx
xorq %rbx, %rax         # rax ^= rbx
notq %rax               # rax = ~rax
salq $2, %rax           # rax <<= 2
sarq $2, %rax           # rax >>= 2 (arithmetic, giữ sign bit)
shrq $2, %rax           # rax >>= 2 (logical, fill 0)
```

### Compare & Jump
```asm
cmpq %rbx, %rax         # set flags dựa trên (rax - rbx)
testq %rax, %rax        # set flags dựa trên (rax & rax) — check zero

jmp  label              # nhảy vô điều kiện
je   label              # jump if equal      (ZF=1)
jne  label              # jump if not equal
jl   label              # jump if less
jg   label              # jump if greater
jle  label              # jump if less or equal
jge  label              # jump if greater or equal
```

### Stack & Function
```asm
pushq %rax              # rsp -= 8; *rsp = rax
popq  %rax              # rax = *rsp; rsp += 8

call  func              # push rip; jmp func
ret                     # pop rip; jmp rip
```

---

## 6.4 Calling Convention x86-64

### Truyền Arguments
```
Argument 1 → rdi
Argument 2 → rsi
Argument 3 → rdx
Argument 4 → rcx
Argument 5 → r8
Argument 6 → r9
Argument 7+ → stack (push từ phải sang trái)

Return value → rax
```

### Caller-saved vs Callee-saved
```
Caller-saved (hàm gọi phải lưu nếu cần dùng lại):
  rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11

Callee-saved (hàm được gọi phải giữ nguyên):
  rbx, rbp, r12, r13, r14, r15
```

### Ví dụ C → Assembly
```c
// C:
int result = add(3, 4);
```
```asm
movl $3, %edi           # 1st arg = 3
movl $4, %esi           # 2nd arg = 4
call add
# kết quả trong eax
movl %eax, result
```

---

## 6.5 Stack Frame

```
High address
┌──────────────┐
│  caller data │
├──────────────┤ ← rbp (saved)
│  saved rbp   │
├──────────────┤ ← rbp (new)
│  local var 1 │ ← rbp - 8
│  local var 2 │ ← rbp - 16
│  ...         │
├──────────────┤ ← rsp
Low address
```

### Prologue & Epilogue
```asm
# Prologue — đầu mỗi hàm
pushq %rbp              # lưu rbp cũ
movq  %rsp, %rbp        # rbp = rsp (base của frame mới)
subq  $16, %rsp         # cấp phát 16 bytes cho local vars

# Epilogue — cuối mỗi hàm
movq  %rbp, %rsp        # khôi phục rsp
popq  %rbp              # khôi phục rbp
ret
```

---

## 6.6 Xem Assembly từ C

```bash
gcc -S -O0 program.c    # tạo file .s (không optimize)
gcc -S -O2 program.c    # với optimization
```

### Ví dụ C → Assembly
```c
int add(int a, int b) {
    return a + b;
}
```
```asm
add:
    pushq   %rbp
    movq    %rsp, %rbp
    movl    %edi, -4(%rbp)   # lưu a vào stack
    movl    %esi, -8(%rbp)   # lưu b vào stack
    movl    -4(%rbp), %edx
    movl    -8(%rbp), %eax
    addl    %edx, %eax       # eax = a + b
    popq    %rbp
    ret                      # return eax
```

### Vòng lặp trong Assembly
```c
// C:
int sum = 0;
for (int i = 0; i < 5; i++)
    sum += i;
```
```asm
    movl $0, %eax       # sum = 0
    movl $0, %ecx       # i = 0
loop:
    cmpl $5, %ecx       # so sánh i với 5
    jge  done           # if i >= 5 → thoát
    addl %ecx, %eax     # sum += i
    incl %ecx           # i++
    jmp  loop
done:
```

---

## 6.7 GDB xem Assembly

```bash
gdb ./program

disassemble main        # xem assembly hàm main
disassemble add         # xem assembly hàm add

layout asm              # hiển thị cửa sổ assembly
layout regs             # hiển thị cửa sổ registers

stepi                   # chạy từng instruction
nexti                   # như next nhưng theo instruction

info registers          # xem tất cả register
print $rax              # xem giá trị register rax
x/10x $rsp              # xem 10 words tại địa chỉ rsp
x/s 0xABCD              # xem string tại địa chỉ
```

---

## 6.8 Bài tập

1. Viết hàm `int max(int a, int b)` trong C, dùng `gcc -S` xem assembly, giải thích từng dòng.
2. Dùng GDB `disassemble` xem assembly của hàm `pushFront` trong linked list, tìm instruction tương ứng `head = newNode`.
3. Viết file `.s` assembly thuần: hàm tính tổng 1+2+...+n, gọi từ main C.
4. So sánh assembly của cùng 1 hàm khi compile `-O0` vs `-O2`, giải thích sự khác biệt.
