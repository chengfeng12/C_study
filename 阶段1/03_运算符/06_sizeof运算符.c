/*
 * 06_sizeof运算符.c —— 阶段1 / 第 03 课 / 第 06 个文件：sizeof 运算符
 *
 * 编译：gcc 06_sizeof运算符.c -o sizeof_demo
 * 运行：./sizeof_demo
 *
 * 编译时会看到 2 条警告，都是**故意留着**的，正是本文件要讲的两个重点：
 *     sizeof-array-argument   第 6 节：数组作参数后退化成指针
 *     unevaluated-expression  第 8 节：sizeof 里的表达式不会被执行
 * 它们不是错误，程序能正常编译运行；看懂这两条警告，sizeof 就掌握一半了。
 *
 * ====================== 一、sizeof 到底是什么 ======================
 * 它长得像函数，其实是**运算符**，而且是 C 里唯一"以字节为单位取大小"的运算符。
 * 特点：
 *      1) 关键字，不是库函数，不需要 #include
 *      2) 结果是 size_t（无符号整数类型），printf 用 %zu
 *      3) 单位是"字节"（byte），1 字节 = 8 位
 *      4) 大多数情况在**编译期**就算好了，运行时零开销
 *
 * 两种写法，效果完全一样：
 *      sizeof 变量名              →  sizeof a
 *      sizeof(变量名)             →  sizeof(a)
 *      sizeof(类型名)             →  sizeof(int)      ★ 类型必须带括号
 *
 * 记忆法：
 *      "取变量的大小"括号可省；"取类型的大小"括号不能省。
 *      因为 sizeof(int) 里的 int 不是变量，没括号就语法错误。
 *
 * ====================== 二、各种类型有多大 ======================
 * 注意：C 标准只规定"最小保证"和"大小顺序"，具体字节数跟平台有关。
 * 下面第 3 节打印的是本机（64 位）的实际情况：
 *
 *      char        1        short       2
 *      int         4        long        8(64位) / 4(32位 Windows)
 *      long long   8        float       4
 *      double      8        long double 16(x86-64) / 8(ARM)
 *      指针        8(64位)  / 4(32位)    ★ 指针大小和它指向的类型无关
 *
 * 标准保证的顺序：
 *      sizeof(char) == 1
 *      char <= short <= int <= long <= long long
 *      sizeof(float) <= sizeof(double) <= sizeof(long double)
 *
 * ====================== 三、数组：sizeof 最常用的场合 ======================
 *      int arr[10];
 *      sizeof(arr)              →  40    整个数组占的字节数
 *      sizeof(arr[0])           →  4     一个元素的字节数
 *      sizeof(arr)/sizeof(arr[0]) → 10   元素个数（经典求长度写法）
 *
 * 关键点：sizeof(数组名) 得到的是**整个数组**的大小，不是指针大小！
 *        只有这一种写法能"看穿"数组，数组一旦传给函数就退化成指针了。
 *
 * ★★ 最经典的坑：数组作函数参数
 *      void f(int arr[]) { sizeof(arr); }        // 结果是 8，不是数组总大小！
 *      void f(int arr[10]) { sizeof(arr); }      // 结果还是 8，10 是写着看的
 * 原因是形参里的数组**自动退化成指针**。所以：
 *      → 求数组长度必须用 sizeof(arr)/sizeof(arr[0]) 在**定义它的那个作用域**里算，
 *        然后把长度当参数传给函数；或者用哨兵值；或者定义成结构体包起来。
 *
 * ====================== 四、结构体：sizeof 会算上"填充字节" ======================
 * 结构体的大小 ≠ 成员大小之和，编译器为了内存对齐会插入填充（padding）：
 *
 *      struct A { char c; int i; };      // 1 + (3 填充) + 4 = 8
 *      struct B { char c; double d; };   // 1 + (7 填充) + 8 = 16
 *
 * 目的是让每个成员都落在自己对齐要求的地址上，CPU 访问更快。
 * 想让结构体更紧凑：把大成员放前面、小成员放后面（按大小降序排列）。
 *
 *      struct E { int i; char c; };      // 4 + 1 + (3 尾部填充) = 8
 *
 * 注意末尾也可能有填充：结构体整体大小要凑成"最大对齐数"的整数倍，
 * 这样结构体数组里每个元素都还是对齐的。
 *
 * ====================== 五、sizeof 不求值 ======================
 * sizeof 的操作数**不会被求值**（除了变长数组），它只是在"问类型"：
 *
 *      int i = 5;
 *      sizeof(i++)          →  4，而 i 仍然是 5，自增没发生
 *      sizeof(printf("x"))  →  4，什么都不会打印
 *
 * 所以 sizeof 里的表达式可以"随便写"，不会产生副作用，也不会执行。
 * 这也说明它是编译期行为：编译器只需要知道表达式的**类型**。
 *
 * 例外：变长数组（VLA，C99）的长度运行期才知道，
 *       这时 sizeof 会真的在运行时算，见第 10 节。
 *
 * ====================== 六、优先级：sizeof 很高 ======================
 * sizeof 是一元运算符，优先级和 ! ~ ++ -- 同级，只低于后缀运算符：
 *
 *      sizeof a + 1        →  (sizeof a) + 1     ✓ 不是 sizeof(a+1)
 *      sizeof(x) + 1       →  结果 + 1
 *      sizeof *p           →  sizeof(*p)         ✓ 一元运算符右结合
 *      sizeof(int) * 2     →  8
 *
 * 坑：
 *      sizeof a + 1      想表达"a+1 的大小"，必须写 sizeof(a + 1)
 *      sizeof(int) * p   ← 这是"字节数乘指针"，不是"大小为 int*p 的东西"
 *
 * 结论：只要不是最简单的 sizeof 变量，**一律加括号**，省得自己骗自己。
 *
 * ====================== 七、size_t 是无符号的，比较要小心 ======================
 * sizeof 的结果类型是 size_t（无符号），参与运算时会把有符号数"传染"成无符号：
 *
 *      int n = -1;
 *      if (n < sizeof(int))            // 直觉是 -1 < 4 为真，实际为假！
 *      ...                             // 因为 -1 被转成 18446744073709551615
 * 标准写法：
 *      if (n < 0 || (size_t)n < sizeof(int))
 *      if ((size_t)n < sizeof(int))    // 明确知道 n 不会是负数时
 * 打印用 %zu，不要用 %d（类型不匹配是未定义行为）：
 *      printf("%zu\n", sizeof(int));
 *
 * ====================== 八、sizeof 和 strlen 的区别 ======================
 *      char s[] = "hello";
 *      sizeof(s)   →  6     ★ 包含结尾的 '\0'
 *      strlen(s)   →  5     ★ 只数可见字符，需要 #include <string.h>
 *
 * 换成指针就更明显：
 *      char *p = "hello";
 *      sizeof(p)   →  8     ★ 指针本身的大小，和字符串多长毫无关系
 *      strlen(p)   →  5
 *
 * 一句话：sizeof 数"占多少内存"，strlen 数"字符串多长"。
 *
 * ====================== 九、常见坑总结 ======================
 *  1. 对指针用 sizeof 求"字符串/数组长度"      结果永远是指针大小
 *  2. 函数参数里 sizeof(arr)                   数组已退化成指针
 *  3. sizeof 返回 size_t 却用 %d 打印          格式不匹配
 *  4. 和有符号数比较                            无符号传染，负数变巨大正数
 *  5. sizeof a + 1 当成 sizeof(a + 1)          优先级误会
 *  6. sizeof(int) * p 本意是 sizeof(int * p)   少写括号
 *  7. 以为结构体大小 = 成员大小之和             对齐填充
 *  8. 在 malloc 里写死 sizeof(int)             建议 malloc(n * sizeof *p)
 *
 * ====================== 十、实践建议 ======================
 *  - 凡是"大小"，一律 sizeof，不要手写 4、8 这种魔法数字
 *  - 求数组元素个数：sizeof(arr) / sizeof(arr[0])
 *  - malloc 推荐写法：p = malloc(n * sizeof *p);  改类型时不用改这句
 *  - 传数组给函数时，长度也一起传：void f(int *a, size_t n)
 *  - 打印一律 %zu
 *  - 拿不准优先级就加括号，括号不要钱
 */

#include <stdio.h>
#include <string.h>     /* strlen */

/* 结构体对齐演示 */
struct A { char c; int    i; };
struct B { char c; double d; };
struct C { char a; char   b; };
struct D { int  i; char   c; };

/* 数组作参数：这里的 arr 已经不是数组，而是 int * */
static size_t bad_len(int arr[])
{
    return sizeof(arr);          /* 编译器可能会警告：这返回的是指针大小 */
}

int main(void)
{
    /* ---------- 1. 两种写法，结果一样 ---------- */
    int a = 10;
    size_t a_size_t = sizeof(a);
    printf("1. sizeof a   = %zu\n", sizeof a);
    printf("   sizeof(a)  = %zu\n", sizeof(a));
    printf("   sizeof(int)= %zu（类型必须带括号）\n", sizeof(int));
    printf("   结果类型是 size_t，所以用 %%zu 打印\n");
    printf("   类型返回 size_t: %zu", a_size_t);

    /* ---------- 2. 单位是字节 ---------- */
    printf("2. 1 字节 = 8 位；int 是 %zu 字节 = %zu 位\n",
           sizeof(int), sizeof(int) * 8);

    /* ---------- 3. 基本类型的大小 ---------- */
    printf("3. 基本类型（本机 64 位）：\n");
    printf("   char        %zu\n", sizeof(char));
    printf("   short       %zu\n", sizeof(short));
    printf("   int         %zu\n", sizeof(int));
    printf("   long        %zu\n", sizeof(long));
    printf("   long long   %zu\n", sizeof(long long));
    printf("   float       %zu\n", sizeof(float));
    printf("   double      %zu\n", sizeof(double));
    printf("   long double %zu\n", sizeof(long double));
    printf("   指针        %zu（和指向什么类型无关）\n", sizeof(void *));

    /* ---------- 4. 指针的大小都是 8，不管指向谁 ---------- */
    char        *pc = NULL;
    int         *pi = NULL;
    double      *pd = NULL;
    printf("4. sizeof(pc)=%zu sizeof(pi)=%zu sizeof(pd)=%zu\n",
           sizeof pc, sizeof pi, sizeof pd);
    printf("   指针只存地址，地址长度由 CPU 位数决定\n");

    /* ---------- 5. 数组：求总大小和元素个数 ---------- */
    int arr[10];
    printf("5. int arr[10]:\n");
    printf("   sizeof(arr)                = %zu（整个数组）\n", sizeof(arr));
    printf("   sizeof(arr[0])             = %zu（一个元素）\n", sizeof(arr[0]));
    printf("   sizeof(arr)/sizeof(arr[0]) = %zu（元素个数）\n",
           sizeof(arr) / sizeof(arr[0]));

    /* ---------- 6. 数组作函数参数会退化成指针 ---------- */
    printf("6. 传进函数后：\n");
    printf("   函数里 sizeof(arr) = %zu（已经不是数组大小！）\n", bad_len(arr));
    printf("   所以求长度必须在定义数组的地方算，再把长度传进去\n");

    /* ---------- 7. 结构体的填充字节 ---------- */
    printf("7. 结构体大小（有对齐填充，不等于成员之和）：\n");
    printf("   struct A { char; int;    } → %zu（1 + 3 填充 + 4）\n", sizeof(struct A));
    printf("   struct B { char; double; } → %zu（1 + 7 填充 + 8）\n", sizeof(struct B));
    printf("   struct C { char; char;   } → %zu（无填充）\n", sizeof(struct C));
    printf("   struct D { int;  char;   } → %zu（4 + 1 + 3 尾部填充）\n", sizeof(struct D));

    /* ---------- 8. sizeof 不会真的求值表达式 ---------- */
    int i = 5;
    size_t s = sizeof(i++);
    printf("8. sizeof(i++) = %zu，而 i 仍然是 %d（自增没执行）\n", s, i);
    printf("   sizeof 只是问类型，不执行表达式，没有副作用\n");

    /* ---------- 9. 优先级：注意括号 ---------- */
    printf("9. sizeof a + 1  = %zu（等于 (sizeof a) + 1）\n", sizeof a + 1);
    printf("   sizeof(a + 1) = %zu（这个才是 a+1 的大小）\n", sizeof(a + 1));
    int *p = &a;
    printf("   sizeof *p     = %zu（等于 sizeof(*p)，不是 sizeof(*p)*2）\n",
           sizeof *p);

    /* ---------- 10. 变长数组：sizeof 在运行期才求值 ---------- */
    int n = 5;
    int vla[n];                 /* C99 变长数组，长度由变量决定 */
    for (int k = 0; k < n; k++) vla[k] = k;
    printf("10. int vla[%d]，sizeof(vla) = %zu（运行期算：%d * %zu）\n",
           n, sizeof(vla), n, sizeof(vla[0]));
    printf("    普通数组 sizeof 在编译期就算好了，VLA 是唯一的例外\n");

    /* ---------- 11. sizeof 和 strlen 的区别 ---------- */
    char s1[] = "hello";        /* 数组：6 个字节，含结尾 '\0' */
    char *s2  = "hello";        /* 指针：只存地址 */
    printf("11. char s1[] = \"hello\":\n");
    printf("    sizeof(s1) = %zu（含 '\\0'）  strlen(s1) = %zu（只数可见字符）\n",
           sizeof(s1), strlen(s1));
    printf("    char *s2 = \"hello\":\n");
    printf("    sizeof(s2) = %zu（指针大小！）  strlen(s2) = %zu\n",
           sizeof(s2), strlen(s2));
    printf("    sizeof 数“占多少内存”，strlen 数“字符串多长”\n");

    /* ---------- 12. size_t 是无符号的 ---------- */
    int neg = -1;
    printf("12. sizeof 的结果是无符号 size_t：\n");
    printf("    直觉：-1 < 4 为%s\n", (neg < (int)sizeof(int)) ? "真" : "假");
    printf("    实际：-1 < sizeof(int) 为 %s！\n",
           ((size_t)neg < sizeof(int)) ? "真" : "假");
    printf("    因为比较时 -1 被转成 18446744073709551615，比 4 大\n");
    printf("    写比较时先检查负数，或显式转成 (int)\n");

    /* ---------- 13. malloc 的推荐写法 ---------- */
    /* 对比三种写法，见注释：
     *     int *q1 = malloc(10 * sizeof(int));      // 可以，但要同步改两处
     *     int *q2 = malloc(10 * sizeof *q2);       // ★ 推荐：改类型只改一处
     *     double *q3 = malloc(10 * sizeof *q3);    //     sizeof *q3 自带括号感
     */
    printf("13. malloc 推荐：p = malloc(n * sizeof *p)，改类型时不容易错\n");

    /* ---------- 14. 小结 ---------- */
    printf("14. 小结：\n");
    printf("    是运算符不是函数，结果是 size_t（用 %%zu 打印），单位字节\n");
    printf("    取类型必须带括号：sizeof(int)；取变量可省：sizeof a\n");
    printf("    求数组长度：sizeof(arr)/sizeof(arr[0])，且必须在原作用域算\n");
    printf("    数组作参数会退化成指针，函数里 sizeof 得到的是指针大小\n");
    printf("    结构体大小含对齐填充，不等于成员之和\n");
    printf("    sizeof 不执行操作数，编译期求值（VLA 例外）\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     gcc 06_sizeof运算符.c -o sizeof_demo
 *     ./sizeof_demo
 *
 * 输出如下（Apple Silicon macOS 实测结果）：
 *
 * 1. sizeof a   = 4
 *    sizeof(a)  = 4
 *    sizeof(int)= 4（类型必须带括号）
 *    结果类型是 size_t，所以用 %zu 打印
 * 2. 1 字节 = 8 位；int 是 4 字节 = 32 位
 * 3. 基本类型（本机 64 位）：
 *    char        1
 *    short       2
 *    int         4
 *    long        8
 *    long long   8
 *    float       4
 *    double      8
 *    long double 8（x86-64 Linux 上是 16）
 *    指针        8（和指向什么类型无关）
 * 4. sizeof(pc)=8 sizeof(pi)=8 sizeof(pd)=8
 *    指针只存地址，地址长度由 CPU 位数决定
 * 5. int arr[10]:
 *    sizeof(arr)                = 40（整个数组）
 *    sizeof(arr[0])             = 4（一个元素）
 *    sizeof(arr)/sizeof(arr[0]) = 10（元素个数）
 * 6. 传进函数后：
 *    函数里 sizeof(arr) = 8（已经不是数组大小！）
 *    所以求长度必须在定义数组的地方算，再把长度传进去
 * 7. 结构体大小（有对齐填充，不等于成员之和）：
 *    struct A { char; int;    } → 8（1 + 3 填充 + 4）
 *    struct B { char; double; } → 16（1 + 7 填充 + 8）
 *    struct C { char; char;   } → 2（无填充）
 *    struct D { int;  char;   } → 8（4 + 1 + 3 尾部填充）
 * 8. sizeof(i++) = 4，而 i 仍然是 5（自增没执行）
 *    sizeof 只是问类型，不执行表达式，没有副作用
 * 9. sizeof a + 1  = 5（等于 (sizeof a) + 1）
 *    sizeof(a + 1) = 4（这个才是 a+1 的大小）
 *    sizeof *p     = 4（等于 sizeof(*p)，不是 sizeof(*p)*2）
 * 10. int vla[5]，sizeof(vla) = 20（运行期算：5 * 4）
 *     普通数组 sizeof 在编译期就算好了，VLA 是唯一的例外
 * 11. char s1[] = "hello":
 *     sizeof(s1) = 6（含 '\0'）  strlen(s1) = 5（只数可见字符）
 *     char *s2 = "hello":
 *     sizeof(s2) = 8（指针大小！）  strlen(s2) = 5
 *     sizeof 数“占多少内存”，strlen 数“字符串多长”
 * 12. sizeof 的结果是无符号 size_t：
 *     直觉：-1 < 4 为真
 *     实际：-1 < sizeof(int) 为 假！
 *     因为比较时 -1 被转成 18446744073709551615，比 4 大
 *     写比较时先检查负数，或显式转成 (int)
 * 13. malloc 推荐：p = malloc(n * sizeof *p)，改类型时不容易错
 * 14. 小结：
 *     是运算符不是函数，结果是 size_t（用 %zu 打印），单位字节
 *     取类型必须带括号：sizeof(int)；取变量可省：sizeof a
 *     求数组长度：sizeof(arr)/sizeof(arr[0])，且必须在原作用域算
 *     数组作参数会退化成指针，函数里 sizeof 得到的是指针大小
 *     结构体大小含对齐填充，不等于成员之和
 *     sizeof 不执行操作数，编译期求值（VLA 例外）
 *
 * 平台差异提示：
 *   Windows 64 位（LLP64）上 sizeof(long) = 4、sizeof(指针) = 8；
 *   Linux / macOS 64 位（LP64）上 sizeof(long) = 8。
 *   32 位机器上指针是 4 字节，long 也常常是 4 字节。
 *   所以代码里不要写死 4 / 8，一律用 sizeof 去问。
 * ==========================================================================
 */
