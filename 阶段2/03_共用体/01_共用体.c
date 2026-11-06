/*
 * 01_共用体.c —— 阶段2 / 第 03 课 / 第 01 个文件：共用体（union）从定义到进阶
 *
 * 编译：gcc -std=c99 -Wall -Wextra 01_共用体.c -o bin/01_共用体
 * 运行：./bin/01_共用体
 *
 * 共用体和结构体的语法几乎一样，只差一个关键字，语义却完全相反：
 *
 *      struct  成员的地址**依次排开**，大小 ≈ 成员之和（加填充）
 *      union   所有成员的地址**都是 0**，大小 = 最大的那个成员
 *
 * 一句话：结构体是"和"，共用体是"或" —— 同一时刻只用其中一个。
 *
 * 本篇分三段：
 *      基础（一~四）：怎么定义、怎么初始化、内存里长什么样
 *      使用（五~七）：类型双关、省内存、看字节表示
 *      进阶（八~十）：tagged union、对齐技巧、位域视图
 *
 * ====================== 一、定义方式 ======================
 * 语法和 struct 一模一样，把关键字换掉即可：
 *
 *      union Number {
 *          int          i;
 *          float        f;
 *          unsigned char bytes[4];
 *      };
 *
 * 四种写法和结构体完全对应：
 *
 * (1) **先声明，再定义变量**
 *          union Number n;
 *
 * (2) **声明同时定义变量**
 *          union Number { int i; float f; } n1, n2;
 *
 * (3) **typedef 起别名**（工程里最常用）
 *          typedef union Number { int i; float f; } Number;
 *          Number n;
 *
 * (4) **匿名共用体 + typedef**
 *          typedef union { int i; float f; } Number;
 *      同样没有标签，无法自引用（本来也不需要）。
 *
 * ★ 和结构体一样，`union` 关键字不能省（除非 typedef），声明末尾必须有分号。
 *
 * ====================== 二、大小规则：最大成员 + 对齐 ======================
 *      sizeof(union) = 最大的成员大小，再向上取整到**最大对齐**的倍数
 *
 *      union U1 { char c; int i; };              // 4：int 最大且对齐 4
 *      union U2 { char c; double d; };           // 8
 *      union U3 { char c; char buf[100]; };      // 100
 *      union U4 { int i; unsigned char b[4]; };  // 4
 *
 * 对比结构体：
 *      struct S { int i; unsigned char b[4]; };  // 8（4 + 4，各自占位）
 *      union  U { int i; unsigned char b[4]; };  // 4（共用一块）
 *
 * ★ 共用体的对齐 = **成员里最大的对齐**，不是最大成员的字节数。
 *   所以 `union { char c; short s; }` 是 2 字节，能被 2 整除。
 *
 * ★ 含 long double 或 SIMD 类型时，对齐可能比"最大字节数"更严格：
 *      union AlignBox { long double ld; char buf[64]; };   // 大小 64，对齐 16
 *   可以用"探针结构体 + offsetof"量出真实对齐（本文第八节）。
 *
 * ====================== 三、初始化：只能有一个"有效"成员 ======================
 * 共用体同一时刻只有一个成员有意义，所以初始化也只能给一个：
 *
 * (1) **初始化第一个成员**（最常见）
 *          union Number n = { 42 };              // 初始化的是 i
 *
 * (2) **指定初始化器**（C99，指定任意一个成员）
 *          union Number n = { .f = 3.14f };      // 初始化的是 f
 *
 * (3) **全部清零**
 *          union Number n = { 0 };
 *
 * ★ 不能像结构体那样一次给多个成员赋值：
 *          union Number n = { .i = 1, .f = 2.0f };   // ✗ 编译错误
 *   为什么？因为它们共用一块内存，同时给两个值逻辑上就矛盾。
 *
 * ★ 未初始化的共用体，成员内容同样是**不确定值**，和局部结构体一样。
 *
 * ====================== 四、内存布局：所有成员偏移都是 0 ======================
 *      union Number n;
 *
 *      offsetof(union Number, i)     == 0
 *      offsetof(union Number, f)     == 0
 *      offsetof(union Number, bytes) == 0
 *
 * 三个成员**起始地址完全相同**，只是"看法"不同：
 *
 *      n.i = 0x3f800000  之后：
 *          n.i      按 int 看     → 1065353216
 *          n.f      按 float 看   → 1.0
 *          n.bytes  按 4 个字节看 → 00 00 80 3f（小端）
 *
 * 写一个、读另一个，读到的不是"上次写的那个值"，而是**同一串字节的另一种解释**。
 * 这正是共用体的价值，也是它的风险来源。
 *
 * ★ 共用体可以整体赋值（同类型之间），规则和结构体一样：逐成员/逐字节拷贝。
 *   但不能用 `==` 比较 —— 原因和结构体相同（填充、表示不确定）。
 *
 * ====================== 五、类型双关（type punning）：能做什么，别依赖什么 ======================
 * "写 A 成员、读 B 成员"这种做法叫类型双关。C 标准允许（C11 明确说明这是
 * 查看对象表示的正当方式），但结果**依赖两件事**：
 *
 *      1. 类型的内部表示（float 是不是 IEEE 754？int 几位？）
 *      2. 字节序（小端还是大端）
 *
 * 所以：
 *  - 同一台机器上，用来看"1.0f 的位模式是 0x3f800000"是可靠的
 *  - 用来做**跨平台的数据解析**是不可靠的
 *  - 严格来说读非活跃成员得到的是"实现定义"的值，某些类型还可能是陷阱表示
 *    （trap representation），读它甚至可能触发异常
 *
 * ★ 工程上的替代方案，按可靠性排序：
 *      最稳   memcpy 到目标类型的变量（标准明确允许，编译器会优化掉）
 *      可用   共用体双关（主流编译器支持，C 标准也认可）
 *      最差   指针强转 `*(float *)&i`（违反严格别名规则，优化后可能出错）
 *
 * 也就是说：**pointer cast 是最不该用的写法**，共用体反而比它安全。
 *
 * ====================== 六、用途一：省内存（同一时刻只用一个字段） ======================
 * 最朴素的用途。当几个字段**互斥**（一次只会用其中一个）时，让它们共享内存：
 *
 *      union Value {
 *          int    i;
 *          double d;
 *          void  *p;
 *      };                       // sizeof = 8（最大成员 + 对齐）
 *
 *      struct ValueWithType {   // 8 + 4 → 16（含填充）
 *          int  type;
 *          union Value v;
 *      };
 *
 * 典型场景：内存紧张的嵌入式、协议报文里"同位置不同含义"的字段、
 * 一个通用容器要能装多种类型。
 *
 * ★ 省内存的代价是**必须自己记住当前是哪个成员有效**。
 *   忘记记 → 读到的是别的类型的字节 → 静默的错误值（比崩溃更难查）。
 *   这就引出了第八节的 tagged union。
 *
 * ====================== 七、用途二：查看字节表示 ======================
 * (1) **看浮点的位模式**
 *          union FloatBits { float f; uint32_t u; };
 *          union FloatBits fb = { .f = 1.0f };
 *          printf("%08x", fb.u);            // IEEE 754 单精度：3f800000
 *
 * (2) **判断字节序**（大小端）
 *          union Endian { uint16_t u; unsigned char b[2]; };
 *          union Endian e = { .u = 0x0102 };
 *          e.b[0] == 0x02  →  小端（低位字节放低地址）
 *          e.b[0] == 0x01  →  大端
 *      这是最经典的字节序检测写法，比指针强转干净。
 *
 * (3) **按字节遍历一个对象**（比如算校验和）
 *          union Number n;
 *          for (int i = 0; i < 4; i++) sum += n.bytes[i];
 *      注意这只对"没有指针成员"的类型安全；含指针时字节内容无意义。
 *
 * ★ 网络序 / 磁盘格式永远规定"大端"或"小端"，别把本机字节序直接写进文件。
 *   要可移植就用 htons/htonl 这类转换函数，或者逐字节序列化。
 *
 * ====================== 八、进阶一：tagged union（判别式联合） ======================
 * 共用体最大的坑是"不知道当前哪个成员有效"。解决办法是**加一个标签**：
 *
 *      typedef enum { VAL_INT, VAL_DOUBLE, VAL_STR } ValueKind;
 *
 *      typedef struct {
 *          ValueKind kind;          // 标签：现在有效的是哪个成员
 *          union {
 *              int         i;
 *              double      d;
 *              const char *s;
 *          } as;                    // 名字随便取，as 是常见写法
 *      } Value;
 *
 * 用法固定三步：
 *      写入：先设 kind，再写对应成员
 *      读取：先看 kind，再 switch 到对应成员
 *
 *      Value v;
 *      v.kind  = VAL_INT;
 *      v.as.i  = 42;
 *
 *      switch (v.kind) {
 *      case VAL_INT:    printf("%d", v.as.i); break;
 *      case VAL_DOUBLE: printf("%g", v.as.d); break;
 *      case VAL_STR:    printf("%s", v.as.s); break;
 *      }
 *
 * 这就是 C 里实现"变体类型 / 动态类型"的标准做法：
 *  - 比 `void *` + 强转安全：类型信息跟着数据走，编译器能帮你检查 switch 覆盖
 *  - 比"给每种类型都开一个字段"省内存
 *  - 缺点：每次读写都要判 kind；kind 和成员可能被写歪（需要纪律或封装成函数）
 *
 * ★ 封装建议：别让调用方直接碰 kind，提供 `value_int()` / `value_double()` /
 *   `value_print()` 这类函数，把"设标签 + 写成员"绑在一起，就不会忘。
 *
 * ★ 常见变体：把 `as` 那个 union 直接写成**匿名成员**（C11）：
 *          struct { ValueKind kind; union { int i; double d; }; };
 *   访问变成 `v.i`、`v.d`。C99 不支持匿名成员，本仓库统一用命名的 `as`。
 *
 * ====================== 九、进阶二：用共用体保证对齐 / 做内存池 ======================
 * 要让一块字节缓冲区"能放下任何类型"，就得保证它的对齐足够严格。
 * 单靠 `char buf[N]` 只有 1 字节对齐，塞 double 进去可能未对齐访问。
 *
 * 标准做法：让共用体和最"挑剔"的类型对齐：
 *
 *      union AlignBox {
 *          long double ld;      // 本平台对齐最严的基本类型之一（8 或 16，依平台）
 *          void       *p;       // 对齐 8（64 位）
 *          char        buf[64]; // 64 字节容量
 *      };
 *
 * 这样 `AlignBox` 的大小是 64，对齐取成员里最严的那个（本机量出来是 8，
 * Linux/x86-64 上常见 16 —— 所以**必须用探针量，别照抄别处的数字**）。
 * 用它的 `buf` 当存储时，起始地址天然满足任何基本类型的要求。
 *
 * 用法（简易内存池 / 对象池）：
 *      union AlignBox pool[16];
 *      void *slot = pool[i].buf;      // 起点对齐，随便放什么类型
 *
 * ★ C11 提供了 `max_align_t`（<stddef.h>）专门表示"最大对齐类型"，
 *   C99 里没有，所以老代码用 long double / void * 凑。
 *
 * ★ 这只是"保证对齐"，不是"管理生命周期"：放进去的对象什么时候构造、析构，
 *   仍然要自己管。C 没有构造/析构，但指针成员该 free 还是要 free。
 *
 * ====================== 十、进阶三：位域 + 共用体，一个寄存器两种看法 ======================
 * 嵌入式里常把"整寄存器值"和"各个位域"叠在一起：
 *
 *      union Reg {
 *          uint32_t raw;                 // 整体看
 *          struct {
 *              uint32_t enable : 1;
 *              uint32_t mode   : 3;
 *              uint32_t        : 4;      // 保留位，无名位域
 *              uint32_t value  : 24;
 *          } bits;                       // 按位看
 *      };
 *
 *      r.raw = 0x1234;
 *      r.bits.mode                          // 取某几位
 *      r.bits.enable = 1;                   // 改某一位
 *
 * 好处：一次赋值设置整个寄存器，又能按字段读写，不用手写移位掩码。
 *
 * ★ 三个必须知道的限制：
 *  1. **位域的布局是实现定义的**：谁在低位、怎么跨存储单元，标准和编译器可能不同。
 *     同一份代码在 x86 和某些 DSP 上结果可能不一样。
 *  2. 位域**不能取地址**（`&r.bits.mode` 非法），也不能 offsetof。
 *  3. 无名的 `uint32_t : 4;` 是"填充位"，用来跳过保留字段，不是成员。
 *
 * 所以：**只在"编译器/平台固定"的场合用**（驱动、寄存器映射），
 * 处理外部协议时老老实实用移位和掩码：
 *      (raw >> 1) & 0x7        // 取 mode
 *      raw |= (1u << 0)        // 置 enable
 *
 * ====================== 十一、常见坑 ======================
 *  1. 以为 union 能同时存多个值        它只有一块内存，写一个就覆盖另一个
 *  2. 读没写入过的成员                读到的是别的字节，静默错误
 *  3. 忘了记录"当前哪个成员有效"       必须配标签（tagged union）
 *  4. 用 union 解析网络协议            字节序 + 位域布局都不保证，跨平台必炸
 *  5. 用指针强转代替 union             `*(float *)&i` 违反严格别名，-O2 可能出错
 *  6. 对 union 用 ==                   编译不过；逐成员比也没有意义
 *  7. 用 union 存指针后按字节处理       指针的字节表示没有意义
 *  8. 把位域 union 当成可移植布局       位序依实现，换编译器结果就变
 *  9. 在 union 里放"需要初始化"的成员   C 里没有构造函数，全靠手动
 * 10. 用 union 做"类型自动切换"         C 不会帮你记类型，必须自己管
 *
 * 补充两条和结构体一致的点：
 *  - union 可以**整体赋值**（同类型），但不能比较
 *  - union 里的成员**不能是柔性数组**，也不能是函数（类型本身非法）
 *
 * ====================== 十二、实践建议 ======================
 *  - 需要"同一时刻只有一个字段"才用 union；否则用 struct
 *  - 用 union 就一定配 enum 标签，别让调用方猜
 *  - 把"设 kind + 写成员"封装成函数，杜绝忘记同步
 *  - 想看字节表示：优先 `memcpy` 或 union；**别用指针强转**
 *  - 跨平台数据交换一律逐字段序列化，别把内存里的 union 直接写文件/发网络
 *  - 位域 union 只用于平台固定的寄存器/协议结构，并写注释说明编译器前提
 *  - 需要缓冲区对齐时，用包含 long double / void * 的 union 当"对齐盒子"
 *  - 调试 union 相关 bug：先确认"现在哪个成员有效"，再看字节
 */

#include <stdio.h>
#include <stddef.h>      /* offsetof */
#include <string.h>      /* memset / memcpy */
#include <stdint.h>      /* uint16_t / uint32_t（C99） */

/* ====================== 基础：定义与大小 ====================== */

typedef union Number {
    int           i;
    float         f;
    unsigned char bytes[4];
} Number;

typedef struct NumberStruct {       /* 对照组：结构体 */
    int           i;
    float         f;
    unsigned char bytes[4];
} NumberStruct;

typedef union Mixed {
    char  c;
    int   i;
} Mixed;

typedef union Small {
    char  c;
    short s;
} Small;

/* ====================== 类型双关与字节序 ====================== */

typedef union FloatBits {
    float    f;
    uint32_t u;
} FloatBits;

typedef union Endian {
    uint16_t      u;
    unsigned char b[2];
} Endian;

/* ====================== 进阶：对齐盒子 ====================== */

typedef union AlignBox {
    long double  ld;         /* 对齐要求最高的成员之一 */
    void        *p;
    char         buf[64];    /* 容量 */
} AlignBox;

typedef struct AlignProbe {  /* 用 offsetof 量出 AlignBox 的真实对齐 */
    char     c;
    AlignBox box;
} AlignProbe;

/* ====================== 进阶：位域视图 ====================== */

typedef union Reg {
    uint32_t raw;            /* 整体看 */
    struct {
        uint32_t enable : 1;
        uint32_t mode   : 3;
        uint32_t        : 4; /* 保留位，无名位域 */
        uint32_t value  : 24;
    } bits;                  /* 按位看 */
} Reg;

/* ====================== 进阶：tagged union ====================== */

typedef enum ValueKind {
    VAL_INT,
    VAL_DOUBLE,
    VAL_STR
} ValueKind;

typedef struct Value {
    ValueKind kind;          /* 标签：现在有效的是哪个成员 */
    union {
        int         i;
        double      d;
        const char *s;
    } as;
} Value;

/* 把"设标签 + 写成员"绑在一起，调用方就不会忘 */
static Value value_int(int i)
{
    Value v;
    v.kind = VAL_INT;
    v.as.i = i;
    return v;
}

static Value value_double(double d)
{
    Value v;
    v.kind = VAL_DOUBLE;
    v.as.d = d;
    return v;
}

static Value value_str(const char *s)
{
    Value v;
    v.kind = VAL_STR;
    v.as.s = s;
    return v;
}

/* 读之前必须先看标签 */
static void value_print(const Value *v)
{
    switch (v->kind) {
    case VAL_INT:
        printf("%d（int）", v->as.i);
        break;
    case VAL_DOUBLE:
        printf("%g（double）", v->as.d);
        break;
    case VAL_STR:
        printf("%s（const char *）", v->as.s);
        break;
    default:
        printf("未知标签 %d", (int)v->kind);
        break;
    }
}

int main(void)
{
    /* ---------- 1. 定义与大小：union vs struct ---------- */
    printf("1. 定义与大小\n");
    printf("   sizeof(Number)       = %zu（最大成员 4 字节）\n", sizeof(Number));
    printf("   sizeof(NumberStruct) = %zu（成员各占各的，再加填充）\n",
           sizeof(NumberStruct));
    printf("   同样的成员，union 省了 %zu 字节\n",
           sizeof(NumberStruct) - sizeof(Number));

    printf("   union Mixed { char c; int i; }  → sizeof = %zu（对齐到 4）\n",
           sizeof(Mixed));
    printf("   union Small { char c; short s; } → sizeof = %zu（对齐到 2）\n",
           sizeof(Small));
    printf("   规律：大小 = 最大成员，向上取整到最大对齐的倍数\n");

    /* ---------- 2. 初始化 ---------- */
    printf("2. 初始化：同一时刻只有一个有效成员\n");
    Number n1 = { 42 };                       /* ① 初始化第一个成员 i */
    Number n2 = { .f = 3.14f };               /* ② C99 指定初始化器 */
    Number n3 = { 0 };                        /* ③ 全部清零 */

    printf("   ① { 42 }          → i = %d\n", n1.i);
    printf("   ② { .f = 3.14f }  → f = %.2f\n", (double)n2.f);
    printf("   ③ { 0 }           → i = %d, f = %g\n", n3.i, (double)n3.f);
    printf("   { .i = 1, .f = 2.0f } 编译不过：两个成员共用一块内存，不能同时给值\n");

    /* ---------- 3. 所有成员偏移都是 0 ---------- */
    printf("3. 内存布局：三个成员起始地址相同\n");
    printf("   offsetof(Number, i)     = %zu\n", offsetof(Number, i));
    printf("   offsetof(Number, f)     = %zu\n", offsetof(Number, f));
    printf("   offsetof(Number, bytes) = %zu\n", offsetof(Number, bytes));
    printf("   sizeof(Number) = %zu，三个成员都从同一块内存的 0 偏移开始\n",
           sizeof(Number));

    Number n;
    n.i = 0x3f800000;                         /* 写入整数的位模式 */
    printf("   写 n.i = 0x3f800000 之后：\n");
    printf("      n.i     = %d（按 int 看）\n", n.i);
    printf("      n.f     = %g（按 float 看，同一串字节的另一种解释）\n",
           (double)n.f);
    printf("      n.bytes = %02x %02x %02x %02x（小端平台：低字节在低地址）\n",
           n.bytes[0], n.bytes[1], n.bytes[2], n.bytes[3]);

    /* ---------- 4. 清零后读另一个成员是安全的 ---------- */
    printf("4. 未写入的成员：内容不确定，别读\n");
    Number z;
    memset(&z, 0, sizeof z);                  /* 全零的位模式是确定的 */
    printf("   memset 全 0 后读 z.f = %g（0 的位模式就是 0.0）\n", (double)z.f);
    printf("   但没清零的 union 直接读另一个成员，得到的是垃圾字节\n");
    printf("   指针强转 *(float *)&i 更糟：违反严格别名规则，-O2 下可能出错\n");

    /* ---------- 5. 类型双关：浮点位模式 ---------- */
    printf("5. 类型双关：float 的位模式\n");
    FloatBits fb;
    fb.f = 1.0f;
    printf("   1.0f   → 0x%08x\n", (unsigned)fb.u);
    fb.f = 0.5f;
    printf("   0.5f   → 0x%08x\n", (unsigned)fb.u);
    fb.f = -2.0f;
    printf("   -2.0f  → 0x%08x\n", (unsigned)fb.u);
    printf("   IEEE 754 单精度：符号 1 位 + 阶码 8 位 + 尾数 23 位\n");
    printf("   这依赖浮点表示，不是所有平台都这样\n");

    /* ---------- 6. 字节序检测 ---------- */
    printf("6. 字节序检测\n");
    Endian e;
    e.u = 0x0102;
    printf("   0x0102 的两个字节：b[0] = 0x%02x, b[1] = 0x%02x\n", e.b[0], e.b[1]);
    printf("   判定：%s\n",
           e.b[0] == 0x02 ? "小端（低位字节在低地址）" : "大端（高位字节在低地址）");
    printf("   本行结果依平台；x86 / ARM64 常见是小端\n");

    /* ---------- 7. 整体赋值 ---------- */
    printf("7. union 可以整体赋值，不能比较\n");
    Number src = { .f = 2.5f };
    Number dst;
    dst = src;                                /* 同类型整体赋值：合法 */
    printf("   dst = src 后 dst.f = %g（整块字节被拷贝）\n", (double)dst.f);
    printf("   if (dst == src) 编译不过：union 没有 == 运算\n");

    /* ---------- 8. 进阶：tagged union ---------- */
    printf("8. tagged union：标签 + 共用体 = 变体类型\n");
    Value vals[3];
    vals[0] = value_int(42);
    vals[1] = value_double(3.5);
    vals[2] = value_str("hello");

    for (int i = 0; i < 3; i++) {
        printf("   vals[%d] = ", i);
        value_print(&vals[i]);
        printf("\n");
    }
    printf("   sizeof(Value) = %zu（enum 4 + 对齐 + union 8）\n", sizeof(Value));
    printf("   读之前先看 kind，写的时候先设 kind —— 这两步必须成对\n");
    printf("   封装成 value_int/value_print 之后，调用方就不容易忘\n");

    /* ---------- 9. 进阶：对齐盒子 ---------- */
    printf("9. 对齐盒子：让缓冲区满足最严格的对齐\n");
    printf("   sizeof(AlignBox) = %zu（buf 决定容量，对齐由 long double 决定）\n",
           sizeof(AlignBox));
    printf("   AlignBox 的对齐 = %zu（用探针结构体的 offsetof 量出来）\n",
           offsetof(AlignProbe, box));
    printf("   long double 的对齐随平台变（8 或 16），所以一定要量，别硬编码\n");
    printf("   char buf[64] 单独用只有 1 字节对齐；套进 union 后起点天然对齐\n");
    printf("   这样 pool[i].buf 里放 double、指针都不会未对齐访问\n");

    AlignBox pool[2];
    memset(&pool, 0, sizeof pool);
    double *slot = (double *)pool[0].buf;     /* 起点对齐，可以安全放 double */
    *slot = 2.5;
    printf("   往 pool[0].buf 里放一个 double：%g\n", *slot);

    /* ---------- 10. 进阶：位域视图 ---------- */
    printf("10. 位域 + union：一个寄存器两种看法\n");
    Reg r;
    r.raw = 0x00000000u;
    r.bits.enable = 1;                        /* 改第 0 位 */
    r.bits.mode   = 5;                        /* 改第 1~3 位 */
    r.bits.value  = 0x1234;                   /* 改第 8~31 位 */
    printf("   设 enable=1, mode=5, value=0x1234 后：\n");
    printf("      r.raw        = 0x%08x\n", (unsigned)r.raw);
    printf("      r.bits.mode  = %u\n", (unsigned)r.bits.mode);
    printf("      r.bits.value = 0x%x\n", (unsigned)r.bits.value);
    printf("   sizeof(Reg) = %zu（4 字节，位域正好铺满一个 uint32_t）\n",
           sizeof(Reg));
    printf("   位域布局依实现：别用这种方式解析外部协议，用移位和掩码\n");
    printf("   等价的手写方式：raw |= (1u << 0); raw |= (5u << 1); raw |= (0x1234u << 8)\n");

    /* ---------- 11. 小结 ---------- */
    printf("11. 小结：\n");
    printf("    结构体是“和”，共用体是“或”：所有成员共用一块内存\n");
    printf("    定义语法和 struct 只差一个关键字；大小 = 最大成员 + 对齐取整\n");
    printf("    初始化只能给一个成员；所有成员 offsetof 都是 0\n");
    printf("    写一个读另一个 = 类型双关，依赖表示和字节序，别用于跨平台解析\n");
    printf("    省内存的前提是\"互斥\"，配一个 enum 标签才安全（tagged union）\n");
    printf("    union 可以整体赋值，不能比较\n");
    printf("    位域 union 好用但布局依实现，只用于平台固定的场景\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 01_共用体.c -o bin/01_共用体
 *     ./bin/01_共用体
 *
 * 输出如下：
 *
 * 1. 定义与大小
 *    sizeof(Number)       = 4（最大成员 4 字节）
 *    sizeof(NumberStruct) = 12（成员各占各的，再加填充）
 *    同样的成员，union 省了 8 字节
 *    union Mixed { char c; int i; }  → sizeof = 4（对齐到 4）
 *    union Small { char c; short s; } → sizeof = 2（对齐到 2）
 *    规律：大小 = 最大成员，向上取整到最大对齐的倍数
 * 2. 初始化：同一时刻只有一个有效成员
 *    ① { 42 }          → i = 42
 *    ② { .f = 3.14f }  → f = 3.14
 *    ③ { 0 }           → i = 0, f = 0
 *    { .i = 1, .f = 2.0f } 编译不过：两个成员共用一块内存，不能同时给值
 * 3. 内存布局：三个成员起始地址相同
 *    offsetof(Number, i)     = 0
 *    offsetof(Number, f)     = 0
 *    offsetof(Number, bytes) = 0
 *    sizeof(Number) = 4，三个成员都从同一块内存的 0 偏移开始
 *    写 n.i = 0x3f800000 之后：
 *       n.i     = 1065353216（按 int 看）
 *       n.f     = 1（按 float 看，同一串字节的另一种解释）
 *       n.bytes = 00 00 80 3f（小端平台：低字节在低地址）
 * 4. 未写入的成员：内容不确定，别读
 *    memset 全 0 后读 z.f = 0（0 的位模式就是 0.0）
 *    但没清零的 union 直接读另一个成员，得到的是垃圾字节
 *    指针强转 *(float *)&i 更糟：违反严格别名规则，-O2 下可能出错
 * 5. 类型双关：float 的位模式
 *    1.0f   → 0x3f800000
 *    0.5f   → 0x3f000000
 *    -2.0f  → 0xc0000000
 *    IEEE 754 单精度：符号 1 位 + 阶码 8 位 + 尾数 23 位
 *    这依赖浮点表示，不是所有平台都这样
 * 6. 字节序检测
 *    0x0102 的两个字节：b[0] = 0x02, b[1] = 0x01
 *    判定：小端（低位字节在低地址）
 *    本行结果依平台；x86 / ARM64 常见是小端
 * 7. union 可以整体赋值，不能比较
 *    dst = src 后 dst.f = 2.5（整块字节被拷贝）
 *    if (dst == src) 编译不过：union 没有 == 运算
 * 8. tagged union：标签 + 共用体 = 变体类型
 *    vals[0] = 42（int）
 *    vals[1] = 3.5（double）
 *    vals[2] = hello（const char *）
 *    sizeof(Value) = 16（enum 4 + 对齐 + union 8）
 *    读之前先看 kind，写的时候先设 kind —— 这两步必须成对
 *    封装成 value_int/value_print 之后，调用方就不容易忘
 * 9. 对齐盒子：让缓冲区满足最严格的对齐
 *    sizeof(AlignBox) = 64（buf 决定容量，对齐由 long double 决定）
 *    AlignBox 的对齐 = 8（用探针结构体的 offsetof 量出来）
 *    long double 的对齐随平台变（8 或 16），所以一定要量，别硬编码
 *    char buf[64] 单独用只有 1 字节对齐；套进 union 后起点天然对齐
 *    这样 pool[i].buf 里放 double、指针都不会未对齐访问
 *    往 pool[0].buf 里放一个 double：2.5
 * 10. 位域 + union：一个寄存器两种看法
 *    设 enable=1, mode=5, value=0x1234 后：
 *       r.raw        = 0x0012340b
 *       r.bits.mode  = 5
 *       r.bits.value = 0x1234
 *    sizeof(Reg) = 4（4 字节，位域正好铺满一个 uint32_t）
 *    位域布局依实现：别用这种方式解析外部协议，用移位和掩码
 *    等价的手写方式：raw |= (1u << 0); raw |= (5u << 1); raw |= (0x1234u << 8)
 * 11. 小结：
 *     结构体是“和”，共用体是“或”：所有成员共用一块内存
 *     定义语法和 struct 只差一个关键字；大小 = 最大成员 + 对齐取整
 *     初始化只能给一个成员；所有成员 offsetof 都是 0
 *     写一个读另一个 = 类型双关，依赖表示和字节序，别用于跨平台解析
 *     省内存的前提是"互斥"，配一个 enum 标签才安全（tagged union）
 *     union 可以整体赋值，不能比较
 *     位域 union 好用但布局依实现，只用于平台固定的场景
 *
 * 注：第 3 节的字节顺序、第 6 节的字节序判定、第 10 节的位域布局
 *     都依赖平台。本机是小端，换到大端平台这几行会不同。
 * ==========================================================================
 */
