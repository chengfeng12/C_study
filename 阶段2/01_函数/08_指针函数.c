/*
 * 08_指针函数.c —— 阶段2 / 第 01 课 / 第 08 个文件：指针函数（返回指针的函数）
 *
 * 编译：gcc -std=c99 -Wall -Wextra 08_指针函数.c -o bin/08_指针函数
 * 运行：./bin/08_指针函数
 *
 * 中文里"指针函数"和"函数指针"两个词只是语序不同，含义正好相反。
 * 这一篇只讲前者：
 *
 *      指针函数（pointer function）= 返回指针的函数      int  *f(void);
 *      函数指针（function pointer）= 指向函数的指针      int (*f)(void);
 *
 * 函数指针放在下一篇 09 篇。这里先建立一个习惯：
 * 看到"X 函数"，中心词是"函数"；"某某"是定语。
 * 所以"指针函数"是一种函数，"函数指针"是一种指针。
 *
 * 阶段1 第 06 课第 04 篇已经打过底（函数名即地址、返回指针的几种做法），
 * 这一篇把它展开成完整的规则：什么能返回、什么不能、所有权归谁。
 *
 * ====================== 一、声明怎么读：先看 * 和谁结合 ======================
 *
 *      int  *f(void);        // 返回 int * 的函数 —— 本篇主角
 *      int (*f)(void);       // 指向"返回 int 的无参函数"的指针
 *
 * 唯一的区别是那对括号。括号把 `*f` 绑成一个整体，才变成"指针"；
 * 没有括号时 `*` 属于**返回类型**的一部分，函数本身还是函数。
 *
 * 读法（右左法则）：从名字开始，先向右看，再向左看，遇到括号掉头。
 *
 *      int *f(void)
 *          ↑ f  ← 向右遇到 (，说明 f 是函数，参数是 void
 *        ← 向左遇到 *，说明返回值是指针
 *        最后 -> 这个指针指向 int
 *        连起来：f 是一个"无参、返回 int *"的函数
 *
 *      char *(*g)(const char *)
 *          g 向右 → ( 说明 g 是指针，指向 (*g)
 *          *g 向右 → (const char *) 说明指向的是函数，参数是 const char *
 *          *g 向左 → * 说明该函数返回 char *
 *        连起来：g 是"指向函数的指针"，那个函数接收 const char *、返回 char *
 *
 * 结论：**只要返回类型里出现 *，它就是指针函数**。
 *       `void *malloc(size_t)`、`char *strcpy(char *, const char *)` 都是指针函数。
 *
 * ====================== 二、返回指针的四种合法来源 ======================
 * 返回指针本身没问题，问题是"指针指向的东西在函数返回后还在不在"。
 * 只有生命周期没结束的对象，才能安全地把地址交出去。
 *
 * (1) **静态存储期**：字符串字面量、全局变量、static 局部变量
 *         const char *grade_of(int score)
 *         {
 *             if (score >= 90) return "优秀";     // 字面量在只读段，永久有效
 *             ...
 *         }
 *     ✔ 安全。注意返回类型要写 `const char *`，因为字面量不可写。
 *
 * (2) **调用方提供的对象**：传进来的数组/缓冲区
 *         int *find_int(int *a, int n, int key)     // 返回 a 里某个元素的地址
 *     ✔ 安全。指向的是调用方的内存，生命周期由调用方掌握。
 *     ★ 这是最推荐的写法：谁的数据谁管，函数只借用。
 *
 * (3) **堆内存**：malloc 出来的
 *         char *dup_upper(const char *s);
 *     ✔ 安全但**责任转移**：调用方必须 free，否则泄漏。
 *     函数返回指针却没在文档里写"要 free"，是最常见的接口事故。
 *
 * (4) **结构体按值返回**（对照组，不是指针）
 *         struct Range minmax(const int *a, int n);
 *     ✔ 安全且没有所有权问题，结构体小的时候比返回指针更省心。
 *
 * ====================== 三、绝对不能返回局部变量的地址 ======================
 *
 *      int *bad(void)
 *      {
 *          int x = 42;
 *          return &x;        // ✗ x 在函数返回时就被销毁了
 *      }
 *
 * 为什么错：局部变量在**栈帧**里，函数返回时栈帧被回收，
 * 这块内存下一毫秒就可能被别的函数覆盖。拿到的地址是"野指针"。
 *
 * 为什么有时"看起来能跑"：
 *   - 刚返回时那块内存还没被覆盖，读出来还是 42（运气）
 *   - 多调用几层、或者开 -O2、或者换了编译器，值就变成垃圾
 *   - 这类 bug 叫**未定义行为**：不是"稳定地错"，是"有时对有时错"
 *
 * 同样不能返回的还有：
 *      返回指向局部数组的指针       int buf[10]; return buf;
 *      返回指向局部结构体成员的地址  struct S s; return &s.x;
 *      返回形参的地址               int *f(int x) { return &x; }
 *
 * 编译器一般会警告（-Wreturn-local-addr），但**别指望它**：
 * 指针转一手（先赋给变量、再返回）就可能绕过检查。
 *
 * ====================== 四、用 static 局部变量"续命"的三个代价 ======================
 *
 *      char *upper_static(const char *s)
 *      {
 *          static char buf[64];        // 静态存储期：函数返回后还在
 *          ...
 *          return buf;
 *      }
 *
 * 能编译、能运行，但有三条硬伤：
 *
 *  1. **不可重入**：下一次调用会覆盖上一次的结果
 *         char *a = upper_static("abc");
 *         char *b = upper_static("xyz");     // 此时 a 和 b 指向同一块内存
 *         printf("%s %s", a, b);             // 两个都打印 XYZ
 *
 *  2. **线程不安全**：两个线程同时调用，互相踩数据
 *
 *  3. **容量固定**：buf 只有 64 字节，超长输入要么截断、要么越界
 *
 * 什么时候可以用：单线程的小工具、返回值马上就用完、输入长度有上限。
 * 否则请改成"调用方给缓冲区"或"malloc + 调用方 free"。
 *
 * ====================== 五、标准库里的指针函数 ======================
 *
 *      char *strchr(const char *s, int c);          // 返回第一次出现 c 的地址，找不到返回 NULL
 *      char *strstr(const char *h, const char *n);  // 返回子串起始地址
 *      char *strcpy(char *dst, const char *src);    // 返回 dst
 *      void *memcpy(void *dst, const void *src, size_t n);
 *
 * 为什么 strcpy 要返回 dst？为了**链式调用**：
 *
 *      printf("%s\n", strcpy(buf, "hi"));      // 把 copy 的结果直接喂给 printf
 *      strcpy(strchr(s, ':') + 1, "x");        // 先定位再写入（要自己保证非 NULL）
 *
 * 返回"内部位置的指针"是 C 的惯用法：调用方可以继续做指针算术。
 *
 * ★ 但链式调用的代价是**没有安全检查**：
 *      strchr(s, ':') 找不到会返回 NULL，直接 +1 就是空指针算术（UB）。
 *   工程写法是分两步：
 *      char *p = strchr(s, ':');
 *      if (p != NULL) strcpy(p + 1, "x");
 *
 * ====================== 六、const 要跟着返回类型走 ======================
 *
 *      const char *name_of(int id);     // 返回只读：调用方不能改
 *      char       *buf_of(void);        // 返回可写：调用方可以改
 *
 * 两条规则：
 *   - 返回**字面量/只读数据**，必须写 `const char *`，否则调用方一写就崩（段错误）
 *   - 返回**调用方自己的缓冲区**，不要加 const，否则调用方还得强转去掉
 *
 * 函数参数同理：只读的入参写 `const char *`，既能接受字面量，也表明"我不会改你"。
 *
 * ====================== 七、常见坑 ======================
 *  1. 返回局部变量的地址            栈帧已销毁，读到的是垃圾（UB）
 *  2. 用 static 缓冲区返回          两次调用互相覆盖、线程不安全
 *  3. 返回字面量但类型没写 const    `char *f()` 返回 "abc"，调用方一改就段错误
 *  4. malloc 的返回值没人 free      每调一次泄漏一块
 *  5. 返回 NULL 但调用方不检查      紧接着解引用，直接崩溃
 *  6. 返回的指针指向数组中间        调用方 free 时地址不对（必须 free 原始指针）
 *  7. 把 `int *f()` 当成函数指针    少了一对括号，含义完全不同
 *
 * ====================== 八、实践建议 ======================
 *  - 优先"调用方传缓冲区"，函数只负责填；所有权最清晰
 *  - 必须新分配内存时，函数名里体现所有权：`dup_`、`new_`、`create_`，并在注释写明 free
 *  - 返回只读数据一律 `const char *`，让编译器帮你挡住误写
 *  - 可能失败就返回 NULL，并在注释里写清"失败条件"；调用方第一步先判空
 *  - 别用 static 缓冲区做对外接口，除非确定单线程 + 立即使用
 *  - 声明和定义都写 `(void)`，不要写空括号 `()`（C 里表示"参数未知"）
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ---- 来源 (1)：返回字符串字面量，生命周期是"整个程序" ---- */
static const char *grade_of(int score)
{
    if (score >= 90) return "优秀";
    if (score >= 60) return "及格";
    return "不及格";
}

/* ---- 来源 (2)：返回调用方数组里的某个元素地址 ---- */
static int *find_int(int *a, int n, int key)
{
    for (int i = 0; i < n; i++)
        if (a[i] == key)
            return a + i;            /* 等价于 &a[i]，但更"指针"一点 */
    return NULL;                     /* 找不到就返回空指针，调用方必须判空 */
}

/* ---- 来源 (2) 的变体：双指针游标，切分字符串，返回下一段起始地址 ---- */
static char *next_token(char **cursor)
{
    char *p = *cursor;

    if (p == NULL || *p == '\0')
        return NULL;

    while (*p == ' ')                /* 跳过前导空格 */
        p++;
    if (*p == '\0') {
        *cursor = p;
        return NULL;
    }

    char *start = p;
    while (*p != '\0' && *p != ' ')  /* 找到本段结尾 */
        p++;
    if (*p == ' ') {                 /* 用 '\0' 截断，把本段变成独立字符串 */
        *p = '\0';
        p++;
    }
    *cursor = p;                     /* 游标前移，下次从这里继续 */
    return start;
}

/* ---- 来源 (1) 的变体：static 缓冲区，能返回但不可重入 ---- */
static char *upper_static(const char *s)
{
    static char buf[64];             /* 静态存储期：函数返回后依然有效 */
    size_t i = 0;

    for (; s[i] != '\0' && i < sizeof buf - 1; i++)
        buf[i] = (char)toupper((unsigned char)s[i]);
    buf[i] = '\0';
    return buf;
}

/* ---- 来源 (3)：堆内存，所有权交给调用方（必须 free） ---- */
static char *dup_upper(const char *s)
{
    size_t n = strlen(s);
    char *p = malloc(n + 1);         /* +1 留给 '\0' */

    if (p == NULL)                   /* malloc 会失败，工程里必须检查 */
        return NULL;

    for (size_t i = 0; i < n; i++)
        p[i] = (char)toupper((unsigned char)s[i]);
    p[n] = '\0';
    return p;
}

/* ---- 来源 (4) 的对照：按值返回结构体，没有所有权问题 ---- */
struct Range {
    int min;
    int max;
};

static struct Range minmax(const int *a, int n)
{
    struct Range r = { a[0], a[0] };

    for (int i = 1; i < n; i++) {
        if (a[i] < r.min) r.min = a[i];
        if (a[i] > r.max) r.max = a[i];
    }
    return r;                        /* 返回的是副本，调用方拿到就是自己的 */
}

int main(void)
{
    /* ---------- 1. 两个词的区别 ---------- */
    printf("1. 指针函数 vs 函数指针：只差一对括号\n");
    printf("   int  *f(void);   返回 int * 的函数   ← 指针函数（本篇）\n");
    printf("   int (*f)(void);  指向函数的指针       ← 函数指针（09 篇）\n");
    printf("   读法：从名字往右，再往左，遇括号掉头\n");

    /* ---------- 2. 返回字符串字面量 ---------- */
    printf("2. 返回字面量：const char *grade_of(int)\n");
    for (int s = 95; s >= 55; s -= 40)
        printf("   grade_of(%d) = %s（返回的是只读段里的字面量地址）\n",
               s, grade_of(s));
    printf("   类型写 const char *：字面量不可写，改它直接段错误\n");

    /* ---------- 3. 返回调用方数组里的地址 ---------- */
    printf("3. 返回调用方数组内的指针：int *find_int(int *, int, int)\n");
    int nums[] = { 10, 20, 30, 40, 50 };
    int n = (int)(sizeof nums / sizeof nums[0]);

    int *hit = find_int(nums, n, 30);
    if (hit != NULL) {
        printf("   找到 30，地址偏移 = %d 个元素，值 = %d\n",
               (int)(hit - nums), *hit);
        *hit = 333;                  /* 通过返回的指针改调用方的数组 */
        printf("   通过返回的指针改完：nums[2] = %d\n", nums[2]);
    } else {
        printf("   没找到\n");
    }

    int *miss = find_int(nums, n, 999);
    printf("   找 999 → %s（找不到返回 NULL，调用方必须先判空）\n",
           miss == NULL ? "NULL" : "非空");

    /* ---------- 4. 游标式切分：返回内部位置 ---------- */
    printf("4. 双指针游标：char *next_token(char **cursor)\n");
    char line[] = "  alpha beta   gamma  ";   /* 必须是可写数组，不能是字面量 */
    char *cursor = line;
    char *tok;
    int idx = 0;

    while ((tok = next_token(&cursor)) != NULL)
        printf("   token[%d] = \"%s\"\n", idx++, tok);
    printf("   一共 %d 段：函数返回的是 line 内部的位置，没分配新内存\n", idx);

    /* ---------- 5. static 缓冲区的坑 ---------- */
    printf("5. static 缓冲区：能返回，但两次调用会互相覆盖\n");
    char *a = upper_static("abc");
    char *b = upper_static("xyz");
    printf("   a = %s, b = %s\n", a, b);
    printf("   a 和 b 是同一块静态内存（%s），所以 a 也被改成了 %s\n",
           a == b ? "地址相同" : "地址不同", a);

    /* ---------- 6. 堆内存：所有权转移给调用方 ---------- */
    printf("6. malloc 版本：char *dup_upper(const char *)\n");
    char *h = dup_upper("hello heap");
    if (h != NULL) {
        printf("   dup_upper(\"hello heap\") = %s\n", h);
        printf("   这块内存是函数借给你的，必须 free，否则每调一次泄漏一次\n");
        free(h);                     /* 谁分配谁释放：这里归调用方 */
        h = NULL;                    /* 释放后置空，避免野指针 */
    }

    /* ---------- 7. 对照：按值返回结构体 ---------- */
    printf("7. 对照：不返回指针，直接返回结构体\n");
    struct Range r = minmax(nums, n);
    printf("   minmax → { min = %d, max = %d }（返回副本，无需 free）\n",
           r.min, r.max);
    printf("   结构体小的时候，这样比返回指针更省心\n");

    /* ---------- 8. 小结 ---------- */
    printf("8. 小结：\n");
    printf("    指针函数 = 返回指针的函数；函数指针 = 指向函数的指针\n");
    printf("    返回值里带 * 就是指针函数，int *f(void) 别写成 int (*f)(void)\n");
    printf("    只能返回生命周期没结束的对象：字面量/static/调用方内存/堆\n");
    printf("    返回局部变量地址是未定义行为，别靠“看起来能跑”来判断\n");
    printf("    static 缓冲区不可重入、非线程安全，不适合做对外接口\n");
    printf("    返回 malloc 的指针，注释里必须写清“由调用方 free”\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 08_指针函数.c -o bin/08_指针函数
 *     ./bin/08_指针函数
 *
 * 输出如下：
 *
 * 1. 指针函数 vs 函数指针：只差一对括号
 *    int  *f(void);   返回 int * 的函数   ← 指针函数（本篇）
 *    int (*f)(void);  指向函数的指针       ← 函数指针（09 篇）
 *    读法：从名字往右，再往左，遇括号掉头
 * 2. 返回字面量：const char *grade_of(int)
 *    grade_of(95) = 优秀（返回的是只读段里的字面量地址）
 *    grade_of(55) = 不及格（返回的是只读段里的字面量地址）
 *    类型写 const char *：字面量不可写，改它直接段错误
 * 3. 返回调用方数组内的指针：int *find_int(int *, int, int)
 *    找到 30，地址偏移 = 2 个元素，值 = 30
 *    通过返回的指针改完：nums[2] = 333
 *    找 999 → NULL（找不到返回 NULL，调用方必须先判空）
 * 4. 双指针游标：char *next_token(char **cursor)
 *    token[0] = "alpha"
 *    token[1] = "beta"
 *    token[2] = "gamma"
 *    一共 3 段：函数返回的是 line 内部的位置，没分配新内存
 * 5. static 缓冲区：能返回，但两次调用会互相覆盖
 *    a = XYZ, b = XYZ
 *    a 和 b 是同一块静态内存（地址相同），所以 a 也被改成了 XYZ
 * 6. malloc 版本：char *dup_upper(const char *)
 *    dup_upper("hello heap") = HELLO HEAP
 *    这块内存是函数借给你的，必须 free，否则每调一次泄漏一次
 * 7. 对照：不返回指针，直接返回结构体
 *    minmax → { min = 10, max = 333 }（返回副本，无需 free）
 *    结构体小的时候，这样比返回指针更省心
 * 8. 小结：
 *     指针函数 = 返回指针的函数；函数指针 = 指向函数的指针
 *     返回值里带 * 就是指针函数，int *f(void) 别写成 int (*f)(void)
 *     只能返回生命周期没结束的对象：字面量/static/调用方内存/堆
 *     返回局部变量地址是未定义行为，别靠“看起来能跑”来判断
 *     static 缓冲区不可重入、非线程安全，不适合做对外接口
 *     返回 malloc 的指针，注释里必须写清“由调用方 free”
 * ==========================================================================
 */
