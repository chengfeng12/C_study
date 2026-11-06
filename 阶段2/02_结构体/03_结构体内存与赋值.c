/*
 * 03_结构体内存与赋值.c —— 阶段2 / 第 02 课 / 第 03 个文件：结构体占用空间与变量赋值
 *
 * 编译：gcc -std=c99 -Wall -Wextra 03_结构体内存与赋值.c -o bin/03_结构体内存与赋值
 * 运行：./bin/03_结构体内存与赋值
 *
 * 01 篇给了 sizeof 的直觉和 6 种赋值写法，02 篇讲了嵌套是"摊平"的。
 * 这一篇回答两个更硬的问题：
 *
 *      一、一个结构体到底占多少字节？谁决定的？怎么省？
 *      二、`a = b` 到底发生了什么？哪些能赋值、代价多大、什么时候是共享？
 *
 * ====================== 一、占用空间：三条规则就够了 ======================
 * 不算位域时，结构体的大小由三条规则唯一确定：
 *
 *      规则 1  每个成员放在"自己的对齐倍数"的偏移上，不够就插填充
 *      规则 2  结构体的整体对齐 = 成员里**最大的**那个对齐
 *      规则 3  sizeof 向上取整到"整体对齐"的倍数
 *
 * 拿经典例子走一遍：struct A { char c; int i; char d; };
 *
 *      成员   大小   对齐   放的偏移   说明
 *      c       1      1       0        从 0 开始
 *      i       4      4       4        c 之后要跳到 4 的倍数 → 填 3 字节
 *      d       1      1       8        紧接 i 之后
 *      （末尾）                 9       整体对齐 = 4，向上取整到 12 → 填 3 字节
 *
 *      sizeof(struct A) = 12，而成员之和只有 6。
 *
 * 三条规则的两个直接推论：
 *
 *  (1) **成员顺序影响大小**。把大的放前面，填充就少：
 *          struct B { int i; char c; char d; };   // 4 + 1 + 1 + 2 = 8
 *      同样三个成员，A 是 12，B 是 8。这是**零成本的优化** ——
 *      改个顺序就能省 1/3 内存，不需要任何运行时代价。
 *
 *  (2) **嵌套时内层当成一个整体**（02 篇第五节）：
 *          struct Outer { char tag; struct Inner in; char tail; };
 *      内层自己的对齐是 4，所以 in 要放在偏移 4；内层自带的填充也一起带进来。
 *
 * ★ 对齐 = "地址必须是几的倍数"。它由**成员类型**决定，不是结构体决定的：
 *      char 1、short 2、int 4、long/指针 8（64 位）、double 8、long double 16
 *   具体数值看平台 ABI，但"大小通常等于对齐"这条在主流平台基本成立。
 *
 * ★ 可以用 `offsetof(结构体, 成员)` 验证每个成员的真实偏移（需要 <stddef.h>）。
 *   算不清的时候别硬猜，打印出来看。
 *
 * ====================== 二、为什么要对齐？什么时候该关心？ ======================
 * 三个原因：
 *  1. **硬件要求**：某些架构（老 ARM、部分 DSP）访问未对齐地址会直接异常，
 *     或者被拆成两次访问。x86 能容忍但更慢。
 *  2. **缓存行**：对齐良好的数据更少跨越缓存行，访问更快。
 *  3. **ABI 约定**：函数调用、共享库、系统接口都假定按 ABI 对齐，
 *     所以编译器不能随便改布局。
 *
 * 什么时候必须关心：
 *  - **协议/文件格式**：网络包头、磁盘格式要求"字节精确"，
 *    这时要用 `#pragma pack(1)` 或显式序列化，**不能直接 fwrite 结构体**
 *  - **内存敏感场景**：几十万条的结构体数组，每省 4 字节就是几 MB
 *  - **跨 32/64 位**：指针从 4 变 8，布局整体漂移，二进制不兼容
 *  - **性能热点**：把常用字段放前面，能少跨一个缓存行
 *
 * ★ `#pragma pack(1)` / `__attribute__((packed))` 的代价：
 *      它去掉了填充，但让成员可能落在未对齐地址上。
 *      x86 上只是变慢；某些 ARM 上直接总线错误；即使能跑，
 *      取一个未对齐的 int 可能被编译成两次半字读取。
 *      **只在真的需要字节精确时用，并且只用于"读进来马上解析"的结构体**，
 *      别拿它当日常优化手段。
 *
 * ====================== 三、几类特殊成员的空间 ======================
 * (1) **指针 vs 数组**（最容易低估的一项）
 *          struct S1 { char *name; };      // 64 位下 8 字节，只是个地址
 *          struct S2 { char name[16]; };   // 16 字节，名字真的装在里面
 *      S1 省空间但名字在别处（谁分配谁释放）；S2 占空间但自包含。
 *      32 位平台上前者是 4 字节 —— 这就是"结构体布局不能跨架构假设"的根源。
 *
 * (2) **位域（bit-field）**：让多个小字段共用存储单元
 *          struct Flags { unsigned a : 1; unsigned b : 3; unsigned c : 4; };
 *      sizeof 通常是 4（一个 unsigned 存储单元），8 个位只用了 8 个 bit。
 *      限制：**不能取地址**（`&f.a` 非法）、不能 offsetof、布局依实现、
 *      跨平台不可移植。只用来表示"标志位集合"，别用来解析外部格式。
 *
 * (3) **柔性数组成员**（C99）：结构体末尾放一个"长度待定"的数组
 *          struct Packet { int len; char data[]; };
 *          struct Packet *p = malloc(sizeof(struct Packet) + n);
 *      sizeof(struct Packet) 只算 len，不含 data；data 的长度由 malloc 决定。
 *      这是"变长结构体"的标准做法，比 `char *data` 少一次指针跳转和一次分配。
 *      规则：**必须是最后一个成员**，且结构体里至少还有一个其他成员。
 *
 * (4) **空结构体**：C 标准不允许（没有成员的 struct 是非法类型）。
 *      GCC 作为扩展允许，`sizeof` 为 0；C++ 里则是 1。
 *      别依赖它，需要占位就用 `char dummy;`。
 *
 * (5) **只含一个成员的结构体**：不保证和成员一样大。
 *          struct One { char c; };       // sizeof = 1
 *          struct Two { char c; int i; };// 不是 5，是 8
 *
 * ====================== 四、赋值：`a = b` 到底做了什么 ======================
 * 语义：**逐成员拷贝**。标准规定的是"把 b 的每个成员的值赋给 a 的对应成员"，
 * 不是"把 b 的内存字节整体复制给 a"。
 *
 * 这个区别在正常情况下看不出来，但有一个可观察的后果：
 * **填充字节不保证被复制**。gcc 通常按成员逐个写，所以 a 的填充保持原样；
 * 换个编译器可能整块 memcpy。所以：
 *
 *      ✗ 不能用 memcmp 比较两个"成员都相同"的结构体 —— 填充字节可能不同
 *      ✗ 不能把结构体字节直接算哈希、直接 fwrite 后跨平台读
 *      ✓ 要比较就逐成员比，要序列化就逐个字段写
 *
 * 合法条件：**类型必须相同**。
 *      typedef struct Point Point;
 *      Point a; struct Point b;
 *      a = b;                        // ✔ 别名和标签是同一个类型，合法
 *
 *      struct A { int x; };  struct B { int x; };
 *      struct A x;  struct B y;
 *      x = y;                        // ✗ 成员一样也不行，类型不同
 *
 * 几种"看起来像赋值"的写法，语义各不相同：
 *      Point p = q;                  // 初始化：定义时用 q 的值构造 p
 *      p = q;                        // 赋值：p 已存在，逐成员覆盖
 *      p = (Point){ 1, 2 };          // 复合字面量：临时对象再拷贝
 *      memcpy(&p, &q, sizeof p);     // 字节拷贝：连填充一起搬，且不看类型
 *
 * ★ 赋值表达式有值，可以链式（右结合）：
 *      a = b = c;                    // 等价于 a = (b = c)，b、c 都变成 c 的值
 *   返回的是**左值**，所以 `(a = b).x` 也是合法的（但可读性差，别这么写）。
 *
 * ★ 三个"不能整体赋值"的坑：
 *  1. **数组不能赋值**：`int x[3], y[3]; x = y;` 非法。
 *     办法一：memcpy；办法二：把数组包进结构体 —— 结构体可以整体赋值：
 *          struct IntArray3 { int v[3]; };
 *          IntArray3 s1, s2;  s2 = s1;      // ✔ 合法，数组被逐元素拷贝
 *     这是 C 里"给数组赋值"的常用技巧。
 *  2. **含 const 成员的结构体不能被赋值**（p 不是可修改的左值）：
 *          struct WithId { const int id; int v; };
 *          struct WithId a = { 1, 2 }, b = { 3, 4 };
 *          a = b;                            // ✗ 编译错误
 *     只能初始化，不能改。要"可变"就别给成员加 const。
 *  3. **含柔性数组的结构体不要整体赋值**：数组部分不在 sizeof 里，
 *     赋值只搬了固定部分，data 的内容不会跟着走。
 *
 * ★ 复合字面量的生命周期（C99）：
 *      块作用域里的 `(Point){ 1, 2 }` 是**自动存储期**，语句块结束就失效：
 *          Point *bad(void) { return &(Point){ 1, 2 }; }   // ✗ 返回悬空地址
 *      文件作用域的复合字面量是静态存储期，可以长期使用。
 *      所以复合字面量当**实参**或**立即赋值**没问题，别把它的地址带出去。
 *
 * ====================== 五、赋值的代价：拷贝有多大 ======================
 * 结构体赋值 = 拷贝 sizeof 那么多字节。所以：
 *
 *  - **传参**：`void f(struct Big b)` 每次调用都拷一份；
 *    改成 `void f(const struct Big *b)` 只传一个地址。
 *    经验阈值：超过 2~3 个机器字（16~24 字节）就该用指针。
 *  - **返回**：小结构体（≤16 字节）通常通过寄存器返回；大的走"隐藏参数"，
 *    由调用方提供缓冲区，函数往里写。所以"返回大结构体"不一定慢，
 *    但"返回大结构体再整体赋值"仍然要拷一次。
 *  - **数组排序/搬移**：交换两个 256 字节的结构体要拷 512 字节；
 *    如果结构体很大，改成"结构体指针数组"或"索引数组"排序，只交换指针。
 *  - **链表的节点**：节点里放大结构体 → 每个节点都占一大块；
 *    常见做法是节点只存指针，数据放别处。
 *
 * ★ 别为了"省一次拷贝"把代码写复杂。先看 sizeof，超过几十字节再考虑指针。
 *
 * ====================== 六、浅拷贝的三个具体后果 ======================
 * 赋值只拷贝"结构体自己的字节"。成员是指针时，拷走的只是地址：
 *
 * (1) **双重释放**：两个结构体的指针成员指向同一块 malloc 内存，
 *     各自析构/释放一次 → double free。谁负责释放必须在设计时定死。
 *
 * (2) **改一个动两个**：`p2 = p1; p2.name[0] = 'X';` 后 p1.name 也变了。
 *     字符串用 `char *` 存时尤其常见。
 *
 * (3) **悬空指针**：指向栈上局部变量，函数返回后指针还在，对象已经没了。
 *     （指针的完整用法：const 组合、指针数组、动态分配，见 05 篇。）
 *
 * 解决办法只有两种：
 *  - **明确"借用"语义**：结构体只持有引用，不负责释放，命名上体现（`Ref`、`View`）
 *  - **写深拷贝函数**：`xxx_clone()` 里对每个指针成员重新分配 + 复制
 *
 * ★ memcpy 也一样是浅拷贝，而且更糟：它连"类型"都不看，结构体加了成员也不会提醒你。
 *
 * ====================== 七、常见坑 ======================
 *  1. 按成员之和估算 sizeof        漏了对齐填充，往往少算 30%
 *  2. 成员顺序随便写                大的放前面能白省内存
 *  3. 用 memcmp 比较结构体          填充字节不同 → 误判不等
 *  4. 直接 fwrite 结构体            填充 + 字节序 + 指针，跨平台必炸
 *  5. 用 #pragma pack(1) 图省事      未对齐访问，ARM 上可能直接崩
 *  6. 赋值两个"成员相同但类型不同"的结构体   编译错误
 *  7. 对数组用 =                     非法；用 memcpy 或包进结构体
 *  8. 给含 const 成员的结构体赋值    不是可修改左值，编译错误
 *  9. 返回块作用域复合字面量的地址   自动存储期，出块即失效
 * 10. 大结构体值传递/值返回         每次调用拷一整块，改用 const 指针
 * 11. 赋值后以为指针成员也复制了    浅拷贝，两个结构体共享同一对象
 *
 * ====================== 八、实践建议 ======================
 *  - 布局优化：成员按"对齐从大到小"排，通常最省（可读性优先时另说）
 *  - 用 `offsetof` + `sizeof` 写个断言或打印，验证你的假设
 *  - 需要字节精确的格式：显式序列化（逐字段写），或 pack + 明确字节序
 *  - 变长数据用柔性数组成员，一次 malloc 搞定，少一次指针跳转
 *  - 结构体超过 16~24 字节，传参就用 `const T *`
 *  - 成员里有指针时，在类型名和注释里写清所有权（Owner / Ref / View）
 *  - 需要独立副本就写 clone，别指望赋值帮你深拷贝
 *  - 比较用逐成员函数，序列化用逐字段写，别走 memcmp / fwrite 的捷径
 */

#include <stdio.h>
#include <stddef.h>      /* offsetof */
#include <string.h>      /* memcmp / memset */
#include <stdlib.h>      /* malloc / free */

/* ====================== 布局对比：同样三个成员，两种顺序 ====================== */

typedef struct A {          /* 差顺序：char, int, char */
    char c;
    int  i;
    char d;
} A;

typedef struct B {          /* 好顺序：int, char, char */
    int  i;
    char c;
    char d;
} B;

/* 各种对齐组合 */
typedef struct C { char c; short s; int i; } C;      /* 1 + 填充 + 2 + 4 */
typedef struct D { char c; double d; } D;            /* 1 + 7 填充 + 8 */
typedef struct E { char a, b, c; } E;                /* 无填充 */

/* ====================== 特殊成员 ====================== */

/* 位域：多个小字段共用一个存储单元 */
typedef struct Flags {
    unsigned a : 1;         /* 1 位 */
    unsigned b : 3;         /* 3 位 */
    unsigned c : 4;         /* 4 位，合计 8 位，但占满一个 unsigned 单元 */
} Flags;

/* 柔性数组成员：必须是最后一个成员，sizeof 不含它 */
typedef struct Packet {
    int  len;
    char data[];            /* C99 柔性数组 */
} Packet;

/* 指针成员 vs 数组成员 */
typedef struct WithPtr  { const char *name; } WithPtr;
typedef struct WithArr  { char name[16];     } WithArr;

/* ====================== 赋值相关 ====================== */

/* 把数组包进结构体：这样数组也能整体赋值 */
typedef struct IntArray3 {
    int v[3];
} IntArray3;

/* 带填充的结构体：用来演示"成员相同但字节不同" */
typedef struct Padded {
    char c;
    int  i;
} Padded;

/* 大结构体：演示拷贝成本 */
typedef struct Big {
    int data[64];           /* 256 字节 */
} Big;

/* 指针成员：演示浅拷贝 */
typedef struct Person {
    char *name;             /* 指向别处的字符串 */
    int   age;
} Person;

int main(void)
{
    /* ---------- 1. 三条规则：算一遍 sizeof ---------- */
    printf("1. 占用空间：三条规则\n");
    printf("   规则 1：成员放在自己对齐的倍数偏移上，不够就填\n");
    printf("   规则 2：整体对齐 = 最大成员对齐\n");
    printf("   规则 3：总大小向上取整到整体对齐的倍数\n");

    printf("   struct A { char c; int i; char d; }\n");
    printf("      sizeof(A) = %zu，成员之和只有 %zu\n",
           sizeof(A), sizeof(char) + sizeof(int) + sizeof(char));
    printf("      offsetof(A, c) = %zu, offsetof(A, i) = %zu, offsetof(A, d) = %zu\n",
           offsetof(A, c), offsetof(A, i), offsetof(A, d));
    printf("      c 在 0；i 要跳到 4 → 中间填 3 字节；d 在 8；末尾再填到 12\n");

    printf("   struct C { char c; short s; int i; }\n");
    printf("      sizeof(C) = %zu，offsetof: c=%zu s=%zu i=%zu\n",
           sizeof(C), offsetof(C, c), offsetof(C, s), offsetof(C, i));

    printf("   struct D { char c; double d; }\n");
    printf("      sizeof(D) = %zu（1 + 7 填充 + 8）\n", sizeof(D));
    printf("   struct E { char a, b, c; }\n");
    printf("      sizeof(E) = %zu（没有填充）\n", sizeof(E));
    printf("   具体数值依平台，但推导方式一样，用 offsetof 一验就知道\n");

    /* ---------- 2. 成员重排：零成本省内存 ---------- */
    printf("2. 成员重排：同样三个成员，12 → 8\n");
    printf("   struct A { char c; int i; char d; } → sizeof = %zu\n", sizeof(A));
    printf("   struct B { int i; char c; char d; } → sizeof = %zu\n", sizeof(B));
    printf("   省下 %zu 字节，运行时零代价：只是换个书写顺序\n",
           sizeof(A) - sizeof(B));
    printf("   规律：对齐大的成员放前面，填充自然变少\n");

    /* ---------- 3. 特殊成员的空间 ---------- */
    printf("3. 特殊成员\n");
    printf("   指针成员：sizeof(WithPtr) = %zu（只是一个地址）\n", sizeof(WithPtr));
    printf("   数组成员：sizeof(WithArr) = %zu（16 字节真的装在里面）\n", sizeof(WithArr));
    printf("      → 指针省空间但数据在别处，数组占空间但自包含\n");

    printf("   位域：sizeof(Flags) = %zu，a/b/c 只用 8 个 bit\n", sizeof(Flags));
    printf("      位域不能取地址，也不能 offsetof，布局依实现\n");

    printf("   柔性数组：sizeof(Packet) = %zu（只有 len，data 不算）\n", sizeof(Packet));
    printf("      实际分配时 malloc(sizeof(Packet) + n)，一次搞定变长数据\n");

    printf("   sizeof(Big) = %zu 字节 —— 值传递一次就拷这么多\n", sizeof(Big));

    /* ---------- 4. 赋值：逐成员拷贝，不是字节拷贝 ---------- */
    printf("4. 赋值语义：逐成员拷贝，填充字节不保证被复制\n");

    Padded p1, p2;
    memset(&p1, 0xAA, sizeof p1);        /* 先把填充字节涂成 0xAA */
    memset(&p2, 0x00, sizeof p2);
    p1.c = 'x';  p1.i = 7;               /* 只写真实成员 */
    p2.c = 'x';  p2.i = 7;

    printf("   p1、p2 成员完全相同（c='x', i=7）\n");
    printf("   但 memcmp(&p1, &p2, sizeof) == 0 吗？%s\n",
           memcmp(&p1, &p2, sizeof p1) == 0 ? "是" : "否");
    printf("   因为 p1 的填充是 0xAA、p2 是 0x00 —— 填充字节内容不确定\n");
    printf("   所以：比较要逐成员比，序列化要逐字段写\n");

    /* ---------- 5. 赋值 vs 初始化 vs memcpy ---------- */
    printf("5. 初始化 / 赋值 / 复合字面量 / memcpy\n");
    A a1 = { 'a', 1, 'b' };              /* 初始化 */
    A a2;
    a2 = a1;                             /* 赋值：逐成员拷贝 */
    a2.i = 100;
    printf("   a2 = a1 后改 a2.i：a1.i = %d, a2.i = %d（独立的两块）\n",
           a1.i, a2.i);

    a2 = (A){ 'z', 9, 'y' };             /* 复合字面量赋值 */
    printf("   a2 = (A){ 'z', 9, 'y' } → c=%c, i=%d, d=%c\n",
           a2.c, a2.i, a2.d);

    A a3;
    memcpy(&a3, &a1, sizeof a3);         /* 字节拷贝：连填充一起搬 */
    printf("   memcpy(&a3, &a1, sizeof a3) → c=%c, i=%d, d=%c\n",
           a3.c, a3.i, a3.d);

    A a4, a5;
    a4 = a5 = a1;                        /* 链式赋值，右结合 */
    printf("   链式 a4 = a5 = a1 → a4.i = %d, a5.i = %d\n", a4.i, a5.i);

    /* ---------- 6. 数组不能赋值，但包进结构体可以 ---------- */
    printf("6. 数组不能整体赋值，包进结构体就行\n");
    IntArray3 s1 = { { 1, 2, 3 } };
    IntArray3 s2;
    s2 = s1;                             /* ✔ 结构体整体赋值，数组被逐元素拷贝 */
    s2.v[0] = 99;
    printf("   s2 = s1 后改 s2.v[0]：s1.v[0] = %d, s2.v[0] = %d\n",
           s1.v[0], s2.v[0]);
    printf("   直接写 int x[3], y[3]; x = y; 是非法的，只能 memcpy\n");

    /* ---------- 7. 赋值成本 ---------- */
    printf("7. 赋值成本 = sizeof 字节\n");
    printf("   Big 有 64 个 int：sizeof = %zu\n", sizeof(Big));
    Big big1 = { { 0 } }, big2;
    big2 = big1;                         /* 拷 256 字节 */
    printf("   big2 = big1 一次拷贝 %zu 字节（big2.data[0] = %d）\n",
           sizeof(Big), big2.data[0]);
    printf("   传参用 Big 就要拷 256 字节，用 const Big * 只传 %zu 字节地址\n",
           sizeof(Big *));
    printf("   经验阈值：超过 16~24 字节，传 const 指针\n");

    /* ---------- 8. 浅拷贝的三个后果 ---------- */
    printf("8. 指针成员：赋值只拷地址，三个实际后果\n");

    char name1[] = "alice";
    Person u1 = { name1, 30 };
    Person u2 = u1;                      /* 只拷了 name 指针 */
    printf("   u2 = u1 后：u2.name 和 u1.name 是同一块内存吗？%s\n",
           u1.name == u2.name ? "是" : "否");
    u2.name[0] = 'A';
    printf("   改 u2.name[0] 后 u1.name = %s（一个改，两个都变）\n", u1.name);
    printf("   如果这块内存是 malloc 的，两个 Person 都会去 free → double free\n");
    printf("   解决：明确所有权（Ref/View），或写 person_clone 做深拷贝\n");

    /* ---------- 9. 小结 ---------- */
    printf("9. 小结：\n");
    printf("    三条规则：成员按自身对齐放、整体取最大对齐、总大小向上取整\n");
    printf("    成员顺序影响大小，对齐大的放前面，零成本省内存\n");
    printf("    sizeof ≥ 成员之和，差值是填充；用 offsetof 验证假设\n");
    printf("    指针成员只是地址，数组成员才把数据装在自己身上\n");
    printf("    位域省空间但不能取地址；柔性数组让结构体变长\n");
    printf("    赋值是逐成员拷贝，填充字节不保证复制 → 别 memcmp、别 fwrite\n");
    printf("    数组不能赋值，包进结构体就行；含 const 成员的结构体不能赋值\n");
    printf("    赋值成本 = sizeof，大结构体传 const 指针\n");
    printf("    指针成员赋值是浅拷贝，想独立就写 clone\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 03_结构体内存与赋值.c -o bin/03_结构体内存与赋值
 *     ./bin/03_结构体内存与赋值
 *
 * 输出如下：
 *
 * 1. 占用空间：三条规则
 *    规则 1：成员放在自己对齐的倍数偏移上，不够就填
 *    规则 2：整体对齐 = 最大成员对齐
 *    规则 3：总大小向上取整到整体对齐的倍数
 *    struct A { char c; int i; char d; }
 *       sizeof(A) = 12，成员之和只有 6
 *       offsetof(A, c) = 0, offsetof(A, i) = 4, offsetof(A, d) = 8
 *       c 在 0；i 要跳到 4 → 中间填 3 字节；d 在 8；末尾再填到 12
 *    struct C { char c; short s; int i; }
 *       sizeof(C) = 8，offsetof: c=0 s=2 i=4
 *    struct D { char c; double d; }
 *       sizeof(D) = 16（1 + 7 填充 + 8）
 *    struct E { char a, b, c; }
 *       sizeof(E) = 3（没有填充）
 *    具体数值依平台，但推导方式一样，用 offsetof 一验就知道
 * 2. 成员重排：同样三个成员，12 → 8
 *    struct A { char c; int i; char d; } → sizeof = 12
 *    struct B { int i; char c; char d; } → sizeof = 8
 *    省下 4 字节，运行时零代价：只是换个书写顺序
 *    规律：对齐大的成员放前面，填充自然变少
 * 3. 特殊成员
 *    指针成员：sizeof(WithPtr) = 8（只是一个地址）
 *    数组成员：sizeof(WithArr) = 16（16 字节真的装在里面）
 *       → 指针省空间但数据在别处，数组占空间但自包含
 *    位域：sizeof(Flags) = 4，a/b/c 只用 8 个 bit
 *       位域不能取地址，也不能 offsetof，布局依实现
 *    柔性数组：sizeof(Packet) = 4（只有 len，data 不算）
 *       实际分配时 malloc(sizeof(Packet) + n)，一次搞定变长数据
 *    sizeof(Big) = 256 字节 —— 值传递一次就拷这么多
 * 4. 赋值语义：逐成员拷贝，填充字节不保证被复制
 *    p1、p2 成员完全相同（c='x', i=7）
 *    但 memcmp(&p1, &p2, sizeof) == 0 吗？否
 *    因为 p1 的填充是 0xAA、p2 是 0x00 —— 填充字节内容不确定
 *    所以：比较要逐成员比，序列化要逐字段写
 * 5. 初始化 / 赋值 / 复合字面量 / memcpy
 *    a2 = a1 后改 a2.i：a1.i = 1, a2.i = 100（独立的两块）
 *    a2 = (A){ 'z', 9, 'y' } → c=z, i=9, d=y
 *    memcpy(&a3, &a1, sizeof a3) → c=a, i=1, d=b
 *    链式 a4 = a5 = a1 → a4.i = 1, a5.i = 1
 * 6. 数组不能整体赋值，包进结构体就行
 *    s2 = s1 后改 s2.v[0]：s1.v[0] = 1, s2.v[0] = 99
 *    直接写 int x[3], y[3]; x = y; 是非法的，只能 memcpy
 * 7. 赋值成本 = sizeof 字节
 *    Big 有 64 个 int：sizeof = 256
 *    big2 = big1 一次拷贝 256 字节（big2.data[0] = 0）
 *    传参用 Big 就要拷 256 字节，用 const Big * 只传 8 字节地址
 *    经验阈值：超过 16~24 字节，传 const 指针
 * 8. 指针成员：赋值只拷地址，三个实际后果
 *    u2 = u1 后：u2.name 和 u1.name 是同一块内存吗？是
 *    改 u2.name[0] 后 u1.name = Alice（一个改，两个都变）
 *    如果这块内存是 malloc 的，两个 Person 都会去 free → double free
 *    解决：明确所有权（Ref/View），或写 person_clone 做深拷贝
 * 9. 小结：
 *     三条规则：成员按自身对齐放、整体取最大对齐、总大小向上取整
 *     成员顺序影响大小，对齐大的放前面，零成本省内存
 *     sizeof ≥ 成员之和，差值是填充；用 offsetof 验证假设
 *     指针成员只是地址，数组成员才把数据装在自己身上
 *     位域省空间但不能取地址；柔性数组让结构体变长
 *     赋值是逐成员拷贝，填充字节不保证复制 → 别 memcmp、别 fwrite
 *     数组不能赋值，包进结构体就行；含 const 成员的结构体不能赋值
 *     赋值成本 = sizeof，大结构体传 const 指针
 *     指针成员赋值是浅拷贝，想独立就写 clone
 * ==========================================================================
 */
