/*
 * 11_常量.c —— 阶段1 / 第 02 课 / 第 11 个文件：常量
 *
 * 编译：gcc 11_常量.c -o const_demo
 * 运行：./const_demo
 *
 * ============================ 一、什么是常量 ============================
 * 常量（constant）= 程序运行期间"值不能改变"的量。
 * 与之相对的是变量（variable），变量的值可以随时改。
 *
 *      int a = 10;          // 变量：a 之后还能改成 20
 *      const int b = 10;    // 常量：b 定义后就不能再改
 *
 * 为什么需要常量：
 *   1) 防止误改：比如圆周率、数组上限、端口号，被改了就是 bug
 *   2) 见名知义：MAX_LEN 比裸写 100 好懂
 *   3) 便于维护：改一处，全文件生效，不用满代码找 100
 *   4) 让编译器帮你检查：写错立刻报错，而不是上线后发现
 *
 * ★ 重点提醒：C 语言里"常量"有四种不同的实现方式，它们的行为差别很大，
 *   很多教材混着讲，导致初学者以为 const 和 #define 是一回事。实际上：
 *       const 变量  有类型、占内存（可能）、不能改
 *       #define     纯文本替换、无类型、不占内存
 *       enum        整数常量集合
 *       字面量      直接写在代码里的值
 *
 * ========================= 二、四种常量形式 =========================
 *  形式            写法                     特点
 *  字面量          42、"abc"、3.14          直接写在代码里，没有名字
 *  const 变量      const int MAX = 100;     有类型，编译器做类型检查 C99 中新增的定义方式
 *  宏常量          #define MAX 100          预处理阶段文本替换，无类型
 *  枚举常量        enum { MAX = 100 };      本质是 int 常量，可读性好
 *
 * ========================== 三、字面量常量 ==========================
 * 直接写在代码里的值，就是"字面量"（literal）：
 *      整数：42、052（八进制）、0x2A（十六进制）、42u、42LL
 *      浮点：3.14、3.14f、3.14L、1e3
 *      字符：'A'、'\n'、'\0'
 *      字符串："hello"（注意是双引号，且不可修改）
 *
 * 字面量的问题：没有名字。
 *      if (status == 404)          // 404 是什么？得猜
 *      if (status == NOT_FOUND)    // 一看就懂 ← 应该这样
 * 这叫"魔法数字"（magic number），是代码坏味道之一。
 *
 * ★ 字符串字面量是只读的：
 *      char *p = "hello";
 *      p[0] = 'H';     // ✗ 未定义行为，通常直接崩溃
 *   因为 "hello" 存在只读数据段（.rodata），要改必须复制到数组里：
 *      char buf[] = "hello";
 *      buf[0] = 'H';   // ✓
 *
 * ========================= 四、const 只读变量 =========================
 * 语法：const 类型 名字 = 初值;
 *      const int    MAX_STUDENTS = 50;
 *      const double PI           = 3.141592653589793;
 *      const char   GRADE        = 'A';
 *
 * ★ 三条硬规则：
 *   1) 必须在定义时初始化（因为之后再也没有机会赋值了）
 *          const int x;      // ✗ 合法但无法赋值，等于废了
 *          const int y = 10; // ✓
 *   2) 不能再赋值
 *          y = 20;           // ✗ 编译错误：assignment of read-only variable
 *   3) 可以写在类型前面，也可以写在类型后面，效果一样（推荐写前面）
 *          const int a = 1;
 *          int const b = 1;  // 等价，但少用
 *
 * ★ 4.1 const 与指针：三种位置，三种含义（重点难点）
 * 看 const 在 * 的哪一边，就读成"谁不能变"：
 *
 *      const int *p;         // 指向常量的指针：*p 不能改，p 可以改指向
 *      int * const p;        // 常量指针：p 不能改指向，*p 可以改
 *      const int * const p;  // 两者都不能改
 *
 * 记忆口诀：const 在 * 左边 → 管"指向的内容"；在 * 右边 → 管"指针自己"。
 *      const  int * p     ← const 修饰 int，表示"这个 int 是常量"
 *      int * const p      ← const 修饰 p，表示"这个指针是常量"
 *
 * 常见用途：函数参数加 const，承诺"我只读，不修改你的数据"
 *      void print_array(const int *arr, int n);
 * 好处：调用者放心；函数内部误写 arr[0] = 1 会直接编译报错。
 *
 * ★ 4.2 const 不是"编译期常量"（C 语言的坑，考试常考）
 * 在 C 里，const int n = 10; 的 n 仍然是"只读变量"，
 * 它不满足"整型常量表达式"的要求，所以不能用在：
 *      int arr[n];              // C99 起这是"变长数组 VLA"，运行时才定大小
 *      switch (x) { case n: }   // ✗ 报错：case 需要常量表达式
 *      int arr2[n] = {0};       // ✗ 报错
 * 想要"真正的编译期常量"，用 #define 或 enum（C++ 里 const 可以，C 不行）。
 * 这是 C 和 C++ 的一个著名差异，务必分清。
 *
 * ========================== 五、#define 宏常量 ==========================
 * 语法：#define 名字 替换文本
 *      #define PI      3.14159
 *      #define MAX_LEN 100
 *      #define NAME    "小明"
 *
 * 特点（和 const 完全不同）：
 *   1) 它发生在"预处理阶段"，比编译更早，本质是"文本替换"
 *      代码里所有 PI 都被原样替换成 3.14159，编译器根本不知道有 PI 这个名字
 *   2) 没有类型，没有作用域（从定义处到文件末尾，或到 #undef）
 *   3) 不占内存，不进行类型检查
 *   4) 末尾不写分号！写了分号，分号也会被替换进去
 *          #define MAX 100;      // ✗ 灾难：int a = MAX; 变成 int a = 100;;
 *   5) 不能调试（调试器看不到 PI，只能看到 3.14159）
 *
 * ★ 5.1 宏展开的括号陷阱（最常见的宏 bug）
 *      #define SQUARE(x)  x * x
 *      SQUARE(1 + 2)   →  1 + 2 * 1 + 2  =  5    ← 错了！本该是 9
 * 因为宏只是文本替换，1+2 被直接塞进 x 的位置，运算优先级变了。
 * 正确写法：每个参数加括号，整个表达式再加一层括号
 *      #define SQUARE(x)  ((x) * (x))
 *
 * ★ 5.2 宏的副作用问题（带参数的宏要慎用）
 *      #define SQUARE(x) ((x) * (x))
 *      SQUARE(i++)    →  ((i++) * (i++))     ← i 被加了两次，未定义行为
 * 所以"带参数的宏"在现代 C 里尽量换成 static inline 函数：
 *      static inline int square(int x) { return x * x; }
 * 有类型检查、能调试、没有副作用。
 *
 * ★ 5.3 其他宏常量陷阱
 *      #define MAX 100
 *      #define MAX 200     // 重复定义，编译器可能只警告
 *      宏名会参与文本替换，如果起了个短名字（如 #define x 1），
 *      代码里所有变量 x 都会被替换，灾难性后果。所以宏名一律全大写。
 *
 * =========================== 六、enum 枚举常量 ===========================
 * 枚举用来定义"一组有名字的整数常量"，最适合表示状态、类别：
 *      enum Color { RED, GREEN, BLUE };
 * 默认从 0 开始递增：RED=0, GREEN=1, BLUE=2。
 * 也可以手动指定：
 *      enum HttpStatus { HTTP_OK = 200, HTTP_NOT_FOUND = 404 };
 *      后面的值继续递增：HTTP_NOT_FOUND 之后是 405...
 *
 * 优点：
 *   1) 是真正的整型常量，能用在 case 标签、数组大小里
 *   2) 有类型名（enum Color），可读性和自解释性比 #define 好
 *   3) 调试时能看到名字（不是所有调试器都支持，但比宏强）
 * 注意：C 的 enum 底层就是 int，所以 enum 变量可以赋任意整数，
 *       编译器一般不检查（C++ 会严格一些）。
 *
 * ===================== 七、const vs #define 对比 =====================
 *  对比项            const 变量              #define 宏
 *  处理阶段          编译期                  预处理期（更早）
 *  有无类型          有，做类型检查          无，纯文本
 *  是否占内存        通常占（可能被优化掉）  不占（只是文本）
 *  能否调试          能                      不能（调试器看不到）
 *  作用域            遵循块作用域            从定义处到文件末尾/#undef
 *  能否取地址        &MAX 可以               &MAX 不行（它不是一个变量）
 *  能否用于 case/数组 不能（C 里不是常量表达式） 能
 *  安全性            高                      低（括号、副作用陷阱）
 *
 * 选择建议：
 *    - 需要类型安全、需要作用域、作为函数参数 → const
 *    - 需要"编译期常量"（数组大小、case 标签）→ #define 或 enum
 *    - 一组相关的整数（状态码、颜色）→ enum
 *    - 现代 C 里，能用 const / enum 就别用带参宏
 *
 * ========================== 八、常见问题清单 ==========================
 *  1. const 变量忘了初始化            const int x;  之后无法赋值
 *  2. 试图修改 const 变量             x = 10; 编译报错
 *  3. 试图修改字符串字面量            char *p = "abc"; p[0]='A'; 崩溃
 *  4. 宏末尾加分号                    #define MAX 100; 导致语法错误
 *  5. 宏参数不加括号                  SQUARE(1+2) 得到 5 而不是 9
 *  6. 宏参数有副作用                  SQUARE(i++) 使 i 加两次
 *  7. 用 const 变量当数组大小         int arr[N]; 在 C 里是 VLA，不是常量数组
 *  8. 用 const 变量当 case 标签       C 里不行，要用 #define / enum
 *  9. 宏名与变量名冲突                小写宏名会污染整个文件
 * 10. 宏没有作用域，同名宏互相覆盖    不同头文件里的 MAX 可能打架
 * 11. 以为 const 能保证"运行时绝对不变"  可以通过指针强转绕过（属于 UB），别这么干
 * 12. 忘记 #undef                    临时宏一直有效到最后
 *
 * ============================ 九、实践建议 ============================
 *  - 宏常量名全大写：MAX_LEN、PI、BUFFER_SIZE
 *  - 带参宏每个参数都加括号，整个表达式再加一层括号（或者干脆用 inline 函数）
 *  - 函数参数只读时加 const：void show(const char *s)
 *  - 一组相关常量用 enum，业务含义清晰
 *  - 数组大小、case 标签这类"编译期常量"用 #define 或 enum，不要用 const
 *  - 能用 const 就别用宏：有类型检查，出错能定位
 */

#include <stdio.h>

/* ---------- 宏常量：全部大写，末尾绝对不写分号 ---------- */
#define PI          3.14159
#define MAX_LEN     100
#define APP_NAME    "C 语言学习"
#define SQUARE(x)   ((x) * (x))         /* 正确写法：参数加括号，整体再加括号 */
#define BAD_SQUARE(x) x * x             /* 反面教材：故意不加括号 */

/* ---------- 枚举常量：一组相关的整数 ---------- */
enum Color { RED, GREEN, BLUE };                       /* 默认 0, 1, 2 */
enum HttpStatus { HTTP_OK = 200, HTTP_NOT_FOUND = 404 };

/* ---------- const 作为函数参数：承诺只读 ---------- */
/* const 在这里的作用是"我不会改你的数组"，调用者可以放心传进来 */
static void print_array(const int *arr, int n)
{
    printf("   数组内容：");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
        /* arr[i] = 0;  取消注释会编译报错：read-only parameter */
    }
    printf("\n");
}

int main(void)
{
    /* ---------- 1. const 基本用法 ---------- */
    const int    MAX_STUDENTS = 50;
    const double PI_CONST     = 3.141592653589793;
    const char   GRADE        = 'A';

    printf("1. const 常量：MAX_STUDENTS=%d  PI_CONST=%.5f  GRADE=%c\n",
           MAX_STUDENTS, PI_CONST, GRADE);
    /* MAX_STUDENTS = 60;   取消注释会编译报错：assignment of read-only variable */

    /* ---------- 2. const 数组：每个元素都不能改 ---------- */
    const int days_in_month[3] = {31, 28, 31};
    printf("2. const 数组：%d %d %d（元素同样只读）\n",
           days_in_month[0], days_in_month[1], days_in_month[2]);

    /* ---------- 3. 宏常量 ---------- */
    printf("3. 宏常量：PI=%.5f  MAX_LEN=%d  APP_NAME=%s\n",
           PI, MAX_LEN, APP_NAME);
    /* 宏在预处理阶段就被替换成字面量，编译器看到的其实是 3.14159、100 */

    /* ---------- 4. 宏的括号陷阱：正确 vs 错误 ---------- */
    printf("4. SQUARE(3)       = %d（正确：3*3）\n", SQUARE(3));
    printf("   SQUARE(1 + 2)   = %d（正确：先算 1+2，再平方）\n", SQUARE(1 + 2));
    printf("   BAD_SQUARE(1+2) = %d（错误！展开成 1+2*1+2 = 5）\n",
           BAD_SQUARE(1 + 2));
    printf("   → 宏只是文本替换，不进行任何运算，括号必须自己加\n");

    /* ---------- 5. 宏展开后的真实样子（用字符串演示） ---------- */
    printf("5. BAD_SQUARE(1+2) 展开后是： 1 + 2 * 1 + 2\n");
    printf("   因为 * 的优先级高于 +，所以算成 1 + 2 + 2 = %d\n",
           1 + 2 * 1 + 2);

    /* ---------- 6. 带参宏的副作用（这里只演示正确用法） ---------- */
    int i = 3;
    int sq = SQUARE(i);              /* i 只被读一次，安全 */
    printf("6. SQUARE(i) = %d，i 仍然是 %d\n", sq, i);
    /* 危险写法：SQUARE(i++) 会展开成 ((i++) * (i++))，i 被加两次，未定义行为 */

    /* ---------- 7. enum 枚举常量 ---------- */
    enum Color c = GREEN;
    printf("7. enum Color: RED=%d GREEN=%d BLUE=%d，当前 c=%d\n",
           RED, GREEN, BLUE, c);

    enum HttpStatus st = HTTP_NOT_FOUND;
    printf("   状态码：HTTP_OK=%d  HTTP_NOT_FOUND=%d，当前 st=%d\n",
           HTTP_OK, HTTP_NOT_FOUND, st);
    printf("   枚举本质就是 int，可以参与比较：st == HTTP_NOT_FOUND ? %s\n",
           (st == HTTP_NOT_FOUND) ? "是" : "否");

    /* ---------- 8. const 与指针的三种组合 ---------- */
    int x = 10, y = 20;

    const int *p1 = &x;          /* 指向常量的指针：不能通过 p1 改 x */
    int * const p2 = &x;         /* 常量指针：p2 不能改指向 */
    const int * const p3 = &x;   /* 两者都不能改 */

    printf("8. *p1=%d  *p2=%d  *p3=%d\n", *p1, *p2, *p3);

    p1 = &y;                     /* ✓ 可以改指向 */
    printf("   p1 改指向 y 后：*p1=%d\n", *p1);
    /* *p1 = 99;    取消注释报错：p1 指向的内容是只读的 */
    /* p2 = &y;     取消注释报错：p2 自己是只读的 */
    /* *p3 = 99;  p3 = &y;   都不行 */

    /* ---------- 9. const 作为函数参数（最常见的最佳实践） ---------- */
    int nums[4] = {1, 2, 3, 4};
    printf("9. 向 const 参数传数组：\n");
    print_array(nums, 4);

    /* ---------- 10. 字符串字面量不能改 ---------- */
    char buf[] = "hello";        /* 数组：内容可改 */
    buf[0] = 'H';
    printf("10. 数组可以改：%s\n", buf);
    /* char *p = "hello"; p[0] = 'H';   未定义行为，可能直接崩溃 */
    printf("    字面量 \"hello\" 存在只读段，要改必须先复制到数组\n");

    /* ---------- 11. 宏没有作用域，用完可以 #undef ---------- */
    #define TEMP_FLAG 1
    printf("11. 临时宏 TEMP_FLAG=%d\n", TEMP_FLAG);
    #undef TEMP_FLAG
    /* printf("%d", TEMP_FLAG);   取消注释报错：TEMP_FLAG 未定义 */

    /* ---------- 12. 哪些地方需要"编译期常量" ---------- */
    int arr[MAX_LEN];            /* ✓ 宏是常量表达式，可以当数组大小 */
    arr[0] = 1;
    printf("12. 用宏定义数组大小：int arr[MAX_LEN] 长度 = %zu\n", sizeof(arr) / sizeof(arr[0]));

    switch (GREEN) {
        case RED:
            printf("    switch 走 RED\n");
            break;
        case GREEN:                          /* ✓ 枚举常量可以当 case 标签 */
            printf("    switch 走 GREEN（枚举常量可作 case 标签）\n");
            break;
        default:
            break;
    }
    printf("    注意：const int n = 5; int a[n]; 在 C 里是变长数组，不是常量数组\n");

    /* ---------- 13. 选择建议 ---------- */
    printf("13. 选型建议：\n");
    printf("    需要类型安全 / 作用域 / 函数参数 → const\n");
    printf("    需要编译期常量（数组大小、case）→ #define 或 enum\n");
    printf("    一组相关整数（状态、颜色）      → enum\n");
    printf("    宏名一律全大写，带参宏必须加括号\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     gcc 11_常量.c -o const_demo
 *     ./const_demo
 *
 * 输出如下：
 *
 * 1. const 常量：MAX_STUDENTS=50  PI_CONST=3.14159  GRADE=A
 * 2. const 数组：31 28 31（元素同样只读）
 * 3. 宏常量：PI=3.14159  MAX_LEN=100  APP_NAME=C 语言学习
 * 4. SQUARE(3)       = 9（正确：3*3）
 *    SQUARE(1 + 2)   = 9（正确：先算 1+2，再平方）
 *    BAD_SQUARE(1+2) = 5（错误！展开成 1+2*1+2 = 5）
 *    → 宏只是文本替换，不进行任何运算，括号必须自己加
 * 5. BAD_SQUARE(1+2) 展开后是： 1 + 2 * 1 + 2
 *    因为 * 的优先级高于 +，所以算成 1 + 2 + 2 = 5
 * 6. SQUARE(i) = 9，i 仍然是 3
 * 7. enum Color: RED=0 GREEN=1 BLUE=2，当前 c=1
 *    状态码：HTTP_OK=200  HTTP_NOT_FOUND=404，当前 st=404
 *    枚举本质就是 int，可以参与比较：st == HTTP_NOT_FOUND ? 是
 * 8. *p1=10  *p2=10  *p3=10
 *    p1 改指向 y 后：*p1=20
 * 9. 向 const 参数传数组：
 *    数组内容：1 2 3 4
 * 10. 数组可以改：Hello
 *     字面量 "hello" 存在只读段，要改必须先复制到数组
 * 11. 临时宏 TEMP_FLAG=1
 * 12. 用宏定义数组大小：int arr[MAX_LEN] 长度 = 100
 *     switch 走 GREEN（枚举常量可作 case 标签）
 *     注意：const int n = 5; int a[n]; 在 C 里是变长数组，不是常量数组
 * 13. 选型建议：
 *     需要类型安全 / 作用域 / 函数参数 → const
 *     需要编译期常量（数组大小、case）→ #define 或 enum
 *     一组相关整数（状态、颜色）      → enum
 *     宏名一律全大写，带参宏必须加括号
 * ==========================================================================
 */
