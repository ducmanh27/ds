#include <stdio.h>
typedef int (*Callback)(int, int); // declare function pointer
Callback cb;
int main()
{
    // Địa chỉ    Giá trị
    // 0x1000  →  10        ← x
    // 0x2000  →  0x1000    ← p (lưu địa chỉ của x)
    int x = 10;
    int *p = &x;
    char   *s;      // pointer tới char (string)
    void   *v;      // pointer tới bất kỳ type nào — dùng trong malloc
    int   **pp;     // pointer tới pointer
    int  (*fp)();   // function pointer

    int arr[] = {10, 20, 30, 40, 50};

    return 0;
}
