/*
 * 10_变量间的运算规则.c —— 阶段1 / 第 02 课 / 第 10 个文件：变量间的运算规则与类型转换
 *
 * 编译：gcc 10_变量间的运算规则.c -o conv_demo
 * 运行：./conv_demo
 *
 * ================== 一、为什么需要"变量间的运算规则" ==================
 * 前面几篇讲的都是"单个变量"。但真实代码里经常出现：
 *      int    a = 10;
 *      double b = 2.5;
 *      a + b        ← 一个是整数，一个是小数，怎么加？
 * 计算机的加法器只有"整数加法"和"浮点加法"两套电路，
 * 没办法直接拿一个 4 字节整数和一个 8 字节浮点数相加。
 *
 * 所以 C 的规则是：
 *
 *      ★ 运算前，先把所有操作数转换成"同一种类型"，然后再运算。
 *
 * 这个转换有两种发生方式：
 *      隐式转换（自动）：编译器偷偷做，你看不到代码，但它确实发生了
 *      强制转换（显式）：你自己写 (类型) 明确要求转换
 *
 * 隐式转换发生在这三种场合：
 *   (1) 算术运算中两边的类型不同        a + b
 *   (2) 赋值时左右两边类型不同          int x = 3.99;
 *   (3) 函数调用传参 / return 返回值     printf("%f", 1)  ← 传参时转换
 *
 * 转换的方向基本是两个：
 *      窄 → 宽：安全，值不变（本篇重点，对应课件"情况1"）
 *      宽 → 窄：危险，可能丢数据（截断）
 *
 * ============ 二、隐式转换 · 情况 1：窄类型自动转成宽类型 ============
 * 这是最常见、也最安全的转换，课件里叫"情况 1"：
 *
 *      系统自动把"字节宽度较小"的类型，转换成"字节宽度较大"的类型。
 *
 * 打个比方：小杯子里的水倒进大杯子，一滴都不会洒。
 *
 * 两条主要的"变宽"路线：
 *
 *   路线 A：整数之间
 *      char(1) → short(2) → int(4) → long(8) → long long(8)
 *      宽度只增不减（具体宽度看平台，见 06_整型.c）
 *
 *   路线 B：整数 → 浮点
 *      int → long → float → double → long double
 *      一旦浮点参与运算，另一边会被转成浮点，运算也变成浮点运算
 *
 * ★ 2.1 整型提升（integer promotion）——最容易被忽略的一次转换
 *   在 C 里，凡是比 int 窄的类型（char、short、_Bool，以及它们的位域），
 *   参与运算前都会先被"提升"成 int，哪怕两边都是 char！
 *
 *      char a = 'a', b = 'b';
 *      a + b       →  实际是 (int)a + (int)b，结果是 int 的 195
 *      sizeof(a + b) == 4      ← 不是 1！
 *
 *   为什么要提升：CPU 的算术单元本来就是按 int 宽度设计的，
 *   1 字节的小数在寄存器里也要占一个完整寄存器，提升是"顺手的事"。
 *
 *   记住这条推论：char + char 的结果不是 char，是 int。
 *   所以 char c = 'a' + 1; 能正常工作，但如果你写
 *      char c = 200 + 100;   → 300 超出 char 范围，结果是"由实现决定"的截断值
 *
 * ★ 2.2 不同宽度整数混合运算
 *      char + int        → int
 *      short + int       → int
 *      int + long        → long
 *      int + long long   → long long
 *      char + char       → int（提升）
 *   规则一句话：谁宽听谁的。
 *
 * ★ 2.3 整数与浮点混合
 *      int + float       → float
 *      int + double      → double
 *      float + double    → double
 *   规则：浮点比整数"宽"，所以整数先变成浮点。
 *   注意代价：整数转 float 可能丢精度（float 只有约 6~7 位有效数字），
 *   特别大的整数（超过 2^24）转成 float 后，低位会消失。
 *
 * ================== 三、隐式转换 · 情况 2：有符号 + 无符号 ==================
 * 这条不在课件的"情况 1"里，但它是 C 里最著名的坑，必须单独说：
 *
 *      ★ 当 signed 和 unsigned 相遇，且宽度相同时，
 *        signed 会被转换成 unsigned 再比较/运算。
 *
 * 于是出现了这个经典结论：
 *      int a = -1;
 *      unsigned int b = 1;
 *      a < b        →  结果为"假"！
 * 因为 -1 被转成了 unsigned，变成 4294967295，当然大于 1。
 *
 * 而且编译器往往不会警告（-Wall -Wextra 下也可能只有 -Wsign-compare 才报），
 * 所以这类 bug 特别难查：条件看起来完全合理，结果却反了。
 *
 * 宽度不同时，先按"谁宽听谁的"提升，再判断符号：
 *      unsigned int + long  → 平台相关（long 能否装下 unsigned int）：
 *          装得下 → 转成 long（有符号）
 *          装不下 → 两边都转成 unsigned long
 *      这种"平台相关"就是不要混用的最好理由。
 *
 * 实践建议：
 *      - 表示"个数、长度、下标"用 unsigned / size_t，全流程保持 unsigned
 *      - 一旦要和非负性不确定的量比较，先统一类型，或显式转换后加注释
 *      - 减法前想清楚会不会减出负数，unsigned 减出负数会回绕成巨大的数
 *      - 打开 -Wsign-compare 让编译器帮你查
 *
 * ================== 四、隐式转换 · 情况 3：宽 → 窄（赋值/传参）==================
 * 前面两种情况是"运算时"的转换，第三种发生在"赋值"和"函数调用"时：
 *
 *      double d = 3.99;
 *      int    i = d;          ← 宽 → 窄，小数部分被直接丢掉，i 得 3
 *
 *      int    n = 300;
 *      char   c = n;          ← 300 装不进 1 字节的 char，结果是"由实现决定"
 *
 * 三条必须记住的规则：
 *   1) 浮点 → 整数：直接截断小数部分（不是四舍五入），向零方向
 *          (int)3.99  →  3
 *          (int)-3.99 →  -3
 *   2) 宽整数 → 窄整数：只保留低位字节，高位被丢弃（相当于对 2^N 取模）
 *          300 → char：300 - 256 = 44（有符号 char 下是 44）
 *   3) 超出目标类型能表示的范围时，严格来说属于"未定义行为"或"由实现决定"，
 *      不能依赖具体结果，写代码时应当主动避免。
 *
 * 为什么 C 不报错：C 的设计哲学是"信任程序员，不为运行时检查买单"。
 * 编译器只在你开启 -Wconversion 这类警告时才提醒。
 *
 * ================== 五、完整顺序："寻常算术转换" ==================
 * 遇到 a op b（op 是 + - * / % < > == 等），C 按下面顺序决定用哪种类型算：
 *      第 1 步：整型提升，把 char / short / bool 都提升到 int
 *      第 2 步：看有没有浮点
 *              有 long double → 全部转 long double
 *              有 double      → 全部转 double
 *              有 float       → 全部转 float
 *      第 3 步：都是整数时，按宽度"就高"
 *              谁的类型宽，另一边就转成谁
 *      第 4 步：宽度相同时若一方无符号，另一方也转成无符号
 *
 * 一句话总结：
 *      先提升 → 再向浮点靠 → 再向宽的靠 → 同宽时向无符号靠。
 *
 * ================== 六、强制类型转换（显式转换） ==================
 * 语法：(目标类型) 表达式
 *      (double)5 / 2        把 5 变成 5.0，得到 2.5
 *      (int)3.99            得到 3
 *      (char)65             得到 'A'
 *
 * 强制转换的作用：
 *   1) 让整数除法变成浮点除法（最常用）
 *   2) 消除编译器的类型警告（比如 printf("%d", (int)sizeof(x))）
 *   3) 明确表达"我知道这里会丢精度，我确认要这么做"
 *
 * 注意事项：
 *   - 强制转换只是"换个类型解释同一批位"，不改变内存里的内容
 *   - 它不会做四舍五入，也不会检查范围
 *   - 指针之间的强制转换是另一回事（后面章节讲），别和数值转换混淆
 *   - 不要用强制转换去"消警告"，先想清楚数据是否真的安全
 *
 * ================== 七、三种转换场景的统一例子 ==================
 *      int    a = 5;
 *      double b = 2.0;
 *      a / b            → 隐式：a 转成 5.0，结果 2.5
 *      a / 2            → 整数除法，结果 2
 *      (double)a / 2    → 强制：结果 2.5
 *      int c = a / b;   → 隐式 + 赋值：先得 2.5，再被截断成 2
 *
 * ================== 八、常见坑 ==================
 *  1. -1 < 1u 为假          signed 被转成 unsigned
 *  2. 0u - 1 得到 4294967295 unsigned 回绕
 *  3. 5 / 9 == 0            两个整数相除
 *  4. (int)3.99 == 3        截断不是四舍五入
 *  5. char c = 300          超出范围，结果由实现决定
 *  6. 大整数转 float 丢精度 超过 2^24 的整数在 float 里存不全
 *  7. 2^53 + 1 转 double 丢精度  尾数只有 52 位
 *  8. printf 里类型不匹配    %d 打印 double 是未定义行为
 *  9. 以为 sizeof(a+b) 等于 sizeof(a)   整型提升让它变成 4
 * 10. 用强制转换掩盖警告，却没想清楚是否安全
 *
 * ================== 九、实践建议 ==================
 *  - 需要小数运算时，从一开始就用 double，不要用 int 算完再转
 *  - 整数相除想得到小数：写成 (double)a / b，并加括号保证优先级
 *  - signed / unsigned 不要混用，尤其不要用它做比较
 *  - 赋值时如果会发生窄化，主动写强制转换 + 注释，表明"我知道会丢数据"
 *  - 编译时打开 -Wall -Wextra，有条件再开 -Wconversion、-Wsign-compare
 *  - 记住优先级：转换发生在"运算之前"，不是运算之后
 */

#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

/* 用来演示"传参时的隐式转换"：形参是 float，实参可以是 int */
static void show_float(float f)
{
    printf("   传进来的 float = %.2f\n", (double)f);
}

/* 用来演示"返回值时的隐式转换"：函数声明返回 int，实际返回 double */
static int return_int(void)
{
    double d = 9.99;
    return d;               /* 9.99 被截断成 9 */
}

int main(void)
{
    /* ---------- 1. 整型提升：char + char 的结果是 int ---------- */
    char c1 = 'a', c2 = 'b';
    printf("1. sizeof(char)=%zu，sizeof(c1+c2)=%zu\n", sizeof(char), sizeof(c1 + c2));
    printf("   'a'+'b' 的值 = %d（不是 char，是 int）\n", c1 + c2);
    printf("   → 比 int 窄的类型参与运算前，先提升成 int\n");

    /* ---------- 2. 窄 → 宽：整数之间的自动转换，安全 ---------- */
    char  sc = 10;
    int   si = sc;              /* char → int，安全 */
    long  sl = si;              /* int → long，安全 */
    long long sll = sl;         /* long → long long，安全 */
    printf("2. char %d → int %d → long %ld → long long %lld（一个都没丢）\n",
           sc, si, sl, sll);
    printf("   小杯子往大杯子倒水，不会洒\n");

    /* ---------- 3. 整数 + 浮点：整数先变成浮点 ---------- */
    int    i = 10;
    double d = 2.5;
    printf("3. 10 + 2.5 = %.2f（int 先被转成 double 10.0 再相加）\n", i + d);
    printf("   sizeof(i + d) = %zu 字节（结果类型是 double）\n", sizeof(i + d));

    /* ---------- 4. 规则：谁宽听谁的 ---------- */
    char  a1 = 1;
    short a2 = 2;
    int   a3 = 3;
    long  a4 = 4L;
    printf("4. sizeof(char+short)   = %zu  → int 提升\n", sizeof(a1 + a2));
    printf("   sizeof(short+int)    = %zu  → int\n", sizeof(a2 + a3));
    printf("   sizeof(int+long)     = %zu  → long\n", sizeof(a3 + a4));
    printf("   sizeof(int+double)   = %zu  → double\n", sizeof(a3 + d));

    /* ---------- 5. 有符号 + 无符号：最著名的坑 ---------- */
    int          neg = -1;
    unsigned int one = 1;
    /* 编译器会隐式把 neg 转成 unsigned，等价于 4294967295 < 1 */
    printf("5. -1 < 1u ? %s\n", ((unsigned)neg < one) ? "真" : "假");
    printf("   因为 -1 被转成了 %u，当然比 1 大\n", (unsigned)neg);
    printf("   → signed 和 unsigned 混用是 bug 高发区\n");

    /* ---------- 6. unsigned 减法回绕 ---------- */
    unsigned int ua = 3, ub = 5;
    printf("6. 3u - 5u = %u（不是 -2，而是回绕成一个大正数）\n", ua - ub);
    printf("   → 需要负数结果时，就不要用 unsigned\n");

    /* ---------- 7. 宽 → 窄：赋值时的截断 ---------- */
    double src = 3.99;
    int    truncated = src;             /* 隐式：宽 → 窄，小数被砍掉 */
    printf("7. double 3.99 赋给 int → %d（截断，不是 4）\n", truncated);

    int  big = 300;
    char narrow = (char)big;            /* 显式写出来，表明"我知道会截断" */
    printf("   int 300 塞进 char → %d（只保留低 8 位：300-256=44）\n", narrow);

    /* ---------- 8. 整数除法 vs 浮点除法（转换的经典用途） ---------- */
    int p = 5, q = 2;
    printf("8. 5/2       = %d（两边都是 int，整数除法）\n", p / q);
    printf("   (double)5/2 = %.1f（强制把 5 变成 double）\n", (double)p / q);
    printf("   5/2.0     = %.1f（隐式：2.0 是 double，5 被提升）\n", p / 2.0);

    /* ---------- 9. 传参和返回值也会隐式转换 ---------- */
    printf("9. 函数形参是 float，传入 int 10：\n");
    show_float(10);                     /* int → float，隐式 */
    printf("   函数声明返回 int，实际 return 9.99 → %d（被截断）\n",
           return_int());

    /* ---------- 10. 精度丢失：大整数转 float / double ---------- */
    int    fine_for_float = 16777216;           /* 2^24，float 能精确表示的上限 */
    float  f_ok  = (float)fine_for_float;
    float  f_bad = (float)(fine_for_float + 1); /* 2^24+1，float 已经存不下 */
    printf("10. 2^24 转 float = %.0f\n", (double)f_ok);
    printf("    2^24+1 转 float = %.0f（低位被吃掉了）\n", (double)f_bad);

    long long big_int = 9007199254740993LL;     /* 2^53 + 1 */
    double    as_double = (double)big_int;
    printf("    2^53+1 转 double = %.0f\n", as_double);
    printf("    转回来还等于原来吗？%s（尾数只有 52 位）\n",
           ((long long)as_double == big_int) ? "是" : "不是");

    /* ---------- 11. 强制转换：明确表达"我要这个类型" ---------- */
    double pi = 3.14159;
    int    pi_int = (int)pi;
    printf("11. (int)3.14159 = %d（截断）\n", pi_int);
    printf("    四舍五入要用 round()，不是靠强制转换\n");

    /* ---------- 12. 布尔上下文也是一种"隐式转换" ---------- */
    /* if / while 的条件要求"真/假"，任何非 0 值都会被转成真（见 08 篇） */
    int  flag = 42;
    bool b   = flag;            /* int → bool，42 变成 1 */
    printf("12. int 42 转成 bool = %d（非 0 即真，归一化成 1）\n", b);

    /* ---------- 13. 运算规则总表（打印出来方便对照） ---------- */
    printf("13. 运算结果类型速查：\n");
    printf("    char  + char   → int        （整型提升）\n");
    printf("    short + int    → int\n");
    printf("    int   + long   → long       （谁宽听谁的）\n");
    printf("    int   + float  → float\n");
    printf("    int   + double → double\n");
    printf("    float + double → double\n");
    printf("    int   + unsigned → unsigned （同宽时向无符号靠）\n");
    printf("    → 顺序：提升 → 向浮点 → 向宽的 → 向无符号\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     gcc 10_变量间的运算规则.c -o conv_demo
 *     ./conv_demo
 *
 * 输出如下（数值与注释里的结论一一对应）：
 *
 * 1. sizeof(char)=1，sizeof(c1+c2)=4
 *    'a'+'b' 的值 = 195（不是 char，是 int）
 *    → 比 int 窄的类型参与运算前，先提升成 int
 * 2. char 10 → int 10 → long 10 → long long 10（一个都没丢）
 *    小杯子往大杯子倒水，不会洒
 * 3. 10 + 2.5 = 12.50（int 先被转成 double 10.0 再相加）
 *    sizeof(i + d) = 8 字节（结果类型是 double）
 * 4. sizeof(char+short)   = 4  → int 提升
 *    sizeof(short+int)    = 4  → int
 *    sizeof(int+long)     = 8  → long
 *    sizeof(int+double)   = 8  → double
 * 5. -1 < 1u ? 假
 *    因为 -1 被转成了 4294967295，当然比 1 大
 *    → signed 和 unsigned 混用是 bug 高发区
 * 6. 3u - 5u = 4294967294（不是 -2，而是回绕成一个大正数）
 *    → 需要负数结果时，就不要用 unsigned
 * 7. double 3.99 赋给 int → 3（截断，不是 4）
 *    int 300 塞进 char → 44（只保留低 8 位：300-256=44）
 * 8. 5/2       = 2（两边都是 int，整数除法）
 *    (double)5/2 = 2.5（强制把 5 变成 double）
 *    5/2.0     = 2.5（隐式：2.0 是 double，5 被提升）
 * 9. 函数形参是 float，传入 int 10：
 *    传进来的 float = 10.00
 *    函数声明返回 int，实际 return 9.99 → 9（被截断）
 * 10. 2^24 转 float = 16777216
 *     2^24+1 转 float = 16777216（低位被吃掉了）
 *     2^53+1 转 double = 9007199254740992
 *     转回来还等于原来吗？不是（尾数只有 52 位）
 * 11. (int)3.14159 = 3（截断）
 *     四舍五入要用 round()，不是靠强制转换
 * 12. int 42 转成 bool = 1（非 0 即真，归一化成 1）
 * 13. 运算结果类型速查：
 *     char  + char   → int        （整型提升）
 *     short + int    → int
 *     int   + long   → long       （谁宽听谁的）
 *     int   + float  → float
 *     int   + double → double
 *     float + double → double
 *     int   + unsigned → unsigned （同宽时向无符号靠）
 *     → 顺序：提升 → 向浮点 → 向宽的 → 向无符号
 *
 * 注意：第 10 条里的巨大数值和平台相关（int/long 宽度可能不同），
 *       换一台机器运行，个别数字可能变化，但"丢精度"这个结论不变。
 * ==========================================================================
 */
