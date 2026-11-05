/*
 * 10_函数说明符.c —— 阶段2 / 第 01 课 / 第 10 个文件：函数说明符
 *
 * 编译：gcc -std=c11 -Wall -Wextra 10_函数说明符.c -o bin/10_函数说明符
 * 运行：./bin/10_函数说明符
 *
 * 注意本篇用 -std=c11，不是前面统一的 c99：
 * `_Noreturn` 是 C11 才有的函数说明符，c99 模式下不认这个关键字。
 * 其余各篇没有用到 C11 特性，仍按 c99 编译。
 *
 * ====================== 一、先划清术语 ======================
 * 标准里"函数说明符"（function specifier）只有两个，C11 §6.7.4：
 *
 *      inline       建议内联展开
 *      _Noreturn    声明"这个函数永不返回"
 *
 * 就这么两个。日常说的"函数前面那一堆词"其实混了三类东西：
 *
 *      存储类说明符   static / extern        —— 管"链接属性"（谁能看见）
 *      函数说明符     inline / _Noreturn     —— 管"怎么用这个函数"
 *      类型限定符     const / restrict       —— 管"能不能改"，是类型的一部分
 *
 * 三者可以同时出现，顺序自由：
 *
 *      static inline int max_int(int, int);        // 存储类 + 函数说明符
 *      _Noreturn static void fatal(const char *);  // 函数说明符 + 存储类
 *
 * 这一篇把这三类都讲清楚，因为它们总是挤在同一行里，分不清就会用错。
 *
 * ====================== 二、inline：一半是建议，一半是链接语义 ======================
 * 表面作用：建议编译器把调用处**直接展开**成函数体，省掉压栈、跳转、返回。
 *
 *      inline int square(int x) { return x * x; }   // 只是"建议"
 *
 * 关键：标准**不要求**编译器接受建议。函数太大、开了 -O0、递归函数，
 * 编译器都可以不展开 —— 代码语义完全不变，只是没省下调用开销。
 * 所以不要用 inline 表达"必须内联"，它做不到。
 *
 * ★ 真正难的是**链接语义**（C99 起）：带 inline 的函数定义是"内联定义"，
 *   它**不产生外部可见的定义**。单独一个 .c 里写：
 *
 *        inline int square(int x) { return x * x; }
 *        int main(void) { return square(3); }
 *
 *   在 -O0 下可能链接失败：undefined reference to `square`。
 *   因为编译器可以决定"不展开"，但标准又说这个定义对外不可见，两边一撞就没人提供符号。
 *
 *   两种正确写法：
 *
 *    (1) 头文件里的小函数 → `static inline`
 *            static inline int max_int(int a, int b) { return a > b ? a : b; }
 *        每个包含它的 .c 各有一份**内部副本**（内部链接），绝不重复定义。
 *        代价：每个文件都有一份代码，二进制略大 —— 这正是 inline 的取舍。
 *
 *    (2) 想在多个 .c 之间共享一份实现 → 一个 .c 里写 `extern inline` 或
 *        普通（不加 inline）定义，头文件里只放声明。
 *
 * ★ 实践结论只有一条：**头文件里的小函数写 static inline；其余别加 inline。**
 *   现代编译器在 -O2 下自己会判断该不该内联，人写的 inline 基本不影响结果。
 *
 * ★ inline 不是宏。宏是预处理期文本替换（没有类型检查、可能多次求值），
 *   inline 是真正的函数（有类型检查、参数只求值一次），只是可能被展开。
 *   能用 static inline 的地方，不要用函数式宏（07 篇讲过宏的三个坑）。
 *
 * ====================== 三、_Noreturn：告诉编译器"别等它回来" ======================
 *
 *      _Noreturn void fatal(const char *msg);
 *
 * 语义：这个函数**保证不会返回到调用点**（返回 = 执行到函数末尾或 return）。
 * 它只能以 exit / abort / 长跳转 / 死循环 / 崩溃结束。
 *
 * 编译器拿到这个信息后能做三件事：
 *
 *  1. **不报"缺 return"**：下面这个函数没有 return，但 -Wall 不会警告，
 *     因为编译器知道 `fatal` 那条路径不会回来：
 *          int positive_only(int x)
 *          {
 *              if (x <= 0) fatal("x 必须为正数");   // 不返回
 *              return x;                            // 其余路径必到
 *          }
 *
 *  2. **不生成返回路径**，调用后不需要恢复现场，代码略小略快（教学意义大于实际）。
 *
 *  3. **后续代码视为不可达**，可以做死代码消除：
 *          fatal("boom");
 *          printf("永远不会执行");      // 编译器知道这行没用
 *
 * ★ 反过来，**标了 _Noreturn 却真的返回了是未定义行为**：
 *          _Noreturn void oops(void) { return; }    // UB
 *   编译器会按"不会回来"生成代码，栈帧、寄存器都可能被提前清理。
 *   所以只给 exit/abort 这类"铁定不返回"的函数加，别给"正常情况下会返回"的加。
 *
 * ★ 写法上的三个细节：
 *   - 声明和定义都要写 `_Noreturn`，只在定义写会让调用方看不到这个信息
 *   - C11 提供了 <stdnoreturn.h>，把 `_Noreturn` 包成宏 `noreturn`，
 *     写 `noreturn void fatal(...)` 更顺眼；但 C23 起这个头文件被移除了
 *   - C23 改用属性写法 `[[noreturn]] void fatal(...)`，`_Noreturn` 已废弃；
 *     gcc 的扩展写法是 `__attribute__((noreturn))`（老代码里很常见）
 *
 * ★ 和 `void` 的区别：`void f(void)` 表示"没有返回值"，但**会返回**；
 *   `_Noreturn void f(void)` 表示"连返回都不做"。前者是返回类型，后者是说明符。
 *
 * ====================== 四、存储类说明符：static / extern ======================
 * 这两个不是"函数说明符"，但天天和 inline 一起出现，必须分清。
 * 它们决定函数的**链接属性**（linkage），也就是"别的 .c 能不能看到这个名字"。
 *
 *      static int helper(void);     // 内部链接：只在本翻译单元（本 .c）可见
 *      int        public_api(void);  // 外部链接（默认）：别的 .c 可以 extern 声明来用
 *      extern int public_api(void);  // 显式声明"这个函数在别处定义"
 *
 * 要点：
 *  - 函数**只能是 static 或 extern**，不能是 auto / register（那两个是给变量的）
 *  - `static` 函数的两个好处：
 *      ① 不和别的 .c 里的同名函数冲突（各写各的 helper，互不干扰）
 *      ② 编译器知道它不会被外部调用，可以更激进地内联、删掉未使用的
 *  - `static` 也是**最好的文档**：看到 static 就知道"不用去别的文件找调用者"
 *  - 头文件里**只放声明**，不要放（非 inline 的）函数定义 ——
 *    否则每个包含它的 .c 都会有一份定义，链接时 multiple definition
 *    （`static inline` 是这条规则的唯一例外）
 *  - `extern` 用在**声明**上；定义时不用写（写了也不错）
 *
 * ★ 一个常见误解：`static` 在函数上和在局部变量上完全不是一回事。
 *   函数上的 static = 内部链接；局部变量上的 static = 静态存储期（06 篇）。
 *   同一个关键字，两个不相干的含义。
 *
 * ====================== 五、const / restrict：是类型限定符，不是函数说明符 ======================
 * 它们不改变函数的调用方式，只约束"指向的数据能不能改、能不能别名"。
 *
 * (1) **const 在返回值上**：表示"我给你的指针指向只读数据"
 *          const char *version(void);        // 调用方不能改返回值
 *     漏写 const 的后果：返回字面量却声明成 `char *`，调用方一写就段错误。
 *
 * (2) **const 在参数上**：表示"我不改你"
 *          size_t my_strlen(const char *s);
 *     好处：既能接受字面量，也能接受普通数组；同时是接口契约的一部分。
 *
 * (3) **restrict 在参数上**（C99）：承诺"这块内存只能通过这个指针访问"
 *          void *memcpy(void *restrict dst, const void *restrict src, size_t n);
 *     编译器据此省掉别名检查、放开向量化。
 *     ★ 承诺不成立（src 和 dst 重叠）就是未定义行为 —— memcpy 不能处理重叠，
 *       处理重叠要用 memmove。
 *
 * 记忆法：
 *      const      只读，编译器强制
 *      restrict   独占，靠你保证，编译器相信你
 *
 * ====================== 六、常见坑 ======================
 *  1. 头文件里只写 inline 不加 static    多文件包含时可能链接失败
 *  2. 以为 inline 一定展开              它只是建议，-O0 下多半不展开
 *  3. 给"其实会返回"的函数加 _Noreturn  未定义行为，且编译器不再检查缺 return
 *  4. _Noreturn 只写在定义、没写在声明  调用方看不到，优化和诊断都失效
 *  5. 把 const 当函数说明符用            `const int f(void)` 是"返回 const int"，几乎无意义；
 *                                       有用的是 `const char *f(void)`（指针指向只读）
 *  6. 函数定义写进头文件                 multiple definition；除非是 static inline
 *  7. static 函数忘了用                 同名符号在链接期互相覆盖（外部链接重名会报错）
 *  8. 用 restrict 却传了重叠的内存       UB，且只在开了优化的构建里出错
 *  9. c99 模式下写 _Noreturn             不认识这个关键字，直接编译失败
 *
 * ====================== 七、实践建议 ======================
 *  - 头文件里的小函数：`static inline`，一行写完
 *  - 只在单个 .c 里用的函数：一律 `static`，缩小可见范围
 *  - 每个可能失败的接口，返回值要能表达失败；`_Noreturn` 只留给 fatal/abort
 *  - 返回值指向只读数据时，返回类型必须 `const` 修饰
 *  - 参数只读就加 const，这是零成本的接口文档
 *  - restrict 只在你**真的确定**不重叠时用，否则会引入只在 -O2 下复现的 bug
 *  - 别为了"更快"手工加 inline；先 -O2 测，再 profile，最后才谈优化
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>     /* C11：把 _Noreturn 包成 noreturn 宏 */

/* ---- 存储类：内部链接，只在本文件可见 ---- */
static int square(int x)
{
    return x * x;
}

/* ---- 函数说明符 + 存储类：头文件里最该出现的组合 ---- */
static inline int max_int(int a, int b)
{
    return a > b ? a : b;
}

/* ---- 对照：宏做同样的事，但没有类型检查、可能多次求值 ---- */
#define MAX_MACRO(a, b) ((a) > (b) ? (a) : (b))

/* ---- 外部链接：别的 .c 写上 extern 声明就能用 ---- */
extern int public_double(int x);   /* 声明：告诉编译器"这个符号在别处定义" */

int public_double(int x)           /* 定义：不加 static，默认外部链接 */
{
    return 2 * x;
}

/* ---- 函数说明符：声明"永不返回"。声明和定义都写 ---- */
noreturn static void fatal(const char *msg)
{
    fprintf(stderr, "   [fatal] %s\n", msg);
    exit(EXIT_FAILURE);            /* exit 不返回，符合 _Noreturn 的承诺 */
}

/* ---- 受益于 _Noreturn：一条分支不返回，函数末尾没有 return 也不报警 ---- */
static int positive_only(int x)
{
    if (x <= 0)
        fatal("x 必须为正数");     /* 编译器知道这里不会回来 */
    return x;                      /* 所以只有一条路径需要 return */
}

/* ---- 类型限定符：返回只读数据，必须写 const ---- */
static const char *version(void)
{
    return "v1.0";                 /* 字面量在只读段，类型必须是 const char * */
}

/* ---- 类型限定符：参数只读 + restrict 承诺不重叠 ---- */
static void my_copy(char *restrict dst, const char *restrict src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        dst[i] = src[i];
}

int main(void)
{
    /* ---------- 1. 三类说明符的分工 ---------- */
    printf("1. 函数前面那一堆词，其实分三类\n");
    printf("   存储类说明符：static / extern        管链接属性（谁能看见）\n");
    printf("   函数说明符  ：inline / _Noreturn     管怎么用这个函数\n");
    printf("   类型限定符  ：const / restrict       管能不能改，属于类型\n");
    printf("   标准里的函数说明符只有两个，别把三类混成一类\n");

    /* ---------- 2. inline ---------- */
    printf("2. inline 是建议，不是命令\n");
    printf("   max_int(3, 9) = %d（static inline，头文件的标准写法）\n",
           max_int(3, 9));
    printf("   展开与否由编译器定：-O0 下多半还是普通调用\n");
    printf("   只写 inline 不加 static：C99 语义下可能链接失败\n");
    printf("   结论：头文件小函数写 static inline，其余别加 inline\n");

    /* ---------- 3. inline vs 宏 ---------- */
    printf("3. static inline 能替代大部分函数式宏\n");
    printf("   MAX_MACRO(1 + 2, 3) = %d（宏，参数被文本替换）\n", MAX_MACRO(1 + 2, 3));
    printf("   max_int(1 + 2, 3)   = %d（函数，参数只求值一次）\n", max_int(1 + 2, 3));
    printf("   差别在于：宏能骗过类型检查，也能把 i++ 算两次\n");

    /* ---------- 4. _Noreturn ---------- */
    printf("4. _Noreturn：声明“永不返回”\n");
    printf("   positive_only(7) = %d\n", positive_only(7));
    printf("   函数里没有 else 也没有末尾 return，-Wall 却不警告\n");
    printf("   因为 fatal 标了 noreturn，编译器知道那条路不会回来\n");
    printf("   反过来：标了 noreturn 却真的 return，是未定义行为\n");
    printf("   本程序不调用 fatal(\"boom\")，否则进程会在这里退出\n");

    /* ---------- 5. static / extern ---------- */
    printf("5. static = 内部链接，extern = 外部链接\n");
    printf("   square(6) = %d（static，只在本文件可见，别的 .c 调不到）\n", square(6));
    printf("   public_double(6) = %d（外部链接，别的 .c 用 extern 声明即可）\n",
           public_double(6));
    printf("   static 也是文档：看到它就知道不用去别的文件找调用者\n");

    /* ---------- 6. const / restrict ---------- */
    printf("6. const / restrict 是类型限定符，不是函数说明符\n");
    printf("   version() = %s（返回 const char *，调用方不能改）\n", version());

    char src[] = "hello";
    char dst[8];
    my_copy(dst, src, sizeof src);        /* 不重叠，符合 restrict 的承诺 */
    printf("   my_copy(dst, \"hello\") → dst = %s\n", dst);
    printf("   restrict 的承诺靠调用方保证；src 和 dst 重叠就是 UB\n");

    /* ---------- 7. 小结 ---------- */
    printf("7. 小结：\n");
    printf("    函数说明符只有 inline 和 _Noreturn 两个\n");
    printf("    inline 是建议；头文件里一律写 static inline\n");
    printf("    _Noreturn 表示永不返回，标错就是 UB，声明和定义都要写\n");
    printf("    static 管链接（内部可见），extern 管声明（在别处定义）\n");
    printf("    const 管只读，restrict 管不重叠，两者都不是函数说明符\n");
    printf("    头文件只放声明；函数定义放 .c，唯一例外是 static inline\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c11 -Wall -Wextra 10_函数说明符.c -o bin/10_函数说明符
 *     ./bin/10_函数说明符
 *
 * 输出如下：
 *
 * 1. 函数前面那一堆词，其实分三类
 *    存储类说明符：static / extern        管链接属性（谁能看见）
 *    函数说明符  ：inline / _Noreturn     管怎么用这个函数
 *    类型限定符  ：const / restrict       管能不能改，属于类型
 *    标准里的函数说明符只有两个，别把三类混成一类
 * 2. inline 是建议，不是命令
 *    max_int(3, 9) = 9（static inline，头文件的标准写法）
 *    展开与否由编译器定：-O0 下多半还是普通调用
 *    只写 inline 不加 static：C99 语义下可能链接失败
 *    结论：头文件小函数写 static inline，其余别加 inline
 * 3. static inline 能替代大部分函数式宏
 *    MAX_MACRO(1 + 2, 3) = 3（宏，参数被文本替换）
 *    max_int(1 + 2, 3)   = 3（函数，参数只求值一次）
 *    差别在于：宏能骗过类型检查，也能把 i++ 算两次
 * 4. _Noreturn：声明“永不返回”
 *    positive_only(7) = 7
 *    函数里没有 else 也没有末尾 return，-Wall 却不警告
 *    因为 fatal 标了 noreturn，编译器知道那条路不会回来
 *    反过来：标了 noreturn 却真的 return，是未定义行为
 *    本程序不调用 fatal("boom")，否则进程会在这里退出
 * 5. static = 内部链接，extern = 外部链接
 *    square(6) = 36（static，只在本文件可见，别的 .c 调不到）
 *    public_double(6) = 12（外部链接，别的 .c 用 extern 声明即可）
 *    static 也是文档：看到它就知道不用去别的文件找调用者
 * 6. const / restrict 是类型限定符，不是函数说明符
 *    version() = v1.0（返回 const char *，调用方不能改）
 *    my_copy(dst, "hello") → dst = hello
 *    restrict 的承诺靠调用方保证；src 和 dst 重叠就是 UB
 * 7. 小结：
 *     函数说明符只有 inline 和 _Noreturn 两个
 *     inline 是建议；头文件里一律写 static inline
 *     _Noreturn 表示永不返回，标错就是 UB，声明和定义都要写
 *     static 管链接（内部可见），extern 管声明（在别处定义）
 *     const 管只读，restrict 管不重叠，两者都不是函数说明符
 *     头文件只放声明；函数定义放 .c，唯一例外是 static inline
 * ==========================================================================
 */
