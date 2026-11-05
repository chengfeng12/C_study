/*
 * 07_函数进阶.c —— 阶段2 / 第 01 课 / 第 07 个文件：函数的进阶话题
 *
 * 编译：gcc -std=c99 -Wall -Wextra 07_函数进阶.c -o bin/07_函数进阶
 * 运行：./bin/07_函数进阶
 *
 * 前 6 篇把"普通函数"讲完了。这一篇补三块**天天在用、但前面没讲**的东西：
 *
 *      一、可变参数：printf 的 `...` 到底是什么
 *      二、inline 与函数式宏：为什么"小函数"有两种写法
 *      三、错误处理与程序退出：返回码、errno、exit、atexit
 *
 * 本文件最后一个 atexit 演示会在**程序退出时**才打印，
 * 所以文末结果里它的输出出现在"小结"之后，不是写错了顺序。
 *
 * ====================== 一、可变参数：... 是什么 ======================
 * 从第一天就在用的 printf，签名长这样：
 *
 *      int printf(const char *fmt, ...);
 *                                      ↑ 省略号（variadic）
 *
 * `...` 表示"后面还有任意多个参数，个数和类型调用时说"。
 * 三条规则：
 *
 *  (1) `...` 前面**至少要有一个有名参数**（printf 里是 fmt）。
 *      为什么？因为函数体里要靠它才知道"后面有几个、什么类型"。
 *      `void f(...);` 是非法声明。
 *
 *  (2) 编译器**无法检查** `...` 部分的类型。所以
 *          printf("%d\n", 3.14);        // 格式串说 int，实际传 double
 *      能编过，运行时打印垃圾值。格式串和实参必须自己配对。
 *
 *  (3) 取值时**必须知道类型**，因为 va_arg 不做任何转换检查：
 *          int x = va_arg(ap, int);       // 你说是 int，它就按 int 取
 *      取错了（比如实际是 double）就是未定义行为，可能读到错位的数据。
 *
 * ====================== 二、自己写可变参数函数：stdarg.h 四件套 ======================
 *
 *      #include <stdarg.h>
 *
 *      long sum_all(int n, ...)          // n 是"有名参数"，也就是锚点
 *      {
 *          va_list ap;                   // ① 游标：指向"下一个待取参数"
 *          va_start(ap, n);              // ② 绑定到最后一个有名参数
 *          long total = 0;
 *          for (int i = 0; i < n; i++)
 *              total += va_arg(ap, int); // ③ 取一个 int，游标自动前移
 *          va_end(ap);                   // ④ 收尾，必须和 va_start 配对
 *          return total;
 *      }
 *
 * 四个函数/宏的分工：
 *
 *      va_list      类型，保存"当前取到哪了"
 *      va_start     初始化游标，第二参数是**最后一个有名参数的名字**
 *      va_arg       取下一个值，第二参数是**类型**，游标自动往前走
 *      va_end       清理，不调用是未定义行为
 *
 * ★ **默认实参提升**：传进 `...` 的实参会先做整型提升/浮点提升：
 *
 *      char / short  →  int
 *      float         →  double
 *
 *   所以取的时候必须写 `va_arg(ap, int)` / `va_arg(ap, double)`，
 *   写 `va_arg(ap, char)` 或 `va_arg(ap, float)` 是错的。
 *
 * ★ 没有办法知道"参数取完了没有"。必须像 printf 那样，靠格式串、
 *   或者像 sum_all 那样，靠一个显式的个数参数。取多了就是读到栈上的垃圾。
 *
 * ====================== 三、inline：它只是"建议"，不是"命令" ======================
 *      inline int square(int x) { return x * x; }
 *
 * 作用是**建议**编译器把调用处直接展开成函数体，省掉压栈跳转。
 * 注意是"建议"：编译器完全可以选择不展开（比如函数太大、开了 -O0）。
 *
 * 实际工程里没人关心"提不提示"—— 现代编译器在 -O2 下会自己决定，
 * 小函数基本都会内联。inline 真正有用的场景是**头文件**：
 *
 *      // ---- utils.h ----
 *      static inline int max_int(int a, int b) { return a > b ? a : b; }
 *
 * 为什么必须同时写 static？
 *
 *    - 只写 `inline`：C99 里它的语义很绕，多个 .c 包含这个头文件时
 *      可能链接不到（undefined reference to 'max_int'）
 *    - 加上 `static`：每个 .c 各有一份**内部副本**，绝不会重复定义
 *
 * 所以实践结论：**头文件里的小函数一律写 `static inline`**，别只用 inline。
 * （04 篇第五节讲过这个"头文件只放声明"的例外；
 *   inline / _Noreturn / static / extern 的系统讲解见 10 篇。）
 *
 * ====================== 四、函数式宏 vs 函数 ======================
 * 有了函数，为什么还有人写宏？因为宏有两个函数做不到的能力：
 *
 *      - **类型无关**：#define MAX(a,b) 能比较 int、double、指针……
 *      - **可以操作符号本身**：#x 变成字符串，a##b 拼接标识符
 *
 * 但"用宏做计算"是非常危险的，看这个经典错误：
 *
 *      #define SQUARE_BAD(x) x * x
 *      SQUARE_BAD(1 + 2)          →  1 + 2 * 1 + 2  =  5   ✗ 应该是 9
 *
 * 三个坑，逐个看：
 *
 *  (1) **运算符优先级**：宏是纯文本替换，不加括号就会被拆散。
 *      改成 `#define SQUARE_OK(x) ((x) * (x))` 能解决这个。
 *
 *  (2) **多次求值**：`SQUARE_OK(i++)` 展开成 `((i++) * (i++))`，
 *      i 被加了两次（而且是未定义行为）。加多少括号都救不了 ——
 *      这是宏的**结构性缺陷**，只能靠"别传有副作用的表达式"来避。
 *
 *  (3) **无类型检查、难调试**：宏在预处理阶段就没了，编译器的报错指向
 *      展开后的代码，行号和变量名都可能对不上；函数则能单步调试。
 *
 * 结论：
 *      要"算一个值" → 用函数或 static inline，不要用宏
 *      要"类型无关的极简操作"（如 offsetof、MAX 用于多种类型）→ 谨慎用宏
 *      要"字符串化 / 拼接"（#、##）→ 只能用宏，没有替代品
 *
 * ====================== 五、错误处理与程序退出 ======================
 *
 *  (1) **返回码约定**（最常用）
 *          0        成功
 *          非 0     失败（-1 表示"出错了"，或自定义错误码）
 *
 *      调用方必须检查：
 *          if (fopen(...) == NULL) { ... }
 *      这是 C 里唯一通用的错误传递方式 —— 没有异常。
 *
 *  (2) **errno**：标准库出错时会把错误码放进全局变量 errno
 *          #include <errno.h>
 *          fp = fopen(...);
 *          if (fp == NULL) { perror("fopen"); }      // 打印 "fopen: No such file..."
 *      - errno 只在**函数失败后**才有意义，成功后不要读它
 *      - 调用其他库函数前，errno 可能被改掉，要立刻用
 *      - perror / strerror(errno) 能把错误码变成人能读的句子
 *
 *  (3) **exit vs return**
 *          return 0;        从 main 返回，等价于 exit(0)
 *          exit(1);         立刻结束整个程序（任何函数里都能调）
 *      区别在于位置：exit 可以在深层函数里直接终止程序 ——
 *      这既是"方便"，也是"灾难"：库函数里调 exit 会让调用方无法补救。
 *
 *      规则：**库函数永远不要 exit**，把错误返回给调用方去决定。
 *
 *  (4) **atexit**：注册"程序退出时执行"的函数（后注册的先执行）
 *          atexit(cleanup);
 *      用途：关文件、释放全局资源、写日志收尾。
 *      ★ exit() 会触发它，但 `abort()` 不会（abort 是异常终止）。
 *
 *  (5) `_Noreturn`（C11）：标注"这个函数不会返回"
 *          _Noreturn void die(const char *msg);
 *      给编译器看，避免"缺少 return"的误报，也能优化掉后续代码。
 *
 * ====================== 六、常见坑 ======================
 *  1. `void f(...)`                     省略号前必须有有名参数
 *  2. va_arg 类型写错                  不做检查，直接读错位（UB）
 *  3. 对 char/float 取 va_arg           默认提升后是 int/double
 *  4. 忘了 va_end                      未定义行为
 *  5. 取超过实际参数个数                读到栈上的垃圾
 *  6. 以为 inline 一定展开             它只是建议，-O0 下多半不展开
 *  7. 头文件里只写 inline 不加 static   多文件下可能链接失败
 *  8. 用宏做数学运算                   优先级 + 多次求值两大坑
 *  9. 不检查返回值/errno               错误被静默吞掉
 * 10. 库函数里调 exit                  调用方无法处理，只能整个进程退出
 *
 * ====================== 七、实践建议 ======================
 *  - 可选参数用"结构体 + 标志位"通常比可变参数更安全，只在写格式化/日志
 *    这类真的需要变参的接口时才用 `...`
 *  - 变参函数一定给一个能确定数量/类型的"锚点"（格式串、个数、结束标记 NULL）
 *  - 头文件里的小函数写 `static inline`，别只写 `inline`
 *  - 能写函数就别写宏；宏只留给 #、## 或真正需要类型无关的极简场景
 *  - 每个可能失败的函数都返回明确的状态，调用方必须检查
 *  - errno 用完立刻转成文字（perror/strerror），别留到后面
 *  - 库函数不 exit；清理工作交给 atexit 或调用方
 */

#include <stdio.h>
#include <stdarg.h>          /* va_list / va_start / va_arg / va_end */
#include <errno.h>           /* errno */
#include <stdlib.h>          /* exit / atexit */

/* ---- 可变参数：n 是锚点，告诉函数后面有几个 int ---- */
static long sum_all(int n, ...)
{
    va_list ap;
    va_start(ap, n);                 /* 绑到最后一个有名参数 n */
    long total = 0;
    for (int i = 0; i < n; i++)
        total += va_arg(ap, int);    /* 取一个 int，游标自动前移 */
    va_end(ap);                      /* 必须配对 */
    return total;
}

/* ---- 可变参数的另一形态：标签 + 个数 + 若干 int ---- */
static void print_tagged(const char *label, int n, ...) // n 是用来记录可变参数的个数
{
    va_list ap; // ap 用来存储可变参数
    va_start(ap, n); // 用来初始化 va_list 对象
    printf("   %s:", label);
    for (int i = 0; i < n; i++)
        printf(" %d", va_arg(ap, int));
    printf("\n");
    va_end(ap); // 结束可变参数的处理
}

/* ---- 头文件里该这么写：static inline，每个 .c 各一份副本 ---- */
static inline int max_int(int a, int b)
{
    return a > b ? a : b;
}

/* ---- 对照：普通函数（有类型检查、能调试） ---- */
static int square_fn(int x)
{
    return x * x;
}

/* ---- 宏的反面教材：不加括号 ---- */
#define SQUARE_BAD(x) x * x

/* ---- 加了括号，但"多次求值"依然无解 ---- */
#define SQUARE_OK(x) ((x) * (x))

/* ---- atexit 注册的清理函数 ---- */
static void cleanup(void)
{
    printf("   [atexit] 程序退出时执行：这里可以关文件、释放资源\n");
}

int main(void)
{
    /* ---------- 1. printf 的签名 ---------- */
    printf("1. printf 的签名：int printf(const char *fmt, ...);\n");
    int printed = printf("   这一行由 printf 打印\n");
    printf("   printf 的返回值 = %d（输出了多少个字符）\n", printed);
    printf("   ... 表示后面参数个数和类型都不定，靠 fmt 自己约定\n");

    /* ---------- 2. 自己写一个可变参数函数 ---------- */
    printf("2. 自己写：sum_all(int n, ...)，n 是锚点\n");
    printf("   sum_all(3, 10, 20, 30) = %ld\n", sum_all(3, 10, 20, 30));
    printf("   sum_all(5, 1, 2, 3, 4, 5) = %ld\n", sum_all(5, 1, 2, 3, 4, 5));
    printf("   va_list 是游标，va_arg 每取一次就往前走一格\n");
    print_tagged("带标签的调用", 4, 7, 8, 9, 10);
    printf("   注意：函数没法知道“取完了没有”，必须靠 n 或格式串\n");

    /* ---------- 3. inline ---------- */
    printf("3. inline 只是建议，不保证展开\n");
    printf("   max_int(3, 9) = %d（static inline，放在头文件里很常见）\n",
           max_int(3, 9));
    printf("   只写 inline 不加 static，多文件包含时可能链接不到\n");
    printf("   现代编译器在 -O2 下自己就会内联小函数，不必强求\n");

    /* ---------- 4. 函数式宏的三个坑 ---------- */
    printf("4. 函数式宏 vs 函数\n");
    printf("   #define SQUARE_BAD(x) x * x\n");
    printf("   SQUARE_BAD(1 + 2) = %d  ← 展开成 1 + 2 * 1 + 2，错了\n", SQUARE_BAD(1 + 2));
    printf("   #define SQUARE_OK(x) ((x) * (x))\n");
    printf("   SQUARE_OK(1 + 2)  = %d  ← 加括号就对了\n", SQUARE_OK(1 + 2));
    printf("   square_fn(1 + 2)  = %d  ← 函数天然正确，还能调试\n", square_fn(1 + 2));
    printf("   但 SQUARE_OK(i++) 依然会算两次 i++ —— 加括号救不了多次求值\n");

    /* ---------- 5. 错误处理：errno + perror ---------- */
    printf("5. 错误处理：返回值 + errno + perror\n");
    errno = 0;                            /* 先清零，避免读到上次的残留 */
    FILE *fp = fopen("/no_such_file_for_demo", "r");
    if (fp == NULL) {
        printf("   fopen 失败，errno = %d\n", errno);
        perror("   perror 输出");         /* 自动拼成 "前缀: 错误描述" */
        printf("   工程里用 strerror(errno) 拿到同样的一句话，好拼进日志\n");
    } else {
        fclose(fp);                       /* 成功也要记得关 */
    }

    /* ---------- 6. exit vs return ---------- */
    printf("6. return 从 main 返回，等价于 exit(0)\n");
    printf("   exit(n) 可以在任何函数里立刻结束整个程序\n");
    printf("   但库函数不该调用 exit —— 调用方连收拾残局的机会都没有\n");

    /* ---------- 7. atexit ---------- */
    printf("7. atexit：注册退出时执行的清理函数\n");
    if (atexit(cleanup) == 0)
        printf("   注册成功，稍后在本程序结束时你会看到那行输出\n");
    printf("   exit() 会触发它，abort() 不会（abort 是异常终止）\n");

    /* ---------- 8. 小结 ---------- */
    printf("8. 小结：\n");
    printf("    ... 前面必须有一个有名参数，类型靠约定，编译器不检查\n");
    printf("    va_list/va_start/va_arg/va_end 四件套，va_arg 的类型必须对\n");
    printf("    默认实参提升：char→int、float→double\n");
    printf("    inline 只是建议；头文件里的小函数写 static inline\n");
    printf("    能用函数就别用宏，宏的优先级和多次求值两大坑防不住\n");
    printf("    返回码 + errno + perror 是 C 的标准错误处理方式\n");
    printf("    库函数不 exit；清理交给 atexit 或调用方\n");

    return 0;
}
