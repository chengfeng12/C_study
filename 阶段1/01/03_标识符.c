/*
 * 03_标识符.c —— 阶段1 / 第 01 课 / 第 3 个文件：C 语言标识符（命名规则）
 *
 * 编译：gcc 03_标识符.c -o ident_demo
 * 运行：./ident_demo
 *
 * ============================ 一、什么是标识符 ============================
 * 标识符（identifier）就是你在代码里给"东西"起的名字，比如：
 *     变量名    int age;
 *     函数名    int main(void) { ... }
 *     结构体名  struct Student { ... };
 *     宏名      #define MAX_SIZE 100
 *     标签名    goto end;
 * 起名字不是想怎么写就怎么写，C 语言有一套"硬规则"（编译器强制）
 * 和一套"软规则"（团队约定，不遵守不会报错，但会被同事骂）。
 *
 * ======================= 二、硬规则（编译器强制检查）=======================
 * 1) 只能用三类字符：英文字母 a-z A-Z、数字 0-9、下划线 _
 * 2) 第一个字符不能是数字，只能是字母或下划线
 * 3) 不能是关键字（int、return、for 等，见第三节）
 * 4) 区分大小写：age、Age、AGE 是三个完全不同的标识符
 * 5) 不能包含空格、连字符、运算符等任何其它字符
 *
 * 合法：age  _count  MAX  sum2  total_price  student1  a1b2c3
 * 非法：2sum     （数字开头）
 *        my-age   （含减号，会被当成  my - age  两个标识符相减）
 *        my age   （含空格）
 *        int      （关键字）
 *        total$   （含 $，虽有个别编译器放宽，但 C 标准不允许）
 *        a.b      （含点号，点号是成员访问运算符）
 *
 * ===================== 三、关键字：这些名字已被占用 =====================
 * C89 共 32 个关键字，C99 增加 5 个，C11 再增加几个。常用的有：
 *   类型：   int char float double void short long signed unsigned
 *            struct union enum
 *   存储：   auto static extern register typedef
 *   限定：   const volatile inline restrict
 *   语句：   if else switch case default for while do break continue
 *            return goto sizeof
 *   计算：   _Bool _Complex _Imaginary（C99）
 * 关键字一律小写。注意：Int、INT、Return 不是关键字，可以当标识符，
 * 但千万别这么干——读代码的人会以为自己在看关键字，属于自找麻烦。
 *
 * ================== 四、保留标识符：下划线开头的坑 ==================
 * C 标准把一部分名字"保留给编译器和标准库"，你自己定义会冲突（未定义行为）：
 *   - 以两个下划线开头，或下划线 + 大写字母：__foo、_Foo  → 任何场合都别用
 *   - 以一个下划线开头：_foo → 只在全局（文件作用域）被保留；函数内部的局部变量勉强可用，
 *     但为了不踩雷，统一建议：普通变量不要以下划线开头
 *   - 标准库已经占用的名字：printf、malloc、size_t、errno 等，不要自己重复定义
 * 还有一类"后缀保留"：POSIX 保留了 _t 结尾的类型名（size_t、time_t），
 * 自己定义类型时别用 xxx_t，容易和系统头文件打架。
 *
 * ======================= 五、长度限制与可移植性 =======================
 * C89 只保证前 31 个字符有意义，C99 提高到 63 个（外部链接的标识符 31 个）。
 * 现代编译器基本不截断，但起超长名字既没必要也伤可读性。
 * 另一个隐患：外部链接的名字（全局函数/变量）在链接时可能不区分大小写
 * （某些老式链接器），所以 myFunc 和 myfunc 不要同时存在。
 *
 * ======================== 六、软规则：命名风格 ========================
 * 硬规则只保证"能编译"，软规则保证"人能看懂"。业界常见三派：
 *   (a) 下划线派（Linux/内核风格）  student_count、max_size
 *   (b) 小驼峰（Java/JS 习惯）      studentCount、maxSize
 *   (c) 匈牙利/前缀派（Windows API） nCount、pBuffer、strName
 * 通用约定（比风格更重要）：
 *   - 变量/函数：小写开头，见名知义，别用 a、b、x1、tmp 这种
 *   - 宏和常量：#define MAX_SIZE 100 全大写 + 下划线
 *   - 类型名：struct Student、typedef ... Student（首字母大写区分类型与变量）
 *   - 一个项目里只用一种风格，不要混着来
 *   - 名字长度和"作用域大小"成正比：循环变量 i、j 可以很短，
 *     全局变量必须写清楚
 *
 * ======================== 七、重名与遮蔽（作用域）=======================
 * 同一个名字能不能重复用，取决于"作用域"：
 *   - 同一作用域内不能重复定义         int a; int a;   // 报错
 *   - 内层可以定义同名变量，会"遮蔽"外层（合法，但极易出 bug）
 *   - 全局变量与局部变量同名时，局部优先
 * 下面代码第 8 节会演示遮蔽，注意看注释里的说明。
 *
 * ====================== 八、中文标识符到底能不能用 ======================
 * 结论：能编译，但不要用。
 *   - C99 起标准允许"扩展字符集"（Universal Character Names）作为标识符，
 *     前提是编译器支持（gcc/clang 对 UTF-8 源码基本都支持）
 *   - 但可移植性极差：换个编译器、换个编码（GBK/UTF-8）、换个编辑器就可能报错
 *   - 调试器、日志、脚本工具对中文标识符的支持参差不齐
 *   - 所以：中文只适合出现在**注释**和**字符串**里，不要出现在标识符里
 * 想验证的话，把下面这两行的注释去掉再编译（gcc/clang 能过，但不代表别的编译器能过）：
 *     int 年龄 = 20;
 *     printf("%d\n", 年龄);
 * 反过来，文件名用中文是完全没问题的（本文件就叫 03_标识符.c），
 * 因为文件名是操作系统的事，不参与 C 语言的语法检查。
 */

#include <stdio.h>

/* 全大写的宏名：这是约定，不是语法要求，但全世界都这么写 */
#define MAX_SIZE 100

/* 类型名首字母大写，用来和变量名区分开 */
struct Student {
    int  id;
    char name[20];
};

int main(void)
{
    /* ---------- 1. 大小写敏感：三个名字互不相干 ---------- */
    int age = 18;
    int Age = 20;
    int AGE = 22;

    printf("1. age=%d Age=%d AGE=%d（三个不同的变量）\n", age, Age, AGE);

    /* ---------- 2. 下划线开头：局部变量虽然可用，但不推荐 ---------- */
    /* 局部作用域里 _tmp 不冲突，但养成习惯后早晚在全局踩雷，所以别这么写 */
    int total_price = 99;
    int student_count = 3;
    printf("2. total_price=%d student_count=%d MAX_SIZE=%d\n",
           total_price, student_count, MAX_SIZE);

    /* ---------- 3. 名字要见名知义 ---------- */
    /* 对比一下：a、b、c 与 width、height、area 的可读性差距 */
    double width = 3.0, height = 4.0;
    double area  = width * height;
    printf("3. width=%.1f height=%.1f area=%.1f\n", width, height, area);

    /* ---------- 4. 作用域：块内定义的名字，块外就失效 ---------- */
    {
        int inner = 1;      /* 只在这个花括号内有效 */
        printf("4. 块内可以访问 inner=%d\n", inner);
    }
    /* printf("%d\n", inner);   取消注释会编译报错：inner 未声明 */

    /* ---------- 5. 遮蔽（shadowing）：内层同名变量盖住外层 ---------- */
    int value = 100;
    printf("5. 外层 value=%d\n", value);
    {
        int value = 200;    /* 新变量，遮蔽了外层的 value，合法但危险 */
        printf("   内层 value=%d（遮蔽生效）\n", value);
    }
    printf("   回到外层 value=%d（外层的值没被改）\n", value);

    /* ---------- 6. 结构体成员名和变量名不冲突 ---------- */
    /* id 既是结构体成员名，也可以当普通变量名，因为成员的"作用域"在结构体内部 */
    int id = 7;
    struct Student s = {1, "小明"};
    printf("6. 变量 id=%d，成员 s.id=%d\n", id, s.id);

    /* ---------- 7. sizeof 是关键字，不能当变量名 ---------- */
    /* int sizeof = 1;  取消注释会报错：expected identifier */
    printf("7. sizeof(int)=%zu（sizeof 是关键字，不能占用）\n", sizeof(int));

    /* ---------- 8. 命名不规范但能编译的例子（反面教材）---------- */
    /* 下面这些都能编译，但都属于"能跑别学"： */
    int I = 1, l = 2, O = 0;   /* I、l、O 极易和 1、1、0 混淆 */
    printf("8. 反例 I=%d l=%d O=%d（肉眼看不出是字母还是数字）\n", I, l, O);

    return 0;
}
