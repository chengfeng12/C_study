/*
 * 01_内存存储区.c —— 阶段2 / 第 06 课 / 第 01 个文件：内存存储区（栈/堆/静态/常量/代码）
 *
 * 编译：gcc -std=c99 -Wall -Wextra 01_内存存储区.c -o bin/01_内存存储区
 * 运行：./bin/01_内存存储区
 *
 * 学 C 绕不开的一个问题：我写的这个变量，到底存在哪儿？
 * 答案只有五种，而且决定了四件事 —— **谁能看见它、活多久、多大、能不能改**。
 *
 * ====================== 一、先有一张图 ======================
 * 程序运行时，操作系统给每个进程一块**虚拟地址空间**，里面按用途切成了几段。
 * 以常见的 x86-64 Linux / macOS 为例（地址从高到低）：
 *
 *      高地址  ┌───────────────────────────┐
 *              │ 命令行参数 / 环境变量       │
 *              ├───────────────────────────┤
 *              │ 栈 stack        ↓ 向下长    │  ← 局部变量、函数参数、返回地址
 *              ├───────────────────────────┤
 *              │            （空闲）        │
 *              ├───────────────────────────┤
 *              │ 堆 heap         ↑ 向上长    │  ← malloc / calloc / realloc
 *              ├───────────────────────────┤
 *              │ .bss   未初始化/初值为 0    │  ← 全局、static，运行前清零
 *              ├───────────────────────────┤
 *              │ .data  已初始化的全局/静态  │
 *              ├───────────────────────────┤
 *              │ .rodata 只读数据            │  ← 字符串字面量、const 全局
 *              ├───────────────────────────┤
 *              │ .text  机器指令             │  ← 函数体
 *      低地址  └───────────────────────────┘
 *
 * ★ 记两点：
 *   1) 栈在高地址、堆在低地址，**两者相向生长**，中间的空闲区是它们争夺的地盘；
 *   2) 初学者最先要分清的是"**栈 vs 堆**"，因为这是"自动 vs 手动"的分界线。
 *
 * ★ 这是**虚拟**地址，不是物理内存。你在程序里打印出来的 0x7ffd... 是虚拟地址，
 *   由 MMU 映射到物理页，每个进程看到的地址空间都是独立的。
 *
 * ====================== 二、五个存储区逐个说 ======================
 *
 * 【1】栈区 stack —— 自动的，最快，也最小
 *   存什么：函数的局部变量（非 static）、参数、返回地址、保存的寄存器
 *   谁分配：**编译器自动**，函数一进入就挪栈指针，一返回就还原
 *   生命周期：随函数调用，函数返回即失效
 *   大小：默认通常 1–8 MB（Linux 常用 8 MB，macOS 主线程另有规定），
 *        **超了就栈溢出（stack overflow），不是返回 NULL，是直接崩**
 *   特点：分配只是"减一下栈指针"，几乎零开销；地址连续、缓存友好
 *
 *   ★ 两条铁律：
 *     - 绝不要返回局部变量的地址（函数返回后那块栈已被回收，见第六节）
 *     - 别在栈上开大数组，`int buf[1000000];` 就是 4 MB，很容易爆栈
 *
 * 【2】堆区 heap —— 手动的，最灵活，也最容易出错
 *   存什么：你 malloc 出来的任何东西
 *   谁分配：**你自己**（malloc/calloc/realloc），也必须**你自己** free
 *   生命周期：从 malloc 成功到 free，跨越函数调用，与作用域无关
 *   大小：受物理内存 + 交换空间限制，可以要几十 MB、几百 MB
 *   特点：地址不连续、可能碎片化；分配有开销；忘了 free 就是内存泄漏
 *
 *   ★ 需要"大小运行时才知道""要活过函数调用""对象太大"时，才用堆。
 *
 * 【3】全局 / 静态区（.data + .bss）
 *   存什么：全局变量、static 变量（含函数内的 static 局部变量）
 *   谁分配：**程序启动时由加载器**一次分配，结束时统一回收
 *   生命周期：整个程序运行期
 *   细分：
 *       .data  有显式初值且非 0：int g = 42;  static int s = 7;
 *       .bss   没写初值，或显式写 0：int g;  static int s;
 *              → 只记录"需要多大"，**不占可执行文件体积**，启动时清零
 *
 *   ★ static 修饰局部变量 = 把它从栈搬到静态区：
 *     生命周期变成全程，但**作用域仍局限在函数内**。这是"函数级私有状态"。
 *   ★ 默认初值为 0（这一点和栈上局部变量的"垃圾值"正相反）。
 *
 * 【4】常量区（.rodata，只读数据段）
 *   存什么：字符串字面量 "hello"、const 修饰的全局/静态对象、switch 跳转表等
 *   谁分配：编译期就写进可执行文件，加载时映射为**只读页**
 *   生命周期：全程
 *
 *   ★ 最经典的坑：
 *        char *s = "hello";   // s 指向只读区
 *        s[0] = 'H';          // ✗ 未定义行为，多数系统直接段错误
 *      正确写法二选一：
 *        const char *s = "hello";      // 只是不打算改，指过去就行
 *        char buf[] = "hello";         // 数组：把字面量**拷贝**到栈上，随便改
 *   ★ `const char *p = "abc"` 表示"我不改"，并不保证一定放只读区；
 *     但**写它一定是未定义行为**，两条路都要避免。
 *
 * 【5】代码区（.text）
 *   存什么：编译后的机器指令、函数体
 *   谁分配：加载器映射，通常只读 + 可执行
 *   生命周期：全程
 *   特点：同一个程序被运行多次，代码段在物理内存里通常**共享**一份
 *
 *   ★ 函数指针 `int (*fp)(int,int) = add;` 存的就是 .text 里的地址。
 *
 * ====================== 三、怎么亲眼验证 ======================
 * 本篇程序会打印各类变量的地址，你可以直接观察分布规律：
 *      栈上变量地址最高（0x7ffd... / 0x7ffee...）
 *      堆上地址明显更低，且每次 malloc 递增
 *      全局/静态地址居中（0x...601xxx / 0x10000xxxx）
 *      字符串字面量比全局变量更低一点（.rodata 在 .data 前面）
 *
 * 更精确的手段（命令行，不占仓库）：
 *      nm    a.out          # 看符号在哪个段：T=.text  D=.data  B=.bss  R=.rodata
 *      size  a.out          # 看 text/data/bss 各多少字节
 *      objdump -h a.out     # 看段表，macOS 用 otool -l
 *      gcc -fsanitize=address ...   # 查越界、重复释放、泄漏
 *
 * ★ 注意：段的具体布局、名字、地址都是**平台/编译器相关**的。
 *   本篇讲的是通用模型（C 标准只规定"对象有存储期"，并不规定分段），
 *   但几乎所有实现都长这样，理解它才能解释"为什么这么写会崩"。
 *
 * ====================== 四、存储期（storage duration）—— 标准术语对照 ======================
 * C 标准不叫"栈/堆"，叫**存储期**，四类，正好对应上面的区：
 *
 *      自动存储期  automatic     → 栈            局部变量（默认）
 *      动态存储期  allocated     → 堆            malloc 系列
 *      静态存储期  static        → .data/.bss    全局、static、字符串字面量
 *      线程存储期  thread        → TLS           _Thread_local（C11）
 *
 * ★ 面试常问："static 的作用"其实就是三件事，都源于它把存储期改成静态：
 *   1) 修饰局部变量：生命周期延长到全程（作用域不变）
 *   2) 修饰全局变量/函数：**链接属性改为内部链接**，别的 .c 文件看不见
 *   3) 修饰成员（C 没有）—— C++ 才有的用法，别混。
 *
 * ====================== 五、生命周期对照表 ======================
 *   存储区    | 内容              | 谁分配/释放      | 生命周期     | 初值    | 典型大小
 *   ---------|-------------------|-----------------|-------------|--------|----------
 *   栈        | 局部变量、参数     | 编译器自动       | 函数调用期间 | 垃圾值  | 1–8 MB
 *   堆        | malloc 的对象      | 程序员           | 到 free     | 垃圾值  | 受限于内存
 *   .data     | 有初值的全局/静态  | 加载器           | 全程        | 你写的值 | 可执行文件大小
 *   .bss      | 零初值全局/静态    | 加载器           | 全程        | 0       | 不占文件
 *   .rodata   | 字面量、const 全局 | 编译期           | 全程        | 编译期常量 | 只读
 *   .text     | 机器指令           | 加载器           | 全程        | —       | 只读可执行
 *
 * ====================== 六、五个必须记住的坑 ======================
 *  1. **返回局部变量的地址**
 *        char *bad(void) { char buf[32] = "hi"; return buf; }   // ✗ 返回即失效
 *     返回后那块栈可能已被别的函数覆盖，读出来是乱码或崩溃。
 *     正确做法：调用方传缓冲区，或 malloc 交给调用方 free（第 02 篇）。
 *
 *  2. **写字符串字面量**
 *        char *s = "abc"; s[0] = 'X';    // ✗ 只读区，段错误
 *     要改就用数组拷贝一份：char s[] = "abc";
 *
 *  3. **把 const 当成"放在只读区"的保证**
 *        const char *p = "abc";   // p 本身可改指向，指向的字符不可写
 *        const int g = 1;         // 也可能被优化成立即数，根本没有内存
 *     const 是**给编译器的承诺**，不是内存保护机制（真正的只读来自页权限）。
 *
 *  4. **以为栈上的变量有初值**
 *        局部 int x; 里面是什么？**不确定**，可能是上一个函数留下的垃圾。
 *     全局/静态变量才是默认 0。养成"声明即初始化"的习惯。
 *
 *  5. **大对象放栈上**
 *        void f(void) { int buf[1000000]; }   // 4 MB，多次递归立刻爆栈
 *     大缓冲区（>几十 KB）放堆上，或者用 static 挪到静态区。
 *
 * ====================== 七、实践建议 ======================
 *  - 默认用栈（自动存储期）：简单、快、不会泄漏，离开作用域自动回收
 *  - 只有"大小运行期才定""要活过函数""特别大"这三件事之一成立，才上堆
 *  - 全局变量只用于真正全局的状态（配置、计数器），并且尽量加 static 收窄可见性
 *  - static 局部变量用来做"只初始化一次"的缓存/计数，但注意它不是线程安全的
 *  - 需要修改的字符串一律用字符数组，别碰字面量
 *  - 写完带堆的代码，跑一遍 -fsanitize=address，比肉眼检查可靠得多
 */

#include <stdio.h>
#include <stdlib.h>     /* malloc / free */
#include <string.h>     /* memset / snprintf 需要的声明在 stdio.h */

/* ====================== 各区变量，供观察地址 ====================== */

int         g_data   = 42;              /* .data   有非零初值 */
int         g_bss;                      /* .bss    没有初值 → 自动清零 */
int         g_bss_zero = 0;             /* .bss    显式写 0 也进 .bss */
static int  g_static = 7;               /* .data   文件级私有 */
const char  g_ro[]   = "const global";  /* .rodata 只读 */
char        g_buf[1024];                /* .bss    1 KB 全 0，不占可执行文件 */

/*
 * 演示 static 局部变量：作用域只在函数内，但存储期是全程。
 * 变量本身在 .data（有初值 0 时在 .bss），函数返回后**依然存在**。
 */
static int next_id(void)
{
    static int counter = 0;             /* 只初始化一次 */

    return ++counter;
}

/*
 * 演示栈帧：递归越深，局部变量的地址越低（栈向下生长）。
 * 只打印地址、不往外传 —— 把局部变量地址传出去就是"悬垂指针"，见第七节。
 */
static void frame_probe(int depth)
{
    int local = depth;

    printf("   depth=%d  &local = %p\n", depth, (void *)&local);
    if (depth < 3)
        frame_probe(depth + 1);
}

/*
 * ✔ 正确做法之一：调用方提供缓冲区。
 * 对比反面例子 char *bad(void) { char buf[32] = "hi"; return buf; }
 */
static void fill_greeting(char *buf, size_t n)
{
    snprintf(buf, n, "hello from stack");
}

/* 打印 "标签 + 地址"，避免中文宽度对齐问题，不用 %-20s */
static void show_addr(const char *label, const void *p)
{
    printf("   %s %p\n", label, p);
}

int main(void)
{
    /* ---------- 1. 全局 / 静态区的地址 ---------- */
    printf("1. 全局、静态、常量区（地址居中，全程序唯一）\n");
    show_addr("g_data   (.data)   =", (void *)&g_data);
    show_addr("g_static (.data)   =", (void *)&g_static);
    show_addr("g_bss    (.bss)    =", (void *)&g_bss);
    show_addr("g_bss_zero (.bss)  =", (void *)&g_bss_zero);
    show_addr("g_buf    (.bss)    =", (void *)g_buf);
    show_addr("g_ro     (.rodata) =", (void *)g_ro);
    printf("   同一区里的变量地址相邻，且 .rodata 在 .data/.bss 之前（更低）\n");

    printf("   默认初值：g_bss = %d，g_bss_zero = %d（.bss 启动时清零）\n",
           g_bss, g_bss_zero);
    memset(g_buf, 0x41, sizeof g_buf);          /* 全局数组可写，演示一下 */
    printf("   g_buf 可写：sizeof = %zu，首字节 = %c\n",
           sizeof g_buf, g_buf[0]);

    /* ---------- 2. 栈区 ---------- */
    printf("2. 栈区（地址最高，函数返回即失效）\n");
    {
        int local = 123;
        char arr[16] = "stack";

        show_addr("&local (栈)        =", (void *)&local);
        show_addr("arr    (栈)        =", (void *)arr);
        printf("   栈上数组 char arr[16]：sizeof = %zu，内容可随意修改\n", sizeof arr);
        arr[0] = 'S';
        printf("   arr = %s\n", arr);
    }
    printf("   离开这个块之后，local 和 arr 都已经不存在了\n");

    printf("   递归观察栈向下生长：\n");
    frame_probe(0);
    printf("   地址逐层变小 → 栈从高地址向低地址生长\n");

    /* ---------- 3. 堆区 ---------- */
    printf("3. 堆区（地址明显低于栈，malloc 得到）\n");
    {
        size_t const n = 4;
        int *heap = malloc(n * sizeof *heap);   /* C 里不需要强转，见第 02 篇 */

        if (heap == NULL) {                      /* 必须判空 */
            printf("   malloc 失败\n");
            return 1;
        }
        for (size_t i = 0; i < n; i++)
            heap[i] = (int)(i * 10);

        show_addr("heap (堆)          =", (void *)heap);
        printf("   heap[0..3] = %d %d %d %d\n", heap[0], heap[1], heap[2], heap[3]);
        printf("   它在函数返回后依然有效，直到 free(heap)\n");

        free(heap);
        heap = NULL;                             /* 释放后立刻置空，防野指针 */
        printf("   free 之后指针已置 NULL，再访问会明确崩溃而不是读到脏数据\n");
    }

    /* ---------- 4. 栈与堆的地址高低对比 ---------- */
    printf("4. 栈比堆高得多，两者相向生长\n");
    {
        int on_stack = 0;
        int *on_heap = malloc(sizeof *on_heap);

        if (on_heap != NULL) {
            *on_heap = 1;
            printf("   栈变量 %p  >  堆对象 %p  ?  %s\n",
                   (void *)&on_stack, (void *)on_heap,
                   (unsigned long)&on_stack > (unsigned long)on_heap ? "是" : "否");
            free(on_heap);
            on_heap = NULL;
        }
        printf("   注意：第一节的图是典型的 Linux 布局；macOS 上 malloc 的区常在\n");
        printf("   数据段之上、栈之下（形如 0x100xxxxxx），但同样远低于栈\n");
    }

    /* ---------- 5. static 局部变量：作用域 vs 生命周期 ---------- */
    printf("5. static 局部变量：把栈变成静态区\n");
    printf("   连续调用 next_id() 三次：%d %d %d\n",
           next_id(), next_id(), next_id());
    printf("   counter 的作用域仍在 next_id 内部，但它的存储期是全程，\n");
    printf("   所以每次调用都接着上次的值累加，不会重置\n");

    /* ---------- 6. 字符串字面量在只读区 ---------- */
    printf("6. 字符串字面量 vs 字符数组\n");
    {
        const char *lit = "hello";        /* 指向 .rodata */
        char copy[]    = "hello";         /* 栈上副本 */

        show_addr("字面量 \"hello\" (只读) =", (void *)lit);
        show_addr("char copy[]    (栈)     =", (void *)copy);
        printf("   lit[0] = 'X' 会段错误（只读页），所以声明为 const char *\n");

        copy[0] = 'H';                    /* ✔ 数组在栈上，可以改 */
        printf("   copy[0] = 'H' 之后 copy = %s（改的是自己的副本）\n", copy);
        printf("   字面量 lit 内容不变：%s\n", lit);
    }

    /* ---------- 7. 返回局部变量地址是错的 ---------- */
    printf("7. 返回局部变量的地址：函数返回后那块栈已失效\n");
    {
        char buf[64];

        fill_greeting(buf, sizeof buf);   /* ✔ 调用方给缓冲区 */
        printf("   正确做法（调用方给缓冲区）：buf = %s\n", buf);
        printf("   错误写法：char *bad(void) { char buf[32] = \"hi\"; return buf; }\n");
        printf("   返回的是栈地址，函数一退出就被回收，gcc 会直接给 -Wreturn-local-addr 警告\n");
        printf("   真要交出所有权，用 malloc（第 02 篇），由调用方负责 free\n");
    }

    /* ---------- 8. 各区大小估算 ---------- */
    printf("8. 各区大小的粗略感受（编译期常量）\n");
    printf("   sizeof(g_buf)  = %zu   ← .bss，1 KB 不进可执行文件\n", sizeof g_buf);
    printf("   sizeof(g_data) = %zu   ← 单个 int\n", sizeof g_data);
    printf("   用 size a.out / nm a.out 可以看到 text / data / bss 的真实字节数\n");

    /* ---------- 9. 小结 ---------- */
    printf("9. 小结：\n");
    printf("     栈：编译器管，快、小、随函数生灭，初值是垃圾\n");
    printf("     堆：你管，灵活、可跨函数，忘 free 就泄漏\n");
    printf("     .data/.bss：全局和 static，全程存在，默认 0，.bss 不占文件\n");
    printf("     .rodata：字面量与 const 全局，只读，写它就崩\n");
    printf("     .text：机器指令，只读可执行，函数指针指向这里\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 地址随平台、系统、每次运行（ASLR）而变，数值不同属正常，**看相对高低**即可。
 *
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 01_内存存储区.c -o bin/01_内存存储区
 *     ./bin/01_内存存储区
 *
 * 输出示意：
 *
 * 1. 全局、静态、常量区（地址居中，全程序唯一）
 *    g_data   (.data)   = 0x10044c000
 *    g_static (.data)   = 0x10044c004
 *    g_bss    (.bss)    = 0x10044c00c
 *    g_bss_zero (.bss)  = 0x10044c008
 *    g_buf    (.bss)    = 0x10044c010
 *    g_ro     (.rodata) = 0x100447470
 *    同一区里的变量地址相邻，且 .rodata 在 .data/.bss 之前（更低）
 *    默认初值：g_bss = 0，g_bss_zero = 0（.bss 启动时清零）
 *    g_buf 可写：sizeof = 1024，首字节 = A
 * 2. 栈区（地址最高，函数返回即失效）
 *    &local (栈)        = 0x16f9b9b28
 *    arr    (栈)        = 0x16f9b9b70
 *    栈上数组 char arr[16]：sizeof = 16，内容可随意修改
 *    arr = Stack
 *    离开这个块之后，local 和 arr 都已经不存在了
 *    递归观察栈向下生长：
 *    depth=0  &local = 0x16f9b9a58
 *    depth=1  &local = 0x16f9b9a28
 *    depth=2  &local = 0x16f9b99f8
 *    depth=3  &local = 0x16f9b99c8
 *    地址逐层变小 → 栈从高地址向低地址生长
 * 3. 堆区（地址明显低于栈，malloc 得到）
 *    heap (堆)          = 0x100999e70
 *    heap[0..3] = 0 10 20 30
 *    它在函数返回后依然有效，直到 free(heap)
 *    free 之后指针已置 NULL，再访问会明确崩溃而不是读到脏数据
 * 4. 栈比堆高得多，两者相向生长
 *    栈变量 0x16f9b9b0c  >  堆对象 0x100999e70  ?  是
 *    注意：第一节的图是典型的 Linux 布局；macOS 上 malloc 的区常在
 *    数据段之上、栈之下（形如 0x100xxxxxx），但同样远低于栈
 * 5. static 局部变量：把栈变成静态区
 *    连续调用 next_id() 三次：1 2 3
 *    counter 的作用域仍在 next_id 内部，但它的存储期是全程，
 *    所以每次调用都接着上次的值累加，不会重置
 * 6. 字符串字面量 vs 字符数组
 *    字面量 "hello" (只读) = 0x100447a96
 *    char copy[]    (栈)     = 0x16f9b9af0
 *    lit[0] = 'X' 会段错误（只读页），所以声明为 const char *
 *    copy[0] = 'H' 之后 copy = Hello（改的是自己的副本）
 *    字面量 lit 内容不变：hello
 * 7. 返回局部变量的地址：函数返回后那块栈已失效
 *    正确做法（调用方给缓冲区）：buf = hello from stack
 *    错误写法：char *bad(void) { char buf[32] = "hi"; return buf; }
 *    返回的是栈地址，函数一退出就被回收，gcc 会直接给 -Wreturn-local-addr 警告
 *    真要交出所有权，用 malloc（第 02 篇），由调用方负责 free
 * 8. 各区大小的粗略感受（编译期常量）
 *    sizeof(g_buf)  = 1024   ← .bss，1 KB 不进可执行文件
 *    sizeof(g_data) = 4   ← 单个 int
 *    用 size a.out / nm a.out 可以看到 text / data / bss 的真实字节数
 * 9. 小结：
 *     栈：编译器管，快、小、随函数生灭，初值是垃圾
 *     堆：你管，灵活、可跨函数，忘 free 就泄漏
 *     .data/.bss：全局和 static，全程存在，默认 0，.bss 不占文件
 *     .rodata：字面量与 const 全局，只读，写它就崩
 *     .text：机器指令，只读可执行，函数指针指向这里
 * ==========================================================================
 */
