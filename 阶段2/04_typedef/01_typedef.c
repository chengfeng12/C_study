/*
 * 01_typedef.c —— 阶段2 / 第 04 课 / 第 01 个文件：typedef（类型别名）
 *
 * 编译：gcc -std=c99 -Wall -Wextra 01_typedef.c -o bin/01_typedef
 * 运行：./bin/01_typedef
 *
 * typedef 只做一件事：**给一个已存在的类型起个新名字**。
 * 它不创建新类型、不分配内存、不产生任何运行时代码，
 * 但能让 C 的类型声明从"天书"变成"能读的句子"。
 *
 * 本篇按三个问题展开：
 *      为什么需要它（一~三）
 *      怎么用（四~六）
 *      用在哪（七~十一）
 *
 * ====================== 一、为什么需要 typedef ======================
 * 三个真实痛点。
 *
 * **痛点 1：C 的结构体名不带"类型"身份**
 *      struct Point { int x, y; };
 *      struct Point p;                 // 每次都要写 struct
 *      struct Point *pp = &p;          // 指针更啰嗦
 * 想只写 `Point p;`，就得靠 typedef。这是 C 与 C++ 最显眼的差别之一。
 *
 * **痛点 2：函数指针声明几乎读不出来**
 *      int (*fp)(int, int);                        // 一个函数指针
 *      int (*table[4])(int, int);                  // 四个函数指针
 *      void (*signal(int, void (*)(int)))(int);    // 现实中的噩梦
 * 只要出现两次以上，就该起名：
 *      typedef int (*BinOp)(int, int);
 *      BinOp table[4];                             // 一眼看懂
 *
 * **痛点 3：类型名承载不了语义，也挡不住"哪个平台"**
 *      unsigned long long timeout;      // 8 字节？64 位？
 *      double width, height;            // 谁是像素、谁是米？
 *      uint64_t elapsed_ms;             // typedef 之后：宽度明确、语义清楚
 *
 * ★ 一句话总结：typedef 的价值不在"少打几个字"，而在
 *   **把复杂类型藏到一个名字后面，让接口的签名表达意图而不是语法**。
 *
 * ====================== 二、语法：照抄变量声明，前面加 typedef ======================
 * 记法（最实用的一条）：
 *
 *      先像定义变量那样写：      int  x;
 *      再在前面加 typedef：      typedef int  x;        // x 变成一个类型名
 *
 * 换成复杂类型同样成立：
 *      变量：int (*fp)(int, int);        →  typedef int (*fp)(int, int);   → BinOp
 *      变量：int  arr[3];                →  typedef int  arr[3];           → Vec3
 *      变量：char *s;                    →  typedef char *s;               → string
 *
 * 也就是说：**别名出现的位置，和变量名出现的位置完全一样**。
 * 这也是为什么 `typedef int (*BinOp)(int,int);` 里 BinOp 在括号内 ——
 * 因为它替代的是"变量名"那个位置。
 *
 * ★ 常见误写：
 *      typedef int(*BinOp)(int, int);    // 合法但不推荐，空格位置看不出结构
 *      typedef (int *)(BinOp);           // ✗ 语法错误，typedef 不是类型转换
 *
 * ====================== 三、typedef 不是宏：#define vs typedef ======================
 * 两者都能"起名"，但机制完全不同：
 *
 *      #define INT_MACRO int          // 预处理期文本替换
 *      typedef int IntAlias;          // 编译期的类型别名
 *
 * 四个关键区别：
 *
 * (1) **宏是文本替换，组合时会出意外**
 *          #define PTR_INT int *
 *          PTR_INT a, b;              // 展开成 int * a, b;  → b 是 int！
 *
 *          typedef int *IntPtr;
 *          IntPtr c, d;               // c、d 都是 int *
 *      这是 typedef 最经典的优势场景。
 *
 * (2) **宏没有作用域概念**
 *      #define 一旦出现，后面全文件生效（除非 #undef）；
 *      typedef 遵守块作用域、文件作用域，可以只在函数里定义。
 *
 * (3) **宏不做类型检查，typedef 参与类型系统**
 *      宏只是"换个写法"，编译器的报错信息里看不到你的名字；
 *      typedef 的名字会出现在诊断信息和调试器里（`Point` 比 `struct Point` 好读）。
 *
 * (4) **宏能做的事更多**：条件编译、拼接标识符、生成代码 —— typedef 做不到，
 *      但它也不该做那些事。**类型相关的用 typedef，文本相关的才用宏**。
 *
 * ★ 结论：给类型起名，永远优先 typedef。
 *
 * ====================== 四、常见形态（六种，覆盖 95% 的用法） ======================
 * (1) **基本类型**：给宽度和语义起名
 *          typedef unsigned char byte;
 *          typedef unsigned int  u32;
 *          typedef double        Meters;      // 语义化，防混用（靠自觉）
 *
 * (2) **结构体**：最常用，通常和定义写在一起
 *          typedef struct Point { int x, y; } Point;
 *      也可以先定义再起名：
 *          struct Point { int x, y; };
 *          typedef struct Point Point;        // 别名和标签同名，不冲突
 *      自引用必须留标签：
 *          typedef struct Node { struct Node *next; } Node;   // ✔
 *          typedef struct      { Node *next; } Node;          // ✗ Node 还不存在
 *
 * (3) **共用体 / 枚举**：语法一致
 *          typedef union  Number { int i; float f; } Number;
 *          typedef enum   Color  { RED, GREEN, BLUE } Color;
 *
 * (4) **指针**：**慎用**（见第六节的 const 陷阱）
 *          typedef char *string;              // 危险：const string 不是 const char *
 *      真要用，把 const 一起写进名字里：
 *          typedef const char *cstring;
 *
 * (5) **数组**
 *          typedef int Vec3[3];               // Vec3 是"3 个 int 的数组"
 *          Vec3 v = { 1, 2, 3 };
 *      ★ 数组 typedef 有陷阱：当函数参数时会退化成指针（见第十一节）。
 *
 * (6) **函数指针**：typedef 最有价值的用法
 *          typedef int (*BinOp)(int, int);    // BinOp 是一种函数指针类型
 *          BinOp op = add;
 *          BinOp table[3] = { add, sub, mul };
 *          BinOp pick(char c);                // 返回函数指针，签名一眼可读
 *
 * ★ 注意别写成 `typedef int *BinOp(int,int);` —— 那是"返回 int * 的函数类型"，
 *   差一对括号，意思完全相反。
 *
 * ====================== 五、作用域与重复定义 ======================
 * typedef 也是一条声明，遵守作用域规则：
 *
 *      typedef int T;                 // 文件作用域
 *
 *      void f(void)
 *      {
 *          typedef double T;          // 块作用域：在 f 里 T 是 double
 *          T x = 1.5;                 // double
 *      }
 *      T y = 1;                       // 回到文件作用域，T 是 int
 *
 * 用途：把"某个函数内部才需要的复杂类型"限制在局部，减少命名污染。
 *
 * ★ 同一作用域**不能重复 typedef 成不同类型**：
 *      typedef int T;
 *      typedef double T;              // ✗ 冲突
 *   把相同类型重复定义两遍，C11 允许、C99 不允许 ——
 *   写头文件时用 include guard 挡住重复包含，比依赖这条规则可靠。
 *
 * ★ 头文件里的 typedef 要有保护：
 *      #ifndef POINT_H
 *      #define POINT_H
 *      typedef struct Point { int x, y; } Point;
 *      #endif
 *
 * ====================== 六、const 陷阱：typedef 会把指针"藏起来" ======================
 * 这是 typedef 最坑的地方，必须单独讲。
 *
 *      typedef char *string;
 *      const string s = buf;
 *
 * 直觉：s 应该是"指向只读字符的指针"。**错。**
 *
 * const 修饰的是**typedef 后的整体类型**，也就是 `char *`：
 *      const string  ==  char *const s     // 指针只读，数据可写！
 *      const char *  !=  string            // 这才是"数据只读"
 *
 * 后果：
 *      s = other;      // ✗ 编译错误（指针本身是 const）
 *      s[0] = 'X';     // ✔ 合法！数据照样能改
 *
 * 这就是为什么很多规范禁止 `typedef char *string;`：
 * 调用方以为加了 const 就安全，实际保护错了对象。
 *
 * ★ 正确姿势：**把 const 写进 typedef 里，别指望调用方加**：
 *      typedef const char *cstring;
 *      cstring s = "hello";      // s[0] = 'H' 编译不过；s = other 可以
 *
 * ★ 通用规则：**typedef 一个指针类型时，const 会落在指针上，不会穿透到目标**。
 *   要么别 typedef 指针，要么把限定符写死在名字里。
 *
 * ====================== 七、场景一：简化结构体名（最常见） ======================
 *      typedef struct Point { int x, y; } Point;
 *
 *      Point p;                    // 不用写 struct
 *      Point *pp = &p;
 *      Point arr[10];
 *      Point make(int x, int y);
 *
 * 三个附带好处：
 *  - 函数签名短了，读起来像"高级语言"
 *  - 以后要改实现（比如换成不透明指针），只改 typedef，调用方不动
 *  - 调试器里显示 `Point`，不是 `struct Point`
 *
 * ★ 别名和标签同名不冲突（它们在不同命名空间），这是行业惯例写法。
 *
 * ====================== 八、场景二：函数指针与回调表 ======================
 * 回调接口的签名一旦出现两次，就该起名：
 *
 *      typedef int (*BinOp)(int, int);
 *
 *      int apply(int a, int b, BinOp op);        // 参数意图清楚
 *      BinOp pick_op(char c);                    // 返回函数指针，可读
 *      BinOp table[] = { add, sub, mul };        // 分发表，语法干净
 *
 * 对比不 typedef 的版本：
 *      int apply(int a, int b, int (*op)(int, int));
 *      int (*pick_op(char c))(int, int);
 *      int (*table[])(int, int) = { add, sub, mul };
 *
 * 功能完全一样，可读性差一个量级。函数指针的细节见 09 篇。
 *
 * ★ 更上一层：把"名字 + 函数指针"打包成结构体，就是命令表：
 *      typedef struct { const char *name; BinOp fn; } OpEntry;
 *
 * ====================== 九、场景三：平台无关整型 ======================
 * 标准库的 <stdint.h> 就是一堆 typedef：
 *
 *      typedef signed char        int8_t;
 *      typedef short              int16_t;
 *      typedef int                int32_t;      // 具体底层类型依平台
 *      typedef long long          int64_t;
 *      typedef unsigned int       uint32_t;
 *
 * 为什么需要：`int` 的宽度**不保证**是 32 位（标准只说"至少 16 位"）。
 * 协议、文件格式、哈希，都必须用固定宽度类型。
 *
 *      uint32_t crc;            // 明确 32 位无符号
 *      int32_t  delta;          // 明确 32 位有符号
 *
 * ★ 同理还有 `<stddef.h>` 的 `size_t`、`ptrdiff_t`：
 *      size_t   是 sizeof 的结果类型，宽度随平台变（32 位 4 字节、64 位 8 字节）
 *      ptrdiff_t 是指针相减的类型
 *   用它们而不是 `int`，能避免大量"64 位下的截断"bug。
 *
 * ====================== 十、场景四：不透明类型（抽象数据类型） ======================
 * 库设计里最有用的一招：**头文件只暴露名字，不暴露成员**。
 *
 *      // ---- stack.h ----
 *      typedef struct Stack Stack;        // 只有名字，没有定义 → 不完整类型
 *      Stack *stack_new(void);
 *      void   stack_push(Stack *s, int v);
 *      void   stack_free(Stack *s);
 *
 *      // ---- stack.c ----
 *      struct Stack {                     // 真正的定义藏在这里
 *          int *data;
 *          int  top, cap;
 *      };
 *
 * 调用方拿到的是 `Stack *`，但：
 *  - 看不到成员，改不了内部结构 → 实现可以随时重构
 *  - 不能 `Stack s;`（大小未知），只能通过 new/free → 强制走接口
 *  - 二进制兼容性更好（改结构体大小不用重编调用方）
 *
 * 代价：多一次 malloc、多一次间接访问、不能放栈上。
 *
 * ★ 这就是 C 的"封装"：语言不给你 private，就用**不完整类型 + 指针**实现。
 *
 * ====================== 十一、场景五：语义化与数组类型（附两个坑） ======================
 * (1) **语义化别名**（文档作用，无强制力）
 *          typedef double Meters;
 *          typedef double Seconds;
 *          Meters  d = 3.5;
 *          Seconds t = 2.0;
 *          d = t;                     // ✔ 编译器不拦！都是 double
 *      C 的类型系统不区分"同底层类型的不同别名"，
 *      所以这只能提醒人，不能防错。要强约束得靠结构体包一层。
 *
 * (2) **数组别名**：写起来方便，但**函数参数会退化**
 *          typedef int Vec3[3];
 *          void f(Vec3 v);            // 等价于 void f(int *v)
 *      ★ 在 f 里 `sizeof v` 是 **8**（指针大小），不是 12！
 *        长度信息丢了，必须另外传。
 *      结论：数组 typedef 适合"局部变量/结构体成员"，
 *      不适合做函数参数类型 —— 除非你清楚它已经退化成指针。
 *
 * (3) **数组指针别名**：二维数组传参时很好用
 *          typedef int (*RowPtr)[3];  // 指向"3 个 int 的数组"
 *          void g(RowPtr rows, int n);  // 比 int (*rows)[3] 好读
 *
 * ====================== 十二、常见坑 ======================
 *  1. 以为 typedef 创建了新类型         它只是别名，和原类型完全等价
 *  2. `typedef int *P; const P p;`       const 落在指针上，不是数据上（第六节）
 *  3. 用 #define 代替 typedef            `#define PTR int *` 会让 `PTR a, b;` 出错
 *  4. 函数指针 typedef 少写括号          `typedef int *F(int,int)` 是函数类型，不是指针
 *  5. 匿名结构体 + typedef 里自引用      `typedef struct { Node *next; } Node;` 编译不过
 *  6. 数组 typedef 当函数参数            `sizeof` 变成指针大小，长度丢失
 *  7. typedef 指针后加 const 以为是只读  实际保护错了对象
 *  8. 同一作用域重复 typedef 不同类型    冲突；相同类型在 C99 下也可能报错
 *  9. 头文件里 typedef 没加 include guard  重复包含时冲突
 * 10. 用 typedef 掩盖过度复杂的类型     绕三层别名之后没人知道底层是什么
 *
 * ====================== 十三、实践建议 ======================
 *  - 结构体一律 `typedef struct X { ... } X;`，别名和标签同名
 *  - 函数指针一律 typedef，签名出现两次就起名
 *  - 定宽整数用 `<stdint.h>` 的 `int32_t` / `uint64_t`，别用裸 int
 *  - 尺寸、下标用 `size_t`，指针差值用 `ptrdiff_t`
 *  - **不要 typedef 指针类型**；非要起名就把 const 写进去（`cstring`）
 *  - 库对外只暴露不透明类型 + 指针，成员定义留在 .c
 *  - 数组 typedef 只用于成员/局部变量，函数参数老老实实写 `T *` + 长度
 *  - 别名要短、要有意义：`Point` 好，`POINT_STRUCT_TYPE` 只是噪音
 *  - 别为了"看起来面向对象"而堆 typedef，类型层次过深反而难查
 */

#include <stdio.h>
#include <stdlib.h>      /* malloc / free */
#include <stdint.h>      /* uint32_t / int64_t */
#include <stddef.h>      /* size_t / ptrdiff_t */

/* ====================== 基本类型别名 ====================== */
typedef unsigned char byte;
typedef unsigned int  u32;
typedef int32_t       i32;          /* 定宽：明确 4 字节 */

/* ====================== 结构体 / 共用体 / 枚举 ====================== */

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Node {               /* 自引用：必须留标签 */
    int          value;
    struct Node *next;
} Node;

typedef union Number {
    int   i;
    float f;
} Number;

typedef enum Color {
    RED,
    GREEN,
    BLUE
} Color;

/* ====================== 指针别名（反面 + 正面） ====================== */

typedef char       *string;         /* 反面教材：const 会落在指针上 */
typedef const char *cstring;        /* 正面写法：把 const 写进名字 */

/* ====================== 函数指针 ====================== */

typedef int (*BinOp)(int, int);

static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }

static int apply(int a, int b, BinOp op)      /* 签名一眼可读 */
{
    return op != NULL ? op(a, b) : 0;
}

/* 名字 + 函数指针 = 命令表 */
typedef struct OpEntry {
    const char *name;
    BinOp       fn;
} OpEntry;

static const OpEntry ops[] = {
    { "add", add },
    { "sub", sub },
    { "mul", mul },
};

/* ====================== 数组与数组指针 ====================== */

typedef int Vec3[3];                /* 3 个 int 的数组 */
typedef int (*RowPtr)[3];           /* 指向"3 个 int 的数组"的指针 */

static int vec3_sum(const Vec3 v)   /* 注意：v 实际已退化为 int * */
{
    int s = 0;
    for (int i = 0; i < 3; i++)
        s += v[i];
    return s;
}

static void vec3_print(const char *label, const Vec3 v)
{
    printf("   %s = [%d, %d, %d]\n", label, v[0], v[1], v[2]);
}

/* ====================== 不透明类型（ADT） ====================== */

typedef struct Counter Counter;     /* 只有名字：调用方看不到成员 */

struct Counter {                    /* 定义放在"实现文件"里 */
    int value;
};

static Counter *counter_new(void)
{
    Counter *c = malloc(sizeof *c);
    if (c != NULL)
        c->value = 0;
    return c;                       /* 调用方负责 counter_free */
}

static void counter_inc(Counter *c)
{
    c->value++;
}

static int counter_get(const Counter *c)
{
    return c->value;
}

static void counter_free(Counter *c)
{
    free(c);
}

int main(void)
{
    /* ---------- 1. 为什么需要：两种写法对比 ---------- */
    printf("1. 为什么需要 typedef\n");
    struct Point raw = { 1, 2 };         /* 不 typedef：必须写 struct */
    Point        nice = { 3, 4 };        /* typedef 之后：像普通类型 */
    printf("   struct Point raw = {%d, %d}  ← 每次都要写 struct\n", raw.x, raw.y);
    printf("   Point nice       = {%d, %d}  ← 别名之后\n", nice.x, nice.y);
    printf("   int (*fp)(int,int); 这种声明出现两次以上就该起名\n");

    /* ---------- 2. 语法：照抄变量声明 ---------- */
    printf("2. 语法：先写变量声明，再在前面加 typedef\n");
    printf("   int x;                    变量 x\n");
    printf("   typedef int x;            x 变成类型名\n");
    printf("   int (*fp)(int,int);       变量：函数指针\n");
    printf("   typedef int (*BinOp)(int,int);   BinOp 变成类型名\n");

    /* ---------- 3. typedef vs #define ---------- */
    printf("3. typedef 不是宏\n");
    printf("   宏是预处理期文本替换；typedef 是编译期的类型别名\n");
    printf("   经典差别：#define PTR int * 之后 PTR a, b; 里 b 是 int，不是指针\n");
    printf("   typedef int *IntPtr; 之后 IntPtr c, d; 两个都是指针\n");

    typedef int *IntPtr;
    int v1 = 1;
    IntPtr c = &v1, d = &v1;
    printf("   sizeof(c) = %zu, sizeof(d) = %zu，都等于 sizeof(int *) = %zu\n",
           sizeof c, sizeof d, sizeof(int *));

    /* ---------- 4. 各种形态 ---------- */
    printf("4. 六种常见形态\n");
    byte b8   = 0x7f;
    u32  u    = 4000000000u;
    i32  i    = -123456;
    printf("   byte = %u, u32 = %u, i32 = %d（宽度由 typedef 固定）\n",
           (unsigned)b8, (unsigned)u, (int)i);

    Color col = GREEN;
    printf("   enum：Color col = GREEN → %d\n", (int)col);

    Number num;
    num.f = 2.5f;
    printf("   union：Number 里存 float → %g\n", (double)num.f);

    Node n2 = { 2, NULL };
    Node n1 = { 1, &n2 };
    printf("   自引用：Node{1} -> Node{2} -> NULL，value = %d -> %d\n",
           n1.value, n1.next->value);

    /* ---------- 5. 数组与数组指针 ---------- */
    printf("5. 数组 typedef\n");
    Vec3 vec = { 1, 2, 3 };
    vec3_print("Vec3 vec", vec);
    printf("   sizeof(vec) = %zu（在 main 里，数组类型完整）\n", sizeof vec);
    printf("   vec3_sum(vec) = %d\n", vec3_sum(vec));
    printf("   但在函数里 sizeof(v) = %zu：数组参数已退化为指针，长度丢了\n",
           sizeof(Vec3 *));

    int grid[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
    RowPtr row = grid;                       /* 指向一整行 */
    printf("   RowPtr row = grid; row[1][2] = %d（第二行第三个）\n",
           row[1][2]);

    /* ---------- 6. const 陷阱 ---------- */
    printf("6. const 陷阱：typedef 指针会隐藏 const\n");
    char msg[] = "hello";

    const string cs = msg;      /* 等价于 char *const cs：指针只读，数据可写 */
    cs[0] = 'H';                /* ✔ 合法！这就是陷阱 */
    printf("   const string cs = msg; cs[0] = 'H' → msg = %s（数据被改了）\n",
           msg);
    printf("   因为 const string == char *const，const 落在了指针上\n");

    cstring ro = "world";       /* const char *：数据只读 */
    ro = msg;                   /* ✔ 指针本身可以改指向 */
    printf("   cstring ro = msg 之后 ro = %s；ro[0] = 'W' 会编译失败\n", ro);
    printf("   结论：不要 typedef 指针；要起名就把 const 写进去\n");

    /* ---------- 7. 函数指针与命令表 ---------- */
    printf("7. 函数指针 typedef\n");
    BinOp op = add;
    printf("   apply(7, 5, add) = %d，apply(7, 5, sub) = %d\n",
           apply(7, 5, op), apply(7, 5, sub));
    printf("   apply(7, 5, NULL) = %d（接口允许传空，内部判空）\n",
           apply(7, 5, NULL));

    for (size_t i = 0; i < sizeof ops / sizeof ops[0]; i++)
        printf("   命令表：%-4s(8, 2) = %d\n", ops[i].name, ops[i].fn(8, 2));

    /* ---------- 8. 不透明类型 ---------- */
    printf("8. 不透明类型：头文件只暴露名字\n");
    Counter *cnt = counter_new();
    if (cnt == NULL) {
        printf("   分配失败\n");
        return 1;
    }
    counter_inc(cnt);
    counter_inc(cnt);
    counter_inc(cnt);
    printf("   counter_new/inc×3/get = %d\n", counter_get(cnt));
    printf("   调用方只能拿 Counter *，看不到 value 成员，也改不了\n");
    printf("   代价：多一次 malloc、不能放栈上；换来的是封装和二进制兼容\n");
    counter_free(cnt);
    cnt = NULL;

    /* ---------- 9. 作用域 ---------- */
    printf("9. typedef 也有作用域\n");
    {
        typedef double LocalType;        /* 只在块内有效 */
        LocalType lt = 1.5;
        printf("   块内 LocalType lt = %.1f\n", lt);
    }
    printf("   出了块，LocalType 就不存在了（这里不能用它）\n");

    /* ---------- 10. 语义化别名的局限 ---------- */
    printf("10. 语义化别名只是文档，编译器不拦\n");
    typedef double Meters;
    typedef double Seconds;
    Meters  dist  = 3.5;
    Seconds tm    = 2.0;
    dist = tm;                       /* ✔ 编译器不报错：都是 double */
    printf("   dist = tm 合法（现在 dist = %.1f）：底层都是 double\n", dist);
    printf("   真要强约束，得用结构体包一层（见 02 篇）\n");

    /* ---------- 11. 定宽整型 ---------- */
    printf("11. 定宽整型：stdint.h 就是一堆 typedef\n");
    printf("   sizeof(int32_t) = %zu, sizeof(uint64_t) = %zu\n",
           sizeof(int32_t), sizeof(uint64_t));
    printf("   而 sizeof(int) = %zu 只保证“至少 16 位”，跨平台别拿它当 4 字节\n",
           sizeof(int));
    printf("   同理 size_t = %zu（sizeof 的结果类型），ptrdiff_t = %zu（指针差）\n",
           sizeof(size_t), sizeof(ptrdiff_t));
    printf("   协议、文件格式、哈希，一律用定宽类型\n");

    /* ---------- 12. 小结 ---------- */
    printf("12. 小结：\n");
    printf("    typedef 只是别名：不建新类型、不占内存、无运行时开销\n");
    printf("    语法照抄变量声明，别名写在变量名那个位置\n");
    printf("    typedef 有作用域、参与类型检查；#define 只是文本替换\n");
    printf("    结构体 typedef struct X { ... } X; 是行业惯例\n");
    printf("    函数指针一律 typedef，签名出现两次就起名\n");
    printf("    不要 typedef 指针；要起名就把 const 写进名字里\n");
    printf("    数组 typedef 当函数参数会退化为指针，长度必须另传\n");
    printf("    定宽整型、size_t、不透明类型，都是 typedef 的主战场\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 01_typedef.c -o bin/01_typedef
 *     ./bin/01_typedef
 *
 * 输出如下：
 *
 * 1. 为什么需要 typedef
 *    struct Point raw = {1, 2}  ← 每次都要写 struct
 *    Point nice       = {3, 4}  ← 别名之后
 *    int (*fp)(int,int); 这种声明出现两次以上就该起名
 * 2. 语法：先写变量声明，再在前面加 typedef
 *    int x;                    变量 x
 *    typedef int x;            x 变成类型名
 *    int (*fp)(int,int);       变量：函数指针
 *    typedef int (*BinOp)(int,int);   BinOp 变成类型名
 * 3. typedef 不是宏
 *    宏是预处理期文本替换；typedef 是编译期的类型别名
 *    经典差别：#define PTR int * 之后 PTR a, b; 里 b 是 int，不是指针
 *    typedef int *IntPtr; 之后 IntPtr c, d; 两个都是指针
 *    sizeof(c) = 8, sizeof(d) = 8，都等于 sizeof(int *) = 8
 * 4. 六种常见形态
 *    byte = 127, u32 = 4000000000, i32 = -123456（宽度由 typedef 固定）
 *    enum：Color col = GREEN → 1
 *    union：Number 里存 float → 2.5
 *    自引用：Node{1} -> Node{2} -> NULL，value = 1 -> 2
 * 5. 数组 typedef
 *    Vec3 vec = [1, 2, 3]
 *    sizeof(vec) = 12（在 main 里，数组类型完整）
 *    vec3_sum(vec) = 6
 *    但在函数里 sizeof(v) = 8：数组参数已退化为指针，长度丢了
 *    RowPtr row = grid; row[1][2] = 6（第二行第三个）
 * 6. const 陷阱：typedef 指针会隐藏 const
 *    const string cs = msg; cs[0] = 'H' → msg = Hello（数据被改了）
 *    因为 const string == char *const，const 落在了指针上
 *    cstring ro = msg 之后 ro = Hello；ro[0] = 'W' 会编译失败
 *    结论：不要 typedef 指针；要起名就把 const 写进去
 * 7. 函数指针 typedef
 *    apply(7, 5, add) = 12，apply(7, 5, sub) = 2
 *    apply(7, 5, NULL) = 0（接口允许传空，内部判空）
 *    命令表：add (8, 2) = 10
 *    命令表：sub (8, 2) = 6
 *    命令表：mul (8, 2) = 16
 * 8. 不透明类型：头文件只暴露名字
 *    counter_new/inc×3/get = 3
 *    调用方只能拿 Counter *，看不到 value 成员，也改不了
 *    代价：多一次 malloc、不能放栈上；换来的是封装和二进制兼容
 * 9. typedef 也有作用域
 *    块内 LocalType lt = 1.5
 *    出了块，LocalType 就不存在了（这里不能用它）
 * 10. 语义化别名只是文档，编译器不拦
 *    dist = tm 合法（现在 dist = 2.0）：底层都是 double
 *    真要强约束，得用结构体包一层（见 02 篇）
 * 11. 定宽整型：stdint.h 就是一堆 typedef
 *    sizeof(int32_t) = 4, sizeof(uint64_t) = 8
 *    而 sizeof(int) = 4 只保证“至少 16 位”，跨平台别拿它当 4 字节
 *    同理 size_t = 8（sizeof 的结果类型），ptrdiff_t = 8（指针差）
 *    协议、文件格式、哈希，一律用定宽类型
 * 12. 小结：
 *     typedef 只是别名：不建新类型、不占内存、无运行时开销
 *     语法照抄变量声明，别名写在变量名那个位置
 *     typedef 有作用域、参与类型检查；#define 只是文本替换
 *     结构体 typedef struct X { ... } X; 是行业惯例
 *     函数指针一律 typedef，签名出现两次就起名
 *     不要 typedef 指针；要起名就把 const 写进名字里
 *     数组 typedef 当函数参数会退化为指针，长度必须另传
 *     定宽整型、size_t、不透明类型，都是 typedef 的主战场
 * ==========================================================================
 */
