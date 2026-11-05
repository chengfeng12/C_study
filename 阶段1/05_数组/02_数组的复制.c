/*
 * 02_数组的复制.c —— 阶段1 / 第 05 课 / 第 02 个文件：数组的复制（循环 / memcpy / memmove）
 *
 * 编译：gcc 02_数组的复制.c -o bin/02_数组的复制
 * 运行：./bin/02_数组的复制
 *
 * 本文件能干净编译（-Wall -Wextra 零警告）。
 * 第 1 节的 `b = a` 是错误的，只写在注释里（真写上去编译不过）。
 * 第 9 节演示"memcpy 长度写错"，但先把目标数组清零，输出才可预期。
 *
 * ====================== 一、为什么不能写 b = a ======================
 * 数组名代表首地址（01 篇第 6 节），它是**地址常量**，不是"一包数据"：
 *
 *      int a[5] = {1,2,3,4,5};
 *      int b[5];
 *      b = a;                  // ✗ 编译错误：数组类型不可赋值
 *
 * 编译器报的通常是：
 *      error: assignment to expression with array type
 *
 * 原因：C 没有"把 a 这块内存整体搬到 b"的运算符。= 只做**值复制**，
 *      而数组名的"值"是地址，地址又不可改（地址常量）。
 *
 * 所以复制必须**显式搬运字节**：要么自己循环，要么调 memcpy。
 *
 * ====================== 二、四种复制方式 ======================
 *
 *  (1) 循环逐元素（最基础，能顺便做转换/过滤）
 *
 *          for (int i = 0; i < n; i++)
 *              dst[i] = src[i];
 *
 *      优点：完全可控，可以在循环里做加法、过滤、转换；
 *      缺点：代码长一点，语义上不如 memcpy 直白。
 *
 *  (2) memcpy：整块搬字节（最常用）
 *
 *          #include <string.h>
 *          memcpy(dst, src, n * sizeof(src[0]));      // 推荐写法
 *          memcpy(dst, src, sizeof(src));             // src 是数组时等价
 *
 *      参数：目标、源、**字节数**（不是元素个数！这是第一大坑）。
 *      要求：**两块内存不能重叠**，否则行为未定义。
 *
 *  (3) memmove：允许重叠（同一数组内挪动必用它）
 *
 *          memmove(a, a + 1, 5 * sizeof(a[0]));       // 整体左移一位
 *
 *      比 memcpy 多做一步"检测方向"，所以略慢一点点，但对重叠安全。
 *      规则：**不确定会不会重叠，就用 memmove。**
 *
 *  (4) 用结构体包一层，享受整体赋值
 *
 *          struct Box { int v[5]; };
 *          struct Box p = {{1,2,3,4,5}};
 *          struct Box q = p;                  // ✓ 结构体可以整体赋值
 *
 *      结构体赋值 = 逐字节拷贝成员，所以里面的数组会被完整复制。
 *      这是个常见小技巧，但会多一层包装，读起来不如 memcpy 直白。
 *
 * ====================== 三、复制前必须想清楚的三件事 ======================
 *
 *  (1) 目标够不够大？
 *          int dst[3];
 *          memcpy(dst, src, 5 * sizeof(int));   // ✗ 写爆 dst，越界
 *      目标数组的元素个数必须 ≥ 源要复制的元素个数。
 *
 *  (2) 长度算的是字节还是元素？
 *          memcpy(dst, src, 5);                 // ✗ 只搬了 5 字节 = 1 个 int
 *          memcpy(dst, src, 5 * sizeof(int));   // ✓ 20 字节
 *      记住：**sizeof 求的是字节，元素个数要自己乘。**
 *
 *  (3) 两块内存会不会重叠？
 *          同一数组内 `a[0] = a[1]` 这种挪动，重叠区间必须用 memmove。
 *
 * ====================== 四、复制的是"值"，不是"引用" ======================
 * C 的数组没有"引用/别名"的概念（除了用指针主动去指）：
 *
 *      int a[3] = {1, 2, 3};
 *      int b[3];
 *      for (int i = 0; i < 3; i++) b[i] = a[i];
 *      b[0] = 99;                  // 只改 b
 *      // a[0] 仍然是 1
 *
 * 复制完是**两份独立的内存**，改谁都不影响另一个。
 * 但如果只是 `int *p = a;`，那是"两个名字指同一块内存"，改 p[0] 就是改 a[0]。
 *
 *      复制内容（值语义） → 循环 / memcpy / memmove
 *      共享内存（引用语义）→ 指针赋值
 * 这两件事完全不是一回事，别混。
 *
 * ====================== 五、字符串数组的复制 ======================
 *      char s1[] = "hello";        // 6 字节，含结尾 '\0'
 *      char s2[10];
 *
 *      strcpy(s2, s1);            // ✓ 会把 '\0' 一起带过去
 *      memcpy(s2, s1, strlen(s1));// ✗ 少了 '\0'，s2 变成"没有结尾的字符数组"
 *      memcpy(s2, s1, strlen(s1) + 1);   // ✓ 加 1 才是完整字符串
 *
 * 三个长度别搞混：
 *      strlen(s1)      = 5   可见字符数
 *      sizeof(s1)      = 6   数组字节数（含 '\0'）
 *      sizeof(char *)  = 8   指针大小（s1 退化成指针后）
 *
 * 安全提醒：strcpy 不检查目标容量，目标不够会溢出；
 *          实际工程里用 strncpy / snprintf 并保证手工补 '\0'。
 *
 * ====================== 六、常见坑 ======================
 *  1. 写 b = a                   数组不可赋值，编译错误
 *  2. memcpy 第三个参数写成元素个数  只搬了"元素数"这么多字节
 *  3. 目标数组比源小              溢出，破坏别的变量，可能崩
 *  4. memcpy 处理重叠内存          未定义行为，要用 memmove
 *  5. 复制字符串忘记 '\0'          strlen+1，或直接用 strcpy
 *  6. 用 sizeof(指针) 算长度       函数里 sizeof 是 8（退化），不是数组大小
 *  7. 以为 memcpy 会"智能转换类型"  它只搬字节，int 复制到 double 会得到乱码
 *  8. 复制后以为两个数组联动       值复制就是两份独立内存
 *  9. 只复制了一半数据就当真        比如循环写了 i <= n/2
 * 10. 忘记 #include <string.h>    memcpy/memmove 的隐式声明警告（C99 起是错误）
 *
 * ====================== 七、性能与选择 ======================
 *      memcpy   通常最快：编译器/库会用 SIMD、按字长搬，还可能内联展开
 *      循环     编译器经常能把它优化成 memcpy；可读性更好、能加逻辑
 *      memmove  比 memcpy 多一次方向判断，重叠场景才用它
 *
 * 实测建议：
 *      纯搬运 → memcpy（长度用 sizeof 或 n * sizeof(元素)）
 *      要转换/过滤 → 循环
 *      同数组内挪动 → memmove
 *      字符串 → strcpy / snprintf（记得 '\0'）
 *
 * 一句话：**先选对的，再谈快；绝大多数场景两者的差别可以忽略。**
 *
 * ====================== 八、实践建议 ======================
 *  - 复制前先写清楚三件事：目标容量、复制多少、是否重叠
 *  - 长度统一用 `sizeof(src) / sizeof(src[0])` 求，别手写数字
 *  - 数组传参后退化成指针，**长度必须一起传**
 *  - 字符串用 strcpy/strncpy，别用 memcpy 手工算 '\0'
 *  - 大数组复制前先想"真的需要复制吗"，能传指针就别复制
 *  - 复制出错往往不报错只出错结果，必要时用 memcmp 验证
 */

#include <stdio.h>
#include <string.h>          /* memcpy / memmove / strcpy / memset / memcmp */

/* 结构体包数组：可以整体赋值 */
struct Box {
    int v[4];
    int n;
};

int main(void)
{
    /* ---------- 1. 不能直接赋值 ---------- */
    printf("1. b = a 不行：数组名是地址常量，不能整体赋值\n");
    int a[5] = {1, 2, 3, 4, 5};
    int b[5] = {0};
    /* b = a;    // ← 真这么写会编译报错：assignment to expression with array type */
    printf("   若写 b = a; 编译器报 assignment to expression with array type\n");
    printf("   复制必须显式搬字节：循环或 memcpy\n");

    /* ---------- 2. 循环逐元素复制 ---------- */
    printf("2. 循环复制：一个元素一个元素搬\n");
    for (int i = 0; i < 5; i++)
        b[i] = a[i];
    printf("   复制后 b = ");
    for (int i = 0; i < 5; i++) printf("%d ", b[i]);
    printf("\n");

    b[0] = 99;
    printf("   把 b[0] 改成 99 后：a[0] = %d，b[0] = %d\n", a[0], b[0]);
    printf("   两份独立内存，改一个不影响另一个\n");

    /* ---------- 3. memcpy 整块复制 ---------- */
    printf("3. memcpy：一次搬完一整块\n");
    int c[5];
    memcpy(c, a, sizeof(a));                 /* sizeof(a) = 20 字节 */
    printf("   memcpy(c, a, sizeof(a)) 后 c = ");
    for (int i = 0; i < 5; i++) printf("%d ", c[i]);
    printf("\n   sizeof(a) = %zu 字节 = 5 个元素 × %zu 字节\n",
           sizeof(a), sizeof(a[0]));
    printf("   用 memcmp 验证内容一致：%s\n",
           (memcmp(a, c, sizeof(a)) == 0) ? "完全相同" : "不同");

    /* ---------- 4. memmove：允许重叠 ---------- */
    printf("4. memmove：同一数组内部挪动（内存重叠）\n");
    int m[6] = {10, 20, 30, 40, 50, 60};
    printf("   原始 m = ");
    for (int i = 0; i < 6; i++) printf("%d ", m[i]);
    memmove(m, m + 1, 5 * sizeof(m[0]));      /* 整体左移一位，源和目标重叠 */
    printf("\n   左移一位后 m = ");
    for (int i = 0; i < 6; i++) printf("%d ", m[i]);
    printf("\n   源 m+1 与目标 m 有重叠区间，所以必须用 memmove 而不是 memcpy\n");
    printf("   末尾还有一个残留的 60：只搬了 5 个元素，第 6 个位置没被覆盖\n");

    /* ---------- 5. 部分复制：只要中间几个 ---------- */
    printf("5. 部分复制：从 src[2] 起拿 3 个元素\n");
    int src[6] = {1, 2, 3, 4, 5, 6};
    int sub[3] = {0, 0, 0};
    memcpy(sub, src + 2, 3 * sizeof(src[0])); /* 起点 src+2，3 个元素 */
    printf("   src = ");
    for (int i = 0; i < 6; i++) printf("%d ", src[i]);
    printf("\n   sub = ");
    for (int i = 0; i < 3; i++) printf("%d ", sub[i]);
    printf("（取的是 src[2]~src[4]）\n");

    /* ---------- 6. 字符串数组的复制 ---------- */
    printf("6. 字符串：strcpy 会自带 '\\0'，memcpy 要自己加 1\n");
    char s1[] = "hello";
    char s2[10] = {0};
    char s3[10] = {0};
    strcpy(s2, s1);                          /* 连 '\0' 一起复制 */
    memcpy(s3, s1, strlen(s1) + 1);          /* ★ +1 才带上 '\0' */
    printf("   s1 = \"%s\"：strlen=%zu，sizeof=%zu\n", s1, strlen(s1), sizeof(s1));
    printf("   strcpy 后 s2 = \"%s\"（长度 %zu）\n", s2, strlen(s2));
    printf("   memcpy 用 strlen+1 后 s3 = \"%s\"（长度 %zu）\n", s3, strlen(s3));
    printf("   若省略 +1，s3 里没有 '\\0'，strlen/printf 会一直往下读，属未定义行为\n");

    /* ---------- 7. 结构体包数组：可以整体赋值 ---------- */
    printf("7. 结构体可以整体赋值，里面的数组会被完整复制\n");
    struct Box p = {{1, 2, 3, 4}, 4};
    struct Box q = p;                        /* ✓ 结构体赋值 = 逐字节拷贝 */
    q.v[0] = 100;
    printf("   p.v = ");
    for (int i = 0; i < p.n; i++) printf("%d ", p.v[i]);
    printf("\n   q.v = ");
    for (int i = 0; i < q.n; i++) printf("%d ", q.v[i]);
    printf("（q 改了，p 不受影响）\n");

    /* ---------- 8. 复制并转换 ---------- */
    printf("8. 循环的优势：边复制边处理\n");
    int dbl[5];
    for (int i = 0; i < 5; i++)
        dbl[i] = a[i] * 2;                   /* 复制的同时做运算 */
    printf("   a 的两倍 dbl = ");
    for (int i = 0; i < 5; i++) printf("%d ", dbl[i]);
    printf("\n   memcpy 只能搬原样，这种需求必须用循环\n");

    /* ---------- 9. 坑：memcpy 长度写成元素个数 ---------- */
    printf("9. 坑：memcpy 第三个参数是字节数，不是元素个数\n");
    int e[5];
    memset(e, 0, sizeof(e));                 /* 先清零，结果才可预期 */
    memcpy(e, a, 4);                         /* ✗ 本想复制 4 个元素，实际只有 4 字节 */
    printf("   memcpy(e, a, 4) 后 e = ");
    for (int i = 0; i < 5; i++) printf("%d ", e[i]);
    printf("\n   只有 4 字节被搬过来，也就是 1 个 int；其余仍是 memset 的 0\n");
    printf("   正确写法：memcpy(e, a, 4 * sizeof(int));\n");

    /* ---------- 10. 二维数组也是连续内存，可以整块搬 ---------- */
    printf("10. 二维数组在内存里同样连续，可以整块 memcpy\n");
    int g[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int h[2][3];
    memcpy(h, g, sizeof(g));                 /* 2×3 = 6 个 int 一次搬完 */
    printf("    g = ");
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++) printf("%d ", g[i][j]);
    printf("\n    h = ");
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++) printf("%d ", h[i][j]);
    printf("（sizeof(g) = %zu 字节，2×3 个 int 连续存放）\n", sizeof(g));

    /* ---------- 11. 小结 ---------- */
    printf("11. 小结：\n");
    printf("    数组不能整体赋值（b = a 编译错误），必须显式搬字节\n");
    printf("    纯搬运用 memcpy(dst, src, 字节数)，长度 = 元素数 × sizeof(元素)\n");
    printf("    同数组内挪动（重叠）用 memmove，memcpy 重叠是未定义行为\n");
    printf("    要转换/过滤就用循环，memcpy 只会原样搬\n");
    printf("    字符串用 strcpy；用 memcpy 必须传 strlen+1\n");
    printf("    复制是值语义：两份独立内存，改一份不影响另一份\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc 02_数组的复制.c -o bin/02_数组的复制 && ./bin/02_数组的复制
 *
 * 输出如下：
 *
 * 1. b = a 不行：数组名是地址常量，不能整体赋值
 *    若写 b = a; 编译器报 assignment to expression with array type
 *    复制必须显式搬字节：循环或 memcpy
 * 2. 循环复制：一个元素一个元素搬
 *    复制后 b = 1 2 3 4 5
 *    把 b[0] 改成 99 后：a[0] = 1，b[0] = 99
 *    两份独立内存，改一个不影响另一个
 * 3. memcpy：一次搬完一整块
 *    memcpy(c, a, sizeof(a)) 后 c = 1 2 3 4 5
 *    sizeof(a) = 20 字节 = 5 个元素 × 4 字节
 *    用 memcmp 验证内容一致：完全相同
 * 4. memmove：同一数组内部挪动（内存重叠）
 *    原始 m = 10 20 30 40 50 60
 *    左移一位后 m = 20 30 40 50 60 60
 *    源 m+1 与目标 m 有重叠区间，所以必须用 memmove 而不是 memcpy
 *    末尾还有一个残留的 60：只搬了 5 个元素，第 6 个位置没被覆盖
 * 5. 部分复制：从 src[2] 起拿 3 个元素
 *    src = 1 2 3 4 5 6
 *    sub = 3 4 5 （取的是 src[2]~src[4]）
 * 6. 字符串：strcpy 会自带 '\0'，memcpy 要自己加 1
 *    s1 = "hello"：strlen=5，sizeof=6
 *    strcpy 后 s2 = "hello"（长度 5）
 *    memcpy 用 strlen+1 后 s3 = "hello"（长度 5）
 *    若省略 +1，s3 里没有 '\0'，strlen/printf 会一直往下读，属未定义行为
 * 7. 结构体可以整体赋值，里面的数组会被完整复制
 *    p.v = 1 2 3 4
 *    q.v = 100 2 3 4 （q 改了，p 不受影响）
 * 8. 循环的优势：边复制边处理
 *    a 的两倍 dbl = 2 4 6 8 10
 *    memcpy 只能搬原样，这种需求必须用循环
 * 9. 坑：memcpy 第三个参数是字节数，不是元素个数
 *    memcpy(e, a, 4) 后 e = 1 0 0 0 0
 *    只有 4 字节被搬过来，也就是 1 个 int；其余仍是 memset 的 0
 *    正确写法：memcpy(e, a, 4 * sizeof(int));
 * 10. 二维数组在内存里同样连续，可以整块 memcpy
 *     g = 1 2 3 4 5 6
 *     h = 1 2 3 4 5 6 （sizeof(g) = 24 字节，2×3 个 int 连续存放）
 * 11. 小结：
 *     数组不能整体赋值（b = a 编译错误），必须显式搬字节
 *     纯搬运用 memcpy(dst, src, 字节数)，长度 = 元素数 × sizeof(元素)
 *     同数组内挪动（重叠）用 memmove，memcpy 重叠是未定义行为
 *     要转换/过滤就用循环，memcpy 只会原样搬
 *     字符串用 strcpy；用 memcpy 必须传 strlen+1
 *     复制是值语义：两份独立内存，改一份不影响另一份
 *
 * 几个容易忽略的对照点：
 *   第 2 条：改 b[0] 之后 a[0] 仍是 1 —— 这就是"值复制"，和指针赋值完全不同。
 *   第 4 条：左移后 60 出现两次，说明"搬 5 个元素"只覆盖前 5 个位置，
 *           第 6 个位置原样保留；要干净收尾得自己把末位清掉。
 *   第 5 条：memcpy 的源可以直接写 src + 2（指针偏移），不需要另建子数组。
 *   第 6 条：strlen 是 5、sizeof 是 6，差的那 1 就是 '\0'；
 *           memcpy 少写 +1 时，"字符串"就变成了一段没有结尾的字节。
 *   第 9 条：刻意先把 e 清零，才能稳定地展示"只搬了 1 个元素，其余是 0"；
 *           如果是未初始化数组，剩下的就是垃圾值，输出每次都不一样。
 *   第 10 条：二维数组也是连续内存，所以 sizeof(g) 一次就能整块搬走 ——
 *            这正是下一节二维数组要展开的内容。
 * ==========================================================================
 */
