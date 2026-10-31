/*
 * 05_整型.c —— 阶段1 / 第 01 课 / 第 5 个文件：整型（整数类型）
 *
 * 【学习顺序】本目录按文件名前缀的数字从小到大阅读：
 *     01_hello.c              第 1 篇：最小可运行的 C 程序
 *     02_printf_tutorial.c    第 2 篇：printf 占位符与格式化输出
 *     03_标识符.c             第 3 篇：标识符（名字）的命名规则
 *     04_变量.c               第 4 篇：变量的类型、初始化、作用域
 *     05_整型.c               第 5 篇：本篇，整数类型家族
 *     06_浮点类型.c           第 6 篇：float / double 与精度
 *     07_字符与字符串.c       第 7 篇：char、ASCII、字符串本质
 *     08_布尔类型.c           第 8 篇：bool、逻辑运算与短路
 *
 * 编译：gcc 05_整型.c -o int_demo
 * 运行：./int_demo
 *
 * 本篇只讲"整数"。浮点看 06，字符/字符串看 07。
 * 之所以拆开：整数和浮点的存储原理、运算规则、陷阱完全不同，
 * 混在一起讲容易记串。
 *
 * ====================== 一、为什么需要不同的数据类型 ======================
 * 内存只是一串字节，类型决定了三件事：
 *   1) 占几个字节      → 决定能表示多大的范围
 *   2) 怎么解释这些位  → 同样的 0/1，按整数读是 65，按浮点读就完全是另一个数
 *   3) 能做什么运算    → 整数能取模 %、能移位，浮点不能
 * 选错类型不会报错，但会得到错的结果（溢出、精度丢失），所以必须先搞清楚。
 *
 * C 的基本类型只有四大类：
 *      整型     char、short、int、long、long long（各有 signed / unsigned 版本）
 *      浮点型   float、double、long double
 *      布尔型   _Bool（C99），配合 <stdbool.h> 可用 bool / true / false
 *      空类型   void（"没有类型"，用于函数返回值/参数）
 * 在此基础上派生出指针、数组、结构体、联合、枚举——那是后面的章节。
 *
 * ============================ 二、整型家族 ============================
 * 名称               典型字节  取值范围（有符号）              常用场景
 * char                1        -128 ~ 127                    字符、原始字节
 * unsigned char       1        0 ~ 255                        字节、位操作
 * short               2        -32768 ~ 32767                省内存的小整数
 * unsigned short      2        0 ~ 65535
 * int                 4        -2147483648 ~ 2147483647      默认首选
 * unsigned int        4        0 ~ 4294967295                计数、位掩码
 * long                4或8     见下                           平台差异大，谨慎
 * unsigned long       4或8
 * long long           8        约 ±9.2e18                    需要 64 位时用它
 * unsigned long long  8        0 ~ 1.8e19
 *
 * ★ 二点一、signed / unsigned 这两个词到底什么意思 ★
 * 英文原义：
 *      signed    = "带符号的"，sign 就是"正负号"（+ 和 - 都是符号）
 *      unsigned  = "不带符号的"，只有大小，没有正负
 * 所以：
 *      signed int    → 有符号整数 → 能装负数
 *      unsigned int  → 无符号整数 → 只能装 0 和正数
 *
 * 落到二进制上，差别只在"最高位怎么用"：
 *      signed   最高位当"符号位"：0 表示非负，1 表示负数
 *               剩下的位表示数值，所以 32 位范围是 -2^31 ~ 2^31-1
 *      unsigned 最高位也当数值位，32 位全部用来表示大小
 *               范围是 0 ~ 2^32-1（正数上限翻倍）
 *
 * 以 8 位（char）为例，同一串 0/1 用两种类型读，结果完全不同：
 *      二进制      按 signed char 读    按 unsigned char 读
 *      0111 1111   127                  127
 *      1000 0000   -128                 128
 *      1111 1111   -1                   255
 *
 * 为什么负数那边能多表示一个（-128 而正数只到 127）？
 *      因为 0 被算在"非负"这一侧，占了正数的一个位置。
 *
 * 负数在计算机里怎么存：现代机器用"补码"（two's complement）
 *      -1 的 32 位全是 1（0xFFFFFFFF）。直观理解：
 *      -1 + 1 应该等于 0，而 1111...1111 + 1 会一路进位、最高位丢掉，正好变成 0。
 *      补码的好处：加减法不用为负数写特殊电路，CPU 只有一套加法器。
 *
 * 名字怎么记：
 *      signed   = 有符号 = 能装负数，代价是正数上限减半
 *      unsigned = 无符号 = 放弃负数，换来正数上限翻倍
 *      所以 unsigned 不等于"更安全"，它只是"换了一种取舍"。
 *      表示"个数、长度、下标、位掩码"这类不可能是负数的量时，更贴切；
 *      一旦需要参与减法（可能减出负数），用 unsigned 就会回绕成一个大正数，反而危险。
 *
 * 默认为有符号的：short / int / long / long long 不写前缀就是 signed。
 * 唯一例外是 char——它不写前缀时是否有符号"由编译器决定"（详见 07 篇）。
 *
 * ================== 三、平台差异：别硬编码"int 就是 4 字节" ==================
 * C 标准只规定"最小位数"，不规定具体字节数：
 *      标准保证：sizeof(char)==1，且 char ≤ short ≤ int ≤ long ≤ long long
 *      标准只要求 int 至少 16 位、long 至少 32 位、long long 至少 64 位。
 * 现实差异：
 *      平台              int   long   long long   指针
 *      Windows 64 位      4     4        8         8
 *      macOS/Linux 64 位  4     8        8         8
 * 所以"long 是 8 字节"这句话在 Windows 上是错的。
 * 要跨平台定宽，用 <stdint.h> 的固定宽度类型：
 *      int8_t / int16_t / int32_t / int64_t
 *      uint8_t / uint16_t / uint32_t / uint64_t
 * 要存"长度、大小、下标"，用 size_t（sizeof 的返回类型，无符号）。
 *
 * ======================== 四、整数字面量的写法 ========================
 *      42        十进制
 *      052       八进制（前导 0！不是"五十二"，是 42）——极易看错，建议避免
 *      0x2A      十六进制
 *      0b1010    二进制（C23 进标准，GCC/Clang 早就作为扩展支持）
 * 后缀（可以组合）：
 *      42u       unsigned       42U
 *      42l       long           42L
 *      42ul      unsigned long  42UL
 *      42ll      long long      42LL
 *      42ull     unsigned long long
 * 字符字面量 'A' 其实也是整数（值为 65），见 07 篇。
 *
 * ====================== 五、整型提升与隐式转换 ======================
 * 运算前，比 int 窄的类型（char、short）会先被"提升"成 int 再算：
 *      'A' + 1  →  int 的 66，不是 char
 * 不同宽度混合运算时，窄的向宽的靠拢：
 *      char → short → int → long → long long
 * 宽转窄（如 long → int → char）会截断高位，可能丢数据。
 * signed 与 unsigned 混合时，signed 会被转成 unsigned——这是 bug 高发区：
 *      int a = -1;
 *      unsigned b = 1;
 *      a < b   →  实际比较 4294967295 < 1，结果为假
 *
 * ========================== 六、整数溢出 ==========================
 *   signed 溢出   →  未定义行为（UB）。可能得到负数、可能被优化掉，不可依赖。
 *   unsigned 溢出 →  回绕（模 2^N），标准明确规定，可以放心依赖。
 *       UINT_MAX + 1 == 0
 *       0u - 1 == UINT_MAX
 * 这也是为什么"位掩码、哈希"这类允许溢出的计算要用 unsigned。
 *
 * ========================== 七、整数除法 ==========================
 * 两个整数相除，结果还是整数，小数部分直接丢掉（向零截断）：
 *      5 / 2   == 2
 *      -5 / 2  == -2（C99 起规定向零截断，不是向下取整 -3）
 *
 * ★ 极端但最常见的例子：被除数比除数小
 *      5 / 9   == 0      ← 不是 0.5555，而是 0！
 *      1 / 2   == 0
 *      9 / 10  == 0
 * 因为整数除法只保留"商"，5 里面装不下一个 9，商就是 0，余数是 5。
 * 这正是 5 / 9 常被拿来做"百分比 / 比例"时出错的原因——结果是 0，乘什么都没用。
 * 经典事故：华氏转摄氏公式写成 5 / 9 * (f - 32)，永远是 0。
 *         正确写法：5.0 / 9.0 * (f - 32)   或   (double)5 / 9 * (f - 32)
 *
 * 取余 % 只对整数有意义（浮点要用 fmod）：
 *      5 % 9  == 5（除不尽，余数就是它自己）
 *      5 % 2  == 1
 *      余数的符号跟随被除数：-5 % 2 == -1
 * 想得到 2.5，必须让至少一个操作数变成浮点：(double)5 / 2
 * 记住这条规则：只要 / 两边都是整数，小数部分就一定会被丢掉，
 * 与"结果本来有没有小数"无关。
 *
 * ============================ 八、常见坑 ============================
 *  1. 用 %d 打印 long / long long  →  未定义行为（该用 %ld / %lld）
 *  2. 整数除法当小数用  1/2 == 0
 *  3. signed 与 unsigned 混合比较
 *  4. 把 200 存进 signed char，得到 -56
 *  5. 无符号数做减法：0 - 1 变成 4294967295
 *  6. 硬编码"int 是 4 字节"，换平台（16 位单片机）就错
 *  7. 把 052 当成五十二（其实是 42）
 *  8. sizeof 的结果是 size_t，要用 %zu 打印
 *
 * ============================ 九、选型建议 ============================
 *      默认用 int；
 *      要跨平台定宽，用 int32_t / uint64_t 这些；
 *      计数、长度、下标用 size_t 或 unsigned；
 *      确知范围很小且内存紧张，才用 short / char；
 *      需要 64 位整数，用 long long 或 int64_t（别只写 long）。
 */

#include <stdio.h>
#include <limits.h>     /* 整数范围宏：INT_MAX、CHAR_MIN、ULLONG_MAX ... */
#include <stdint.h>     /* 固定宽度类型：int8_t、int32_t、uint64_t ... */
#include <stddef.h>     /* size_t */

int main(void)
{
    /* ---------- 1. 各整型占多大：全部用 sizeof 实测 ---------- */
    /* sizeof 返回 size_t（无符号），printf 用 %zu */
    printf("1. char=%zu  short=%zu  int=%zu  long=%zu  long long=%zu\n",
           sizeof(char), sizeof(short), sizeof(int),
           sizeof(long), sizeof(long long));
    printf("   指针 void*=%zu  size_t=%zu（跟着平台变）\n",
           sizeof(void *), sizeof(size_t));

    /* ---------- 2. 取值范围：读 <limits.h> 的宏，别背 ---------- */
    printf("2. char      %d ~ %d\n", CHAR_MIN, CHAR_MAX);
    printf("   short     %d ~ %d\n", SHRT_MIN, SHRT_MAX);
    printf("   int       %d ~ %d\n", INT_MIN, INT_MAX);
    printf("   long      %ld ~ %ld\n", LONG_MIN, LONG_MAX);
    printf("   long long %lld ~ %lld\n", LLONG_MIN, LLONG_MAX);
    printf("   unsigned  0 ~ %u\n", UINT_MAX);
    printf("   ullong    0 ~ %llu\n", ULLONG_MAX);

    /* ---------- 3. 同一串二进制位，signed / unsigned 两种读法 ---------- */
    /* signed  = 带正负号，最高位当符号位；unsigned = 不带符号，全部位都是数值 */
    unsigned char bits8  = 0xFF;          /* 8 个位全是 1 */
    unsigned int  bits32 = 0xFFFFFFFFu;   /* 32 个位全是 1 */
    printf("3. 0xFF 按 unsigned char 读 = %u\n", bits8);
    printf("   0xFF 按 signed char 读   = %d（最高位 1 被当成负号）\n",
           (signed char)bits8);
    printf("   0xFFFFFFFF 按 unsigned 读 = %u\n", bits32);
    printf("   0xFFFFFFFF 按 int 读      = %d（补码里它就是 -1）\n",
           (int)bits32);
    printf("   结论：位是同一批位，类型决定怎么解释它\n");

    /* ---------- 4. 字面量的进制与后缀 ---------- */
    int dec = 42;
    int oct = 052;          /* 前导 0：八进制，等于十进制 42 */
    int hex = 0x2A;         /* 0x 开头：十六进制，也等于 42 */
    printf("4. 十进制 42=%d，八进制 052=%d，十六进制 0x2A=%d\n", dec, oct, hex);
    printf("   看到 0052 千万别当成“五十二”\n");

    unsigned int  u  = 42u;         /* u 后缀：无符号 */
    long          L  = 42L;         /* L 后缀：long */
    unsigned long UL = 42UL;        /* 可以组合 */
    long long     LL = 42LL;        /* LL 后缀：long long */
    printf("   后缀写法的值：%u %ld %lu %lld\n", u, L, UL, LL);

    /* ---------- 5. 整型提升：窄类型运算前先变 int ---------- */
    char c1 = 'a', c2 = 'b';
    printf("5. sizeof(c1)=%zu  sizeof(c1+c2)=%zu（运算时提升成 int）\n",
           sizeof(c1), sizeof(c1 + c2));

    /* ---------- 6. 溢出：unsigned 回绕可靠，signed 溢出是 UB ---------- */
    unsigned int umax = UINT_MAX;
    printf("6. UINT_MAX+1 = %u（无符号回绕，标准保证）\n", umax + 1);
    printf("   0u-1        = %u（往回绕到最大值）\n", 0u - 1);
    /* 注意：int 溢出不写出来演示，因为那是未定义行为，结果不可信 */

    /* ---------- 7. 整数除法与取余 ---------- */
    printf("7. 5/2=%d   5%%2=%d\n", 5 / 2, 5 % 2);
    printf("   -5/2=%d  -5%%2=%d（C99 起向零截断）\n", -5 / 2, -5 % 2);

    /* 被除数比除数小时：商直接是 0，这就是 5/9 == 0 的原因 */
    printf("   5/9=%d   5%%9=%d（装不下一个 9，商 0 余 5）\n", 5 / 9, 5 % 9);
    printf("   1/2=%d   9/10=%d（同样的道理，全都变成 0）\n", 1 / 2, 9 / 10);

    /* 经典事故：华氏转摄氏，5/9 先算成 0，整个式子永远是 0 */
    int fahrenheit = 212;                       /* 华氏 212 度 = 摄氏 100 度 */
    int wrong = 5 / 9 * (fahrenheit - 32);      /* ✗ 5/9 先算 = 0 */
    double right = 5.0 / 9.0 * (fahrenheit - 32);  /* ✓ 先变成浮点除法 */
    printf("   华氏 %d 度：5/9*(f-32) = %d（错）\n", fahrenheit, wrong);
    printf("              5.0/9.0*(f-32) = %.1f（对）\n", right);

    printf("   (double)5/2=%.1f（想得小数必须转浮点）\n", (double)5 / 2);
    printf("   (double)5/9=%.4f（把 0 变成真正的比例）\n", (double)5 / 9);

    /* ---------- 8. 定宽类型：跨平台的正确姿势 ---------- */
    int32_t  exact32 = 100;
    uint64_t exact64 = 100ULL;
    printf("8. int32_t=%zu 字节，uint64_t=%zu 字节（大小固定，跨平台一致）\n",
           sizeof(exact32), sizeof(exact64));

    /* ---------- 9. 溢出检测的常见写法 ---------- */
    /* 相加前先判断：a > INT_MAX - b 就说明会溢出（这里只演示思路） */
    int a = 2000000000, b = 2000000000;
    if (a > INT_MAX - b) {
        printf("9. a+b 会溢出，已被提前拦住（a=%d, b=%d）\n", a, b);
    } else {
        printf("9. a+b = %d\n", a + b);
    }

    return 0;
}
