/*
 * 08_void指针与类型转换.c —— 阶段1 / 第 06 课 / 第 08 个文件：void * 与指针类型转换
 *
 * 编译：gcc -std=c99 -Wall -Wextra 08_void指针与类型转换.c -o bin/08_void指针与类型转换
 * 运行：./bin/08_void指针与类型转换
 *
 * 前 7 篇讲的都是「有类型的指针」。这一篇补上最后一个缺口：
 * **没有类型的指针 void *，以及指针之间为什么能互相强转、什么时候不能。**
 *
 * 本文件只用 malloc 一小块内存演示（完整的内存管理在后面章节），
 * 所有会真正触发未定义行为的地方（类型双关的强转写法）都只写在注释里。
 *
 * ====================== 一、void * 是什么 ======================
 *      void *p;        // 一个地址，但**没有说它指向什么**
 *
 * 可以读成 "pointer to void"，但它不是"指向空"的指针，而是
 * **"指向未知类型"的指针**。编译器只知道它是一个地址（64 位下 8 字节），
 * 不知道解引用该读几个字节、按什么方式解释。
 *
 * 三条规则：
 *
 *  (1) 任何**对象指针**都能隐式转成 void *，反过来要显式转回：
 *          int a = 1;
 *          void *v = &a;          // ✓ 隐式，不用强转
 *          int *p = v;            // ✓ C 里 void * 也能隐式转回对象指针
 *          int x = *(int *)v;     // ✓ 解引用前必须转回类型
 *
 *  (2) **不能解引用** void *：*v ✗ 编译错误（不知道读几个字节）。
 *      必须先转成具体类型。
 *
 *  (3) **不能做算术**：v + 1 ✗（标准 C）。因为"一个元素多大"未知。
 *      GCC 把它当 char * 处理（走一个字节），但那是扩展，别依赖。
 *
 *      sizeof(void *) = 8      和其他指针完全一样
 *      而 sizeof(*v)  ✗ 无意义：void 的大小在标准里是不完整的
 *
 * ====================== 二、为什么标准库到处是 void * ======================
 * C 没有模板、没有重载，想让**一个函数服务所有类型**，只有一个办法：
 * 把类型信息擦掉，只传地址和大小。
 *
 *      void *malloc(size_t size);                   // 申请一块内存，谁用谁转
 *      void  free(void *ptr);                       // 什么类型都能还
 *
 *      void *memcpy(void *dst, const void *src, size_t n);   // 按字节复制
 *      void *memset(void *s, int c, size_t n);               // 按字节填充
 *
 *      void qsort(void *base, size_t n, size_t size,
 *                 int (*cmp)(const void *, const void *));    // 任意类型排序
 *
 * 注意它们的共同点：**都额外要一个"大小"参数**。
 * 因为 void * 把"元素多大"这个信息丢了，只能由调用方补上 —— 这也是
 * memcpy 最容易写错的地方（传了元素个数而不是字节数）。
 *
 * ====================== 三、转回原类型：类型必须对得上 ======================
 *      int a = 42;
 *      void *v = &a;
 *
 *      *(int *)v       ✓ 转回原本的类型，读到 42
 *      *(char *)v      ✗ 能编译、能运行，但只读到 4 个字节里的 1 个
 *      *(double *)v    ✗ 读 8 字节，越界 + 按浮点解释 → 垃圾值
 *
 * 编译器不会拦你，因为强转的语义就是"我知道我在干什么"。
 * 所以 void * 的代价是：**类型安全全靠你自己记着**。
 *
 * ★ malloc 的返回值在 C 里**不需要强转**：
 *      int *p = malloc(sizeof *p);          // ✓ 推荐
 *      int *p = (int *)malloc(sizeof *p);   // ✓ 能编译，但不推荐
 *
 *   不推荐的原因：万一忘了 #include <stdlib.h>，编译器会按旧规则
 *   假设 malloc 返回 int，指针就会被截断成 32 位 —— 加了强转反而
 *   把这条错误掩盖掉；不加则会出现 warning。
 *
 * ====================== 四、强转的两种动机 ======================
 *  (1) **类型擦除后再用回来**
 *          void *v = &a;  int *p = (int *)v;
 *      同一种类型，只是中间路过 void *。这是安全的，也是标准库的工作方式。
 *
 *  (2) **把同一块内存按另一种类型解释**（type punning，类型双关）
 *          float f = 1.0f;
 *          unsigned int bits = *(unsigned int *)&f;      // ✗ 未定义行为
 *      目的通常是"看位模式"，但这个写法违反**严格别名规则**（第六节），
 *      在 -O2 下可能得到错误结果。安全写法是 memcpy。
 *
 * 分清这两种动机很重要：第一种是必要的技术，第二种多半是想偷懒。
 *
 * ====================== 五、唯一能"随便转"的例外：char * ======================
 * 标准明确允许：**任何对象都可以按 char / unsigned char 逐字节访问**。
 *
 *      unsigned int u = 0x01020304u;
 *      unsigned char *b = (unsigned char *)&u;      // ✓ 完全合法
 *      b[0], b[1], b[2], b[3]                       // 四个字节，按内存顺序
 *
 * 这是唯一的"通用视角"，也是下面这些功能的基础：
 *  - memcpy / memset 的字节级实现
 *  - 序列化（把结构体写成字节流）
 *  - 哈希（FNV、CRC 都对字节做运算）
 *  - 判断大小端：看低位字节排在最前还是最后
 *
 * 注意用 **unsigned char** 而不是 char：char 的符号性由实现决定，
 * 做数值运算时容易出意外。
 *
 * ====================== 六、严格别名规则（strict aliasing） ======================
 * 编译器优化时假设：**不同类型的指针不会指向同一块内存**。
 * 所以
 *
 *      float f = 1.0f;
 *      unsigned int bits = *(unsigned int *)&f;     // ✗ 违反这个假设
 *
 * 属于未定义行为。在 -O0 下通常"看起来正常"，在 -O2 下可能被优化成
 * 你完全看不懂的结果 —— 这是最典型的"换台机器/换个优化等级就崩"。
 *
 * 三种正确写法：
 *
 *      memcpy(&bits, &f, sizeof bits);       // ✓ 首选，编译器会优化成一条 mov
 *      union { float f; unsigned int u; } x; // ✓ C 允许（C++ 不允许，跨语言代码别用）
 *      char 视角逐字节拼                     // ✓ 合法但啰嗦
 *
 * 编译选项 -fno-strict-aliasing 能让老代码"跑对"，但它是在关掉优化器的
 * 推理能力，属于掩盖问题，不是修复问题。
 *
 * ====================== 七、函数指针 ≠ 对象指针 ======================
 *      void *v = (void *)add;        // ✗ 标准不保证：函数指针和对象指针是两类
 *      int (*fp)(int,int) = ...;
 *      void *v = (void *)fp;         // 同上，属于实现定义行为
 *
 * 只有 POSIX 的 dlsym 等接口才强制这么用（平台扩展），普通代码里不要混。
 * 但**函数指针之间**可以互转再转回，只要是同一个原型。
 *
 * ====================== 八、restrict（C99） ======================
 *      void copy(int *restrict dst, const int *restrict src, size_t n);
 *
 * restrict 只用于指针，是一个**承诺**：
 *
 *      通过 dst 访问的那些对象，不会被 src（以及其他任何指针）访问。
 *
 * 有了这个承诺，编译器就能放心地消除多余的加载、做向量化。
 * 典型的反差是标准库的两个函数：
 *
 *      memcpy(dst, src, n);    // 要求两块内存不重叠（内部用 restrict）
 *      memmove(dst, src, n);   // 允许重叠，所以要处理方向，慢一点
 *
 * ★ restrict 是"承诺"，违反了就是未定义行为，不是"可能慢一点"。
 *   不确定就别写；写了就要保证真的不重叠。
 *
 * ====================== 九、常见坑 ======================
 *  1. 解引用 void *                    *v ✗ 编译错误，必须先转类型
 *  2. 对 void * 做算术                 v + 1 ✗ 标准不允许
 *  3. 转错类型再解引用                 能编过，读到的是垃圾（甚至越界）
 *  4. memcpy 传元素个数而不是字节数     memcpy(d, s, 3) 只复制 3 个字节
 *  5. memset 填非 0 值当整数用          memset(a, 1, sizeof a) 得到 0x01010101
 *  6. 用 *(int *)&f 取位模式           违反严格别名，-O2 下可能出错
 *  7. 忘记判空 malloc 的返回值          NULL 上解引用必崩
 *  8. 强转掩盖 #include 缺失            (int *)malloc 让"隐式声明"错误消失
 *  9. 函数指针强转 void *               实现定义行为，跨平台会翻车
 * 10. 给 restrict 指针传重叠的缓冲区    承诺被打破，未定义行为
 *
 * ====================== 十、实践建议 ======================
 *  - 能不用 void * 就不用；它等于主动放弃类型检查
 *  - 标准库接口必须用 void * 时，把"大小"参数一起传对
 *  - malloc 写 `p = malloc(n * sizeof *p)`，不写强制转换
 *  - 需要"看字节"就用 (unsigned char *)，那是标准唯一允许的通用视角
 *  - 需要"重新解释位模式"就用 memcpy，别用指针强转
 *  - 类型转换后的指针，注释写清"这里转回什么、为什么安全"
 *  - restrict 只在真不重叠时写，拿不准就省掉
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* restrict：向编译器承诺 dst 和 src 不重叠，于是可以放心向量化 */
static void copy_ints(int *restrict dst, const int *restrict src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        dst[i] = src[i];
}

/* 泛型回调：参数是 const void *，所以任何类型的数组都能复用（qsort 的做法） */
static void print_via_void(const void *p)
{
    printf("   通过 void * 拿到 int：%d\n", *(const int *)p);
}

int main(void)
{
    /* ---------- 1. void * 接收各种指针 ---------- */
    printf("1. void * 是「没有类型信息的地址」\n");
    int i = 42;
    double d = 3.14;
    char c = 'A';
    void *v1 = &i;                       /* 任何对象指针都能隐式转成 void * */
    void *v2 = &d;
    void *v3 = &c;
    printf("   sizeof(void *) = %zu，和其他指针完全一样\n", sizeof(void *));
    printf("   v1 = %p   v2 = %p   v3 = %p\n", v1, v2, v3);
    printf("   三个地址长得一样「格式」，但编译器已经不知道它们指向什么了\n");

    /* ---------- 2. 用之前必须转回原类型 ---------- */
    printf("2. 解引用前必须转回原本的类型\n");
    printf("   *(int *)v1    = %d\n", *(int *)v1);
    printf("   *(double *)v2 = %.2f\n", *(double *)v2);
    printf("   *(char *)v3   = '%c'\n", *(char *)v3);
    /* printf("%d", *(double *)v1);    ✗ 能编过，但把 int 的位模式按 double 解释 → 垃圾 */
    printf("   转错类型不会报错：编译器只认你写的那个类型\n");

    /* ---------- 3. malloc 与 sizeof *p 惯用法 ---------- */
    printf("3. malloc 返回 void *，靠 sizeof *p 算大小\n");
    int *arr = malloc(4 * sizeof *arr);  /* ★ 惯用法：sizeof *arr 不必重复写 int */
    if (arr == NULL) {                   /* ★ 必须判空 */
        printf("   malloc 返回 NULL，结束\n");
        return 1;
    }
    for (int k = 0; k < 4; k++)
        arr[k] = k * k;
    printf("   arr = %d %d %d %d（sizeof *arr = %zu 字节）\n",
           arr[0], arr[1], arr[2], arr[3], sizeof *arr);
    free(arr);
    arr = NULL;
    printf("   C 里不需要 (int *)malloc(...) 强转：加了反而会掩盖缺少 #include <stdlib.h>\n");

    /* ---------- 4. memcpy / memset 的 void * 参数 ---------- */
    printf("4. memcpy / memset 收的是 void *，第三个参数是「字节数」\n");
    int src3[3] = {7, 8, 9};
    int dst3[3] = {0, 0, 0};
    memcpy(dst3, src3, sizeof src3);
    printf("   memcpy 之后 dst3 = %d %d %d\n", dst3[0], dst3[1], dst3[2]);
    memset(dst3, 0, sizeof dst3);
    printf("   memset 清零之后 dst3 = %d %d %d\n", dst3[0], dst3[1], dst3[2]);

    int ones[3];
    memset(ones, 1, sizeof ones);        /* 每个字节都填 1，不是"整数 1" */
    printf("   memset(ones, 1, sizeof ones) → ones[0] = %d（0x01010101，不是 1）\n",
           ones[0]);
    printf("   所以 memset 只适合清零；要填别的值就用循环\n");

    /* ---------- 5. (unsigned char *) 逐字节看内存 ---------- */
    printf("5. 唯一能「随便转」的例外：(unsigned char *) 看字节表示\n");
    unsigned int u = 0x01020304u;
    unsigned char *bytes = (unsigned char *)&u;
    printf("   0x%08x 的 %zu 个字节：", u, sizeof u);
    for (size_t k = 0; k < sizeof u; k++)
        printf("%02x ", bytes[k]);
    printf("\n");
    printf("   这台机器是%s（低位字节排在前面就是小端）\n",
           (bytes[0] == 0x04) ? "小端 little-endian" : "大端 big-endian");
    printf("   标准保证：任何对象都能按 char / unsigned char 逐字节读取，这是合法的\n");

    /* ---------- 6. 类型双关：memcpy 安全，强转不安全 ---------- */
    printf("6. 类型双关：看 float 的位模式，要用 memcpy\n");
    float f = 1.0f;
    unsigned int bits = 0;
    memcpy(&bits, &f, sizeof bits);      /* ✓ 标准允许，编译器通常优化成一条 mov */
    printf("   1.0f 的 IEEE 754 位模式 = 0x%08x\n", bits);
    /* unsigned int bad = *(unsigned int *)&f;   ✗ 违反严格别名规则，是未定义行为 */
    printf("   写成 *(unsigned int *)&f 违反严格别名（strict aliasing）：\n");
    printf("   编译器有权假设不同类型的指针不指向同一块内存，-O2 下可能算错\n");
    printf("   安全替代：memcpy（首选）或 union（C 可以，C++ 不行）\n");

    /* ---------- 7. restrict ---------- */
    printf("7. restrict：承诺两块内存不重叠\n");
    int a5[5] = {1, 2, 3, 4, 5};
    int b5[5] = {0, 0, 0, 0, 0};
    copy_ints(b5, a5, 5);
    printf("   copy_ints(b5, a5, 5) → b5 = %d %d %d %d %d\n",
           b5[0], b5[1], b5[2], b5[3], b5[4]);
    printf("   声明里 dst / src 都带 restrict，编译器就能放心向量化\n");
    printf("   对比库函数：memcpy 要求不重叠，memmove 允许重叠（所以慢一点）\n");
    printf("   restrict 违反 = 未定义行为，不是「可能慢一点」\n");

    /* ---------- 8. 泛型回调 ---------- */
    printf("8. void * 是「泛型回调」的载体\n");
    print_via_void(&i);
    printf("   回调签名写成 const void *，同一个函数就能被任何类型的数组复用\n");

    /* ---------- 9. 小结 ---------- */
    printf("9. 小结：\n");
    printf("    void * 是「地址 + 无类型」，不能解引用、不能算术\n");
    printf("    任何对象指针都能与 void * 互转，转回时必须用原类型\n");
    printf("    标准库的泛型接口靠 void * + 一个「大小」参数实现\n");
    printf("    malloc 不需要强转，写 sizeof *p 最省心\n");
    printf("    (unsigned char *) 是唯一被标准允许的「通用视角」\n");
    printf("    看位模式用 memcpy，别用 *(T *)&x（严格别名 UB）\n");
    printf("    restrict 是承诺不是优化提示，违反就是未定义行为\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 08_void指针与类型转换.c -o bin/08_void指针与类型转换
 *     ./bin/08_void指针与类型转换
 *
 * 输出如下（0x 开头的地址每次运行都不同；第 5 条的字节顺序与机器端序有关）：
 *
 * 1. void * 是「没有类型信息的地址」
 *    sizeof(void *) = 8，和其他指针完全一样
 *    v1 = 0x...70   v2 = 0x...68   v3 = 0x...67
 *    三个地址长得一样「格式」，但编译器已经不知道它们指向什么了
 * 2. 解引用前必须转回原本的类型
 *    *(int *)v1    = 42
 *    *(double *)v2 = 3.14
 *    *(char *)v3   = 'A'
 *    转错类型不会报错：编译器只认你写的那个类型
 * 3. malloc 返回 void *，靠 sizeof *p 算大小
 *    arr = 0 1 4 9（sizeof *arr = 4 字节）
 *    C 里不需要 (int *)malloc(...) 强转：加了反而会掩盖缺少 #include <stdlib.h>
 * 4. memcpy / memset 收的是 void *，第三个参数是「字节数」
 *    memcpy 之后 dst3 = 7 8 9
 *    memset 清零之后 dst3 = 0 0 0
 *    memset(ones, 1, sizeof ones) → ones[0] = 16843009（0x01010101，不是 1）
 *    所以 memset 只适合清零；要填别的值就用循环
 * 5. 唯一能「随便转」的例外：(unsigned char *) 看字节表示
 *    0x01020304 的 4 个字节：04 03 02 01
 *    这台机器是小端 little-endian（低位字节排在前面就是小端）
 *    标准保证：任何对象都能按 char / unsigned char 逐字节读取，这是合法的
 * 6. 类型双关：看 float 的位模式，要用 memcpy
 *    1.0f 的 IEEE 754 位模式 = 0x3f800000
 *    写成 *(unsigned int *)&f 违反严格别名（strict aliasing）：
 *    编译器有权假设不同类型的指针不指向同一块内存，-O2 下可能算错
 *    安全替代：memcpy（首选）或 union（C 可以，C++ 不行）
 * 7. restrict：承诺两块内存不重叠
 *    copy_ints(b5, a5, 5) → b5 = 1 2 3 4 5
 *    声明里 dst / src 都带 restrict，编译器就能放心向量化
 *    对比库函数：memcpy 要求不重叠，memmove 允许重叠（所以慢一点）
 *    restrict 违反 = 未定义行为，不是「可能慢一点」
 * 8. void * 是「泛型回调」的载体
 *    通过 void * 拿到 int：42
 *    回调签名写成 const void *，同一个函数就能被任何类型的数组复用
 * 9. 小结：
 *     void * 是「地址 + 无类型」，不能解引用、不能算术
 *     任何对象指针都能与 void * 互转，转回时必须用原类型
 *     标准库的泛型接口靠 void * + 一个「大小」参数实现
 *     malloc 不需要强转，写 sizeof *p 最省心
 *     (unsigned char *) 是唯一被标准允许的「通用视角」
 *     看位模式用 memcpy，别用 *(T *)&x（严格别名 UB）
 *     restrict 是承诺不是优化提示，违反就是未定义行为
 *
 * 几个容易忽略的对照点：
 *   第 1 条：三个地址相差 8 / 1 字节，说明 void * 只是"把地址借走"，
 *           完全不改变对象本身；sizeof(void *) 依然是 8。
 *   第 3 条：`malloc(4 * sizeof *arr)` 里 **arr 还没赋值**，但 sizeof 是编译期
 *           求值的，不会真的解引用 —— 这正是这个惯用法成立的原因。
 *   第 4 条：memcpy 的第三个参数是字节数，所以写 `sizeof src3` 而不是 3；
 *           memset 按字节填，把 int 填成 1 得到 0x01010101 = 16843009，
 *           这个数字是 memset 最经典的坑。
 *   第 5 条：0x01020304 在内存里是 04 03 02 01，说明本机是小端；
 *           换成大端机器会打印 01 02 03 04，代码一行不用改。
 *   第 6 条：0x3f800000 是 1.0f 的标准 IEEE 754 位模式（符号 0、指数 127、
 *           尾数全 0）。用 memcpy 拿到它是合法的；用 *(unsigned int *)&f
 *           拿到同样的值，但那是 UB —— 同样的结果、不同的正确性。
 *   第 7 条：restrict 不是"运行时检查"，编译器不会替你验证承诺；
 *           真传了重叠缓冲区，行为就是未定义（可能算出错结果，也可能没问题）。
 * ==========================================================================
 */
