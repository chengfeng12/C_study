/*
 * 33_指针与函数.c —— 阶段1 / 第 06 课 / 第 33 个文件：指针与函数
 *
 * 编译：gcc -std=c99 -Wall -Wextra 33_指针与函数.c -o bin/33_指针与函数
 * 运行：./bin/33_指针与函数
 *
 * 本篇讲三件事：指针作参数（30 篇已开个头）、**函数指针**、返回指针的函数。
 * 函数指针是 C 里最像「语法谜题」的东西，但只要抓住一句：
 *      **函数名就是地址，圆括号决定谁跟谁先结合**，
 * 所有写法就都能自己读出来。
 *
 * 本程序不打印函数的具体地址（每次运行都不同），只打印调用结果。
 *
 * ====================== 一、函数名就是地址 ======================
 * 编译出来的每个函数都占一段代码空间，有自己的入口地址。
 * 在表达式里，函数名会退化成「指向该函数的指针」，和数组名退化是一个道理。
 *
 *      int add(int, int);
 *
 *      add                 类型 int (*)(int, int)     函数的地址
 *      &add                同上，加 & 也行（多余但合法）
 *      *add                同上，加 * 也行（历史遗留，等效）
 *      add(1, 2)           调用
 *      (*add)(1, 2)        调用，等价
 *
 * 所以下面三种定义方式完全一样：
 *
 *      int (*fp)(int, int) = add;
 *      int (*fp)(int, int) = &add;
 *      int (*fp)(int, int) = *add;
 *
 * ====================== 二、怎么读一个函数指针声明 ======================
 * 用「右左法则」：从变量名出发，先看右边，再看左边。
 *
 *      int (*fp)(int, int);
 *           ↑
 *      1) 括号里 *fp → fp 是一个指针
 *      2) 右边的 (int, int) → 指向的是函数，参数两个 int
 *      3) 左边的 int → 该函数返回 int
 *      合起来：fp 是指针，指向「参数 (int,int)、返回 int」的函数
 *
 * 对比几个容易混的：
 *
 *      int *f(int);          // 函数，返回 int *        （f 是函数）
 *      int (*f)(int);        // 指针，指向返回 int 的函数（f 是指针）
 *      int (*f[3])(int);     // 数组，3 个函数指针       （f 是数组）
 *      int (*(*f)(int))(int);// 指针，指向「返回函数指针的函数」（别写这种东西）
 *
 * 记忆：**括号把 * 和变量名绑在一起，它就是指针；否则它先跟右边的 () 结合，就是函数。**
 *
 * ====================== 三、typedef 让声明变可读 ======================
 *      typedef int (*binop)(int, int);     // binop 是一个「类型名」
 *
 * 之后写 `binop f = add;`、`binop table[4];` 就清爽多了。
 * 标准库自己就是这么干的（比如 signal.h 里的 sighandler_t）。
 *
 * ====================== 四、函数指针作参数 = 回调（callback） ======================
 * 把「做什么」当参数传进函数，这叫回调。C 标准库到处都是：
 *
 *      qsort(base, n, size, cmp);           // 排序规则由 cmp 决定
 *      bsearch(key, base, n, size, cmp);    // 查找规则同理
 *      signal(SIGINT, handler);             // 事件来了调 handler
 *
 * 手写一个：
 *
 *      int reduce(const int *a, int n, int init, binop op)
 *      {
 *          int acc = init;
 *          for (int i = 0; i < n; i++)
 *              acc = op(acc, a[i]);         // 这里不知道 op 具体是谁
 *          return acc;
 *      }
 *
 * 同一个 reduce，传 add 就是求和，传 mul 就是连乘 —— 这就是「行为参数化」。
 * C 没有泛型、没有 lambda，靠的就是函数指针。
 *
 * ====================== 五、qsort：函数指针的经典应用 ======================
 *      void qsort(void *base, size_t n, size_t size,
 *                 int (*cmp)(const void *, const void *));
 *
 * ★ 先认识 void *：它是"只有地址、没有类型信息"的指针，标准库靠它实现
 *   泛型接口（qsort 的 base 和 cmp 参数都是它）。这里只要记住两条：
 *     **不能直接解引用**（不知道读几个字节）、**转回的类型必须和原本一致**。
 *   完整规则（为什么不能做算术、强转的边界在哪）见 37 篇，本篇不必深究。
 *
 * 要点：
 *  1) base 是 void *，size 是「单个元素多大」，必须自己传对
 *  2) cmp 收两个 const void *，你要先转成真实类型再取值：
 *         int x = *(const int *)pa;
 *  3) cmp 的返回值约定：负数表示 a < b，0 相等，正数表示 a > b
 *  4) 别写 `return x - y;` —— int 相减可能溢出。
 *     写成 `(x > y) - (x < y)`，结果只可能是 -1/0/1，绝对安全
 *  5) 排序字符串数组时，元素类型是 char *，所以参数要转成 char **：
 *         const char *const *x = (const char *const *)pa;
 *
 * ====================== 六、返回指针的函数 ======================
 * 三种做法，一种绝对不行：
 *
 *  (1) ✗ 返回局部变量的地址
 *          int *bad(void) { int a[3] = {1,2,3}; return a; }
 *      函数一返回，a 所在的空间就被回收，返回的地址立刻悬垂。
 *      编译器会警告（-Wreturn-stack-address），但写惯了很容易忽略。
 *
 *  (2) △ 返回 static 数组
 *          int *ok(void) { static int d[3] = {1,2,3}; return d; }
 *      地址有效（活到程序结束），但**所有调用共用同一块**：
 *      上一次的结果会被下一次覆盖，多线程下更不能用。
 *
 *  (3) ✓ 调用方传缓冲区进来（最推荐）
 *          void fill(int *out, int n);
 *      谁申请谁负责，没有生命周期歧义，也没有共享状态。
 *
 *  补充：返回字符串字面量（`return "ok";`）是安全的 —— 字面量活在只读段，
 *  程序结束前一直有效。只是不能改它。
 *
 * ====================== 七、常见坑 ======================
 *  1. 函数指针没初始化就用            野指针，调用直接崩
 *  2. 声明里少写括号                  int *f(int) 是函数，不是函数指针
 *  3. 函数指针类型不匹配                参数/返回值不同却硬转，行为未定义
 *  4. 忘记给 qsort 传 size              传成指针大小，排序结果全乱
 *  5. cmp 返回值直接用减法              大数相减溢出，排序结果错
 *  6. 返回局部变量的地址                悬垂指针
 *  7. 以为 static 返回值可以随便用       所有调用共享一块，会被覆盖
 *  8. 回调里改全局状态                  难以测试，也容易重入出错
 *  9. 调用空函数指针                    等同空指针解引用，必崩
 * 10. 用 == 比较两个函数指针            只有「同一个函数」才为真，意义有限
 *
 * ====================== 八、实践建议 ======================
 *  - 用 typedef 给函数指针起名字，别到处写 `int (*)(int, int)`
 *  - 回调参数顺序统一：业务数据在前、回调在后（和 qsort 一致）
 *  - cmp 一律返回 -1 / 0 / 1，不要返回差值
 *  - 需要「返回一块数据」时，优先让调用方传缓冲区
 *  - 函数指针表（dispatch table）是替代长 switch 的好办法
 *  - 调用前判空：`if (cb != NULL) cb(...);`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int add(int a, int b)  { return a + b; }
static int sub(int a, int b)  { return a - b; }
static int mul(int a, int b)  { return a * b; }
static int idiv(int a, int b) { return (b != 0) ? a / b : 0; }

/* 给「接收两个 int、返回 int」的函数指针起个类型名 */
typedef int (*binop)(int, int);

/* 回调：把「做什么运算」当参数传进来，reduce 自己不知道 op 是谁 */
static int reduce(const int *a, int n, int init, binop op)
{
    int acc = init;
    for (int i = 0; i < n; i++)
        acc = op(acc, a[i]);
    return acc;
}

/* qsort 的比较函数：参数是 const void *，必须先转回真实类型 */
static int cmp_int(const void *pa, const void *pb)
{
    int x = *(const int *)pa;
    int y = *(const int *)pb;
    return (x > y) - (x < y);            /* 只返回 -1/0/1，不会溢出 */
}

/* 排序字符串数组：元素类型是 char *，所以参数要按 char ** 解释 */
static int cmp_str(const void *pa, const void *pb)
{
    const char *const *x = (const char *const *)pa;
    const char *const *y = (const char *const *)pb;
    return strcmp(*x, *y);
}

/* ✓ 返回 static 数组：有效，但所有调用共用同一块 */
static int *digits_static(void)
{
    static int d[3] = {7, 8, 9};
    return d;
}

/* ✓ 调用方给缓冲区：最推荐的写法 */
static void fill_squares(int *out, int n)
{
    for (int i = 0; i < n; i++)
        out[i] = i * i;
}

/* ✗ 反面教材，只写在注释里：
 *    static int *bad(void) { int a[3] = {1, 2, 3}; return a; }
 *    函数返回后 a 已销毁，返回的地址是悬垂指针。
 */

int main(void)
{
    /* ---------- 1. 函数名就是地址 ---------- */
    printf("1. 函数名本身代表地址，三种初始化写法等价\n");
    int (*fp)(int, int) = add;           /* ① 直接写函数名 */
    int (*fq)(int, int) = &add;          /* ② 加 & 也行 */
    int (*fr)(int, int) = *add;          /* ③ 加 * 也行（等效，历史遗留） */
    printf("   fp(3, 4)   = %d    ← 用 add 初始化\n", fp(3, 4));
    printf("   fq(1, 1)   = %d    ← 用 &add 初始化\n", fq(1, 1));
    printf("   (*fr)(2, 2)= %d    ← 用 *add 初始化，调用时加 * 也等价\n", (*fr)(2, 2));

    /* ---------- 2. 右左法则读声明 ---------- */
    printf("2. int (*fp)(int, int) 怎么读：先 *fp，再右边，再左边\n");
    printf("   ① 括号里 *fp → fp 是指针\n");
    printf("   ② 右边 (int, int) → 指向函数，两个 int 参数\n");
    printf("   ③ 左边 int → 该函数返回 int\n");
    printf("   对比 int *f(int)：f 是函数，返回 int *（括号一去掉就换了身份）\n");

    /* ---------- 3. 回调：把行为当参数传 ---------- */
    printf("3. 函数指针作参数 = 回调\n");
    int nums[5] = {3, 1, 4, 1, 5};
    printf("   数据：3 1 4 1 5\n");
    printf("   reduce(nums, 5, 0, add)   = %d（累加）\n", reduce(nums, 5, 0, add));
    printf("   reduce(nums, 5, 1, mul)   = %d（连乘）\n", reduce(nums, 5, 1, mul));
    printf("   reduce(nums, 5, 100, sub) = %d（从 100 开始依次减）\n",
           reduce(nums, 5, 100, sub));
    printf("   同一个 reduce，换个函数就换个算法 —— 这就是「行为参数化」\n");

    /* ---------- 4. 函数指针数组：分发表 ---------- */
    printf("4. 函数指针数组 = 一张分发表，替代长 switch\n");
    binop table[4] = {add, sub, mul, idiv};
    const char *symbol[4] = {"+", "-", "*", "/"};
    int lhs = 12, rhs = 4;
    for (int i = 0; i < 4; i++)
        printf("   %2d %s %d = %d\n", lhs, symbol[i], rhs, table[i](lhs, rhs));
    printf("   table[i] 就是函数指针，用法和普通数组元素一样\n");

    /* ---------- 5. qsort：库函数的回调 ---------- */
    printf("5. qsort 的最后一个参数就是函数指针\n");
    int v[6] = {5, 2, 9, 1, 7, 3};
    printf("   排序前：");
    for (int i = 0; i < 6; i++) printf("%d ", v[i]);
    printf("\n");
    qsort(v, 6, sizeof v[0], cmp_int);
    printf("   排序后：");
    for (int i = 0; i < 6; i++) printf("%d ", v[i]);
    printf("\n");
    char *words[4] = {"pear", "apple", "fig", "banana"};
    qsort(words, 4, sizeof words[0], cmp_str);
    printf("   字符串按字典序：%s %s %s %s\n",
           words[0], words[1], words[2], words[3]);
    printf("   cmp 的参数是 const void *：先转回真实类型才能取值（详见 37 篇）\n");
    printf("   cmp 返回 -1/0/1，不要用 x - y（大数相减会溢出）\n");

    /* ---------- 6. 返回指针的函数 ---------- */
    printf("6. 返回指针的三种做法\n");
    int *d = digits_static();
    printf("   ✓ static 数组：d[0..2] = %d %d %d\n", d[0], d[1], d[2]);
    printf("     注意所有调用共用同一块，下一次调用会覆盖上一次的结果\n");
    int out[4];
    fill_squares(out, 4);
    printf("   ✓ 调用方传缓冲区：out = %d %d %d %d（最推荐）\n",
           out[0], out[1], out[2], out[3]);
    printf("   ✗ 返回局部数组 int *f(void){int a[3]; return a;} → 返回即悬垂\n");
    printf("   △ 返回字符串字面量是安全的（活在只读段），但不能改\n");

    /* ---------- 7. 小结 ---------- */
    printf("7. 小结：\n");
    printf("    函数名就是地址，add、&add、*add 三者等价\n");
    printf("    int (*fp)(int,int) 读作「fp 是指针，指向函数」\n");
    printf("    函数指针作参数 = 回调，qsort 是最常见的例子\n");
    printf("    函数指针数组可以当分发表，替掉一大串 switch\n");
    printf("    返回指针别返回局部变量地址，让调用方传缓冲区最安全\n");
    printf("    回调调用前判空，cmp 一律返回 -1/0/1\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 33_指针与函数.c -o bin/33_指针与函数
 *     ./bin/33_指针与函数
 *
 * 输出如下：
 *
 * 1. 函数名本身代表地址，三种初始化写法等价
 *    fp(3, 4)   = 7    ← 用 add 初始化
 *    fq(1, 1)   = 2    ← 用 &add 初始化
 *    (*fr)(2, 2)= 4    ← 用 *add 初始化，调用时加 * 也等价
 * 2. int (*fp)(int, int) 怎么读：先 *fp，再右边，再左边
 *    ① 括号里 *fp → fp 是指针
 *    ② 右边 (int, int) → 指向函数，两个 int 参数
 *    ③ 左边 int → 该函数返回 int
 *    对比 int *f(int)：f 是函数，返回 int *（括号一去掉就换了身份）
 * 3. 函数指针作参数 = 回调
 *    数据：3 1 4 1 5
 *    reduce(nums, 5, 0, add)   = 14（累加）
 *    reduce(nums, 5, 1, mul)   = 60（连乘）
 *    reduce(nums, 5, 100, sub) = 86（从 100 开始依次减）
 *    同一个 reduce，换个函数就换个算法 —— 这就是「行为参数化」
 * 4. 函数指针数组 = 一张分发表，替代长 switch
 *    12 + 4 = 16
 *    12 - 4 = 8
 *    12 * 4 = 48
 *    12 / 4 = 3
 *    table[i] 就是函数指针，用法和普通数组元素一样
 * 5. qsort 的最后一个参数就是函数指针
 *    排序前：5 2 9 1 7 3
 *    排序后：1 2 3 5 7 9
 *    字符串按字典序：apple banana fig pear
 *    cmp 的参数是 const void *：先转回真实类型才能取值（详见 37 篇）
 *    cmp 返回 -1/0/1，不要用 x - y（大数相减会溢出）
 * 6. 返回指针的三种做法
 *    ✓ static 数组：d[0..2] = 7 8 9
 *      注意所有调用共用同一块，下一次调用会覆盖上一次的结果
 *    ✓ 调用方传缓冲区：out = 0 1 4 9（最推荐）
 *    ✗ 返回局部数组 int *f(void){int a[3]; return a;} → 返回即悬垂
 *    △ 返回字符串字面量是安全的（活在只读段），但不能改
 * 7. 小结：
 *     函数名就是地址，add、&add、*add 三者等价
 *     int (*fp)(int,int) 读作「fp 是指针，指向函数」
 *     函数指针作参数 = 回调，qsort 是最常见的例子
 *     函数指针数组可以当分发表，替掉一大串 switch
 *     返回指针别返回局部变量地址，让调用方传缓冲区最安全
 *     回调调用前判空，cmp 一律返回 -1/0/1
 *
 * 几个容易忽略的对照点：
 *   第 1 条：三种初始化（add / &add / *add）结果完全一样，
 *           说明"函数名本身就是一个地址"；同理调用时 fp(3,4) 与 (*fp)(3,4) 等价。
 *           这条和数组名退化是同一套规则，C 里"名字→地址"随处可见。
 *   第 3 条：三组数字全来自同一个 reduce：
 *           0+3+1+4+1+5 = 14，1×3×1×4×1×5 = 60，100-3-1-4-1-5 = 86。
 *           reduce 里没有一行 if，选哪个算法完全由传进来的函数决定。
 *   第 4 条：table[i](lhs, rhs) 这种写法一开始看着怪，
 *           其实就是"先从数组里取出函数指针，再调用"，等价 (*table[i])(lhs, rhs)。
 *   第 5 条：qsort 不知道元素类型，所以必须传 size（每个元素多大），
 *           再由 cmp_int 把它转回 int* —— void * 是"类型擦除"的关键。
 *           字符串排序的 cmp_str 要转成 char **，因为元素本身就是指针。
 *   第 6 条：static 版本能返回，是因为 d 活到程序结束；
 *           如果写成"返回局部数组"，函数一返回地址就失效（悬垂指针）。
 * ==========================================================================
 */
