#include <stdio.h>
#include <errno.h>
// Mọi chương trình C đều có 3 stream mặc định:
// stdin  (fd=0) ← bàn phím (hoặc pipe)
// stdout (fd=1) → màn hình (hoặc file)
// stderr (fd=2) → màn hình (không buffer — in ngay lập tức)

void examplePrinf() {
    // Cú pháp: %[flags][width][.precision]type

    // Type
    /*%d   %i    // int (decimal)
        %u         // unsigned int
        %ld        // long
        %lld       // long long
        %f         // float/double (decimal)
        %e         // scientific notation: 3.14e+00
        %g         // tự chọn %f hay %e tùy giá trị
        %c         // char
        %s         // string
        %p         // pointer (địa chỉ)
        %x   %X    // hex (thường / hoa)
        %o         // octal
        %zu        // size_t
        %%         // in dấu %*/

    // Width & Precision
    printf("%10d\n",   42);     // "        42"  (right-align, rộng 10)
    printf("%-10d\n",  42);     // "42        "  (left-align)
    printf("%010d\n",  42);     // "0000000042"  (pad bằng 0)
    printf("%.3f\n",   3.14159); // "3.142"      (3 chữ số thập phân)
    printf("%8.2f\n",  3.14159); // "    3.14"   (rộng 8, 2 decimal)
    printf("%.5s\n",  "Hello World"); // "Hello" (cắt string)
}

void exampleScanf() {
    int x;
    float f;
    char s[100];

    scanf("%d", &x);           // đọc int — nhớ &
    scanf("%f", &f);           // đọc float
    scanf("%s", s);            // đọc word (dừng ở space) — không cần & với mảng
    scanf("%99s", s);          // giới hạn 99 ký tự — an toàn hơn
    int a, b;
    scanf("%d %d", &a, &b);    // đọc 2 int cách nhau bởi space

    // Vấn đề của scanf
    char c[10];
    scanf("%d", &x);
    scanf("%c", &c);   // đọc '\n' còn thừa trong buffer!

    // Fix: dùng getchar() để xóa '\n'
    scanf("%d", &x);
    getchar();
    scanf("%c", &c);

    // Hoặc dùng fgets — an toàn hơn
    char line[256];
    fgets(line, sizeof(line), stdin);   // đọc cả dòng kể cả space
    // fgets giữ lại '\n' ở cuối — cần trim nếu cần
    line[strcspn(line, "\n")] = '\0';   // xóa '\n'
}

int main()
{
    // Mở file
    FILE *f = fopen("data1.txt", "r");
    // Mode:
    // "r"  — đọc (file phải tồn tại)
    // "w"  — ghi (tạo mới hoặc xóa cũ)
    // "a"  — append (thêm vào cuối)
    // "r+" — đọc + ghi
    // "rb" — đọc binary
    // "wb" — ghi binary
    if (f == NULL) {
        perror("fopen");    // in lỗi chi tiết: "fopen: No such file or directory"
        exit(1);
    }

    // Đọc từng dòng
    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';   // trim '\n'
        printf("Line: %s\n", line);
    }

    // Đọc theo format
    int age; char name[50];
    while (fscanf(f, "%d %s", &age, name) == 2) {
        printf("%s is %d\n", name, age);
    }


    // Ghi file
    FILE *out = fopen("output.txt", "w");
    fprintf(out, "Hello %s, age %d\n", name, age);

    // // Đọc/ghi binary
    // int arr[5] = {1,2,3,4,5};
    // fseek(f, 0, SEEK_END);    // về cuối
    // size_t len = fwrite(arr, sizeof(int), 5, f);    // ghi 5 int
    // if (len <= 0) {
    //     printf("write error: %s\n", strerror(errno));
    //     fflush(stdout);
    // }
    // fseek(f, 0, SEEK_END);    // về cuối
    // fread(arr,  sizeof(int), 5, f);    // đọc 5 int

    fclose(f);    // LUÔN đóng

    return 0;
}


// #include <string.h>

// // Độ dài
// size_t len = strlen("Hello");    // 5 (không tính '\0')

// // Copy
// strcpy(dst, src);                 // nguy hiểm — không kiểm tra size
// strncpy(dst, src, sizeof(dst)-1); // an toàn hơn
// dst[sizeof(dst)-1] = '\0';        // strncpy không đảm bảo null-terminate

// // Nối
// strcat(dst, src);
// strncat(dst, src, n);

// // So sánh
// strcmp(s1, s2)     // 0=bằng, <0=s1<s2, >0=s1>s2
//     strncmp(s1, s2, n) // so sánh n ký tự đầu
//     strcasecmp(s1, s2) // không phân biệt hoa thường (POSIX)

//     // Tìm kiếm
//     char *p = strchr(s, 'c');      // tìm ký tự đầu tiên → pointer hoặc NULL
// char *p = strrchr(s, 'c');     // tìm ký tự cuối cùng
// char *p = strstr(s, "sub");    // tìm substring
// char *tok = strtok(s, ",");    // tách chuỗi theo delimiter
// while (tok != NULL) {
//     printf("%s\n", tok);
//     tok = strtok(NULL, ",");   // tiếp tục từ chỗ dừng
// }

// // Copy vùng nhớ (bất kỳ type)
// memcpy(dst, src, n);      // copy n bytes — vùng nhớ KHÔNG được overlap
// memmove(dst, src, n);     // an toàn hơn — cho phép overlap
// memset(arr, 0, sizeof(arr));   // set n bytes về 0
// memcmp(a, b, n);          // so sánh n bytes


// #include <stdlib.h>

// // Chuyển đổi string → số
// int    x = atoi("42");          // string to int
// double d = atof("3.14");        // string to double
// long   l = strtol("42", NULL, 10);   // an toàn hơn atoi, base 10
// // strtol trả về error nếu không convert được

// // Random
// srand(time(NULL));              // seed (cần #include <time.h>)
// int r = rand() % 100;           // 0-99
// int r = rand() % (max - min + 1) + min;  // min đến max

// // Thoát chương trình
// exit(0);     // thoát bình thường
// exit(1);     // thoát do lỗi
// abort();     // thoát ngay, tạo core dump

// // Sort & Search
// qsort(arr, n, sizeof(int), compare_func);
// void *found = bsearch(&key, arr, n, sizeof(int), compare_func);

// // Math (math.h — cần link -lm)
// #include <math.h>
// sqrt(x), pow(x,y), abs(x), fabs(x)
//     ceil(x), floor(x), round(x)


// stdout: line-buffered (terminal) hoặc fully-buffered (file/pipe)
// stderr: unbuffered — in ngay lập tức

// printf("Loading...");   // có thể chưa in ra màn hình!
// // Fix 1: thêm \n
// printf("Loading...\n");
// // Fix 2: flush thủ công
// fflush(stdout);
// // Fix 3: dùng stderr
// fprintf(stderr, "Loading...");

// // Ứng dụng thực tế: progress bar
// for (int i = 0; i <= 100; i++) {
//     printf("\rProgress: %d%%", i);
//     fflush(stdout);
//     // sleep(1);
// }
