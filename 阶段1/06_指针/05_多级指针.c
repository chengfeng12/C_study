/*
 * 05_多级指针.c —— 阶段1 / 第 06 课 / 第 05 个文件：多级指针
 *
 * 编译：gcc -std=c99 -Wall -Wextra 05_多级指针.c -o bin/05_多级指针
 * 运行：./bin/05_多级指针
 *
 * 本篇只用到栈上的变量，不涉及 malloc —— 动态内存留到后面章节。
 * 打印地址只打印差值，不打印具体地址。
 *
 * ====================== 一、为什么会有二级指针 ======================
 * 指针也是变量，所以它**自己也有地址**：
 *
 *      int  x  = 42;      // 一块存 int 的空间
 *      int *p  = &x;      // 一块存「地址」的空间，值为 &x
 *      int **pp = &p;     // 又一块存「地址」的空间，值为 &p
 *
 *      变量      类型       值          解引用
 *      --------  ---------  ----------  ----------------
 *      x         int        42          —
 *      p         int *      &x          *p  = 42
 *      pp        int **     &p          *pp = &x
 *                                      **pp = 42
 *
 * 内存图（地址只是示意）：
 *
 *        pp            p             x
 *      ┌──────┐     ┌──────┐     ┌──────┐
 *      │0x300 │────▶│0x200 │────▶│  42  │
 *      └──────┘     └──────┘     └──────┘
 *       0x400        0x300        0x200
 *
 * 每多一个 *，就多剥一层：`**pp` 剥两层才拿到 x 的值。
 *
 * ====================== 二、什么时候真的需要二级指针 ======================
 * 只有一种根本原因：**函数要修改「调用方的指针本身」**。
 *
 * 因为 C 只有值传递（01 篇第七节）：
 *
 *      void f(int *p) { p = NULL; }        // ✗ 改的是副本，调用方毫无变化
 *      void f(int **p) { *p = NULL; }      // ✓ 通过 * 改到调用方的指针
 *
 * 调用时必须传「指针的地址」：
 *
 *      int *sel = &a;
 *      f(&sel);                            // 注意这里是 &sel，不是 sel
 *
 * 三种典型场景：
 *
 *  (1) 让指针指向别处（上面这个）
 *  (2) out 参数：函数里「返回」一个指针（第十节）
 *  (3) 指针数组 / argv：`char **argv` 本质就是「一组字符串」
 *
 * ====================== 三、指针数组的指针就是 char ** ======================
 *      const char *words[4] = {"alpha", "beta", "gamma", "delta"};
 *      const char **pw = words;        // 数组名退化成「指向首元素」
 *
 * 拆开看：
 *
 *      words      是 const char *[4]，退化成 const char **
 *      words[i]   是 const char *，指向第 i 个字符串
 *      *words[i]  是 char，第 i 个字符串的第一个字符
 *
 * 这就是 `main(int argc, char **argv)` 的类型来源：
 * argv 是一个「指向 char * 的指针」，习惯上就当成「字符串数组」用。
 *
 * main 的四个约定（第 4 节末尾有演示）：
 *
 *      argc        参数个数，**包含程序名**，所以至少是 1
 *      argv[0]     程序名（运行时是路径），真正的参数从 argv[1] 开始
 *      argv[argc]  标准保证是 NULL，这给了「遍历到 NULL 为止」的写法
 *      判断有没有参数：用 argc > 1，别去猜 argv[1] 是不是空
 *
 * ====================== 四、二维数组 ≠ int **（最容易错的一节） ======================
 * （这是 02 篇第六节的延伸：那边讲「二维数组退化成行指针」，
 *   这边讲为什么它**不能**换成 int **。）
 *
 *      int m[2][3];        // 一整块连续内存，24 字节，没有指针表
 *      int *rows[2];       // 真的是一张「指针表」，2 个指针
 *
 * 两者的区别在于**内存布局**：
 *
 *      m：      [1 2 3 | 4 5 6]        一整块，m 退化成 int (*)[3]（行指针）
 *      rows：   [ ●   ● ]              指针表
 *                │   └──▶ 另一块内存
 *                └──────▶ 某块内存
 *
 * 所以：
 *
 *      int **p = m;        // ✗ 类型就错了：int (*)[3] 和 int ** 不兼容
 *      int **p = rows;     // ✓ rows 是 int *[2]，退化成 int *
 *
 * 如果硬用 `(int **)m` 强转，会把第一行开头的两个 int（1 和 2）
 * 当成两个指针去解引用，读到非法地址。这是"能编过、一跑就崩"的经典陷阱。
 *
 * ====================== 五、多级指针与 const ======================
 * const 修饰的是「它左边紧挨着的那个 *」所指向的东西：
 *
 *      const char **p;              // 指向 const char *：**p 不能改，*p 可以改
 *      char *const *p;              // 指向 char * const：*p 不能改，**p 可以改
 *      char **const p;              // p 本身是 const 指针
 *      const char *const *p;        // 两层都只读（函数参数里最常见）
 *
 * 实用建议：函数接收「只读的字符串数组」时写 `const char *const *argv`，
 * 语义最完整 —— 既不改字符串内容，也不改数组里的指针。
 *
 * ⚠ C 的 const 在多层指针上的检查比 C++ 宽松，`char **` 传给
 * `const char **` 编译器只给警告。这不是"能用"，而是"小心地用"。
 *
 * ====================== 六、三级指针 ======================
 *      int ***ppp = &pp;      // 指向 int **
 *      ***ppp == x
 *
 * 语法上可以无限套，但实际上**三级以上基本是设计问题**。
 * 真需要「改指针的指针的指针」时，通常应该改成一个结构体：
 *
 *      struct Result { int *data; size_t len; };
 *      void f(struct Result *out);      // 比 int *** 清楚一百倍
 *
 * ====================== 七、常见坑 ======================
 *  1. 改指针却传了一级指针          f(sel) 应该是 f(&sel)，编译器报类型不兼容
 *  2. 用 int ** 接二维数组           类型不对，强转后运行期读到非法地址
 *  3. 混淆 int *a[3] 与 int **a      前者是数组对象，后者只是个指针
 *  4. 解引用层数数错                 少一层拿到的是地址，打印出来像乱码
 *  5. const char ** 接 char **       C 里不报错只警告，但确实违反 const 契约
 *  6. out 参数忘了先置 NULL          函数某条分支没赋值，调用方拿到垃圾
 *  7. 二级指针没初始化就 *p = ...     写飞到一个随机地址
 *  8. 以为三级指针更"高级"            能用一级就别用二级，可读性优先
 *
 * ====================== 八、实践建议 ======================
 *  - 只为了"函数里改指针"才用二级指针，其余场景先想别的设计
 *  - out 参数一律在函数开头置 NULL（`*out = NULL;`），失败时调用方好判断
 *  - 指针数组传参写 `const char *const *`，语义最清楚
 *  - 二维数组就老老实实用 `int (*)[COLS]`，别绕 int **
 *  - 出现 `***` 就该停下来重构：换结构体、换返回值、换设计
 */

#include <stdio.h>

/* 通过二级指针，修改「调用方的指针本身」 */
static void point_to(int **out, int *target)
{
    *out = target;
}

/* out 参数：把找到的位置「返回」出去，找不到就给 NULL */
static void find_first_even(const int *a, int n, const int **out)
{
    *out = NULL;                         /* ★ 先置空，避免走空分支时留下垃圾 */
    for (int i = 0; i < n; i++) {
        if (a[i] % 2 == 0) {
            *out = &a[i];
            return;
        }
    }
}

int main(void)
{
    /* ---------- 1. 一层层拆开看 ---------- */
    printf("1. 二级、三级指针的拆解\n");
    int x = 42;
    int *p = &x;
    int **pp = &p;
    int ***ppp = &pp;
    printf("   x = %d，p 指向它，pp 指向 p，ppp 指向 pp\n", x);
    printf("   *p     = %d（剥 1 层）\n", *p);
    printf("   **pp   = %d（剥 2 层）\n", **pp);
    printf("   ***ppp = %d（剥 3 层）\n", ***ppp);
    printf("   p 与 &x 的地址差 = %ld，pp 与 &p 的地址差 = %ld\n",
           (long)((char *)p - (char *)&x), (long)((char *)pp - (char *)&p));
    printf("   差都是 0：指针里存的就是对方的地址\n");
    printf("   sizeof(int *) = %zu，sizeof(int **) = %zu，sizeof(int ***) = %zu\n",
           sizeof(int *), sizeof(int **), sizeof(int ***));
    printf("   不管几级，都只是一个地址那么大\n");

    /* ---------- 2. 修改指针本身必须传二级指针 ---------- */
    printf("2. 想让函数改「指针本身」，就要传 &指针\n");
    int a = 1, b = 2;
    int *sel = &a;
    printf("   初始：sel 指向 a，*sel = %d\n", *sel);
    point_to(&sel, &b);                  /* ★ 传的是 &sel */
    printf("   point_to(&sel, &b) 之后 *sel = %d（b 没变，变的是 sel 的指向）\n", *sel);
    printf("   如果错写成 point_to(sel, &b)，传进去的是 int *，类型不兼容\n");
    printf("   记住：函数要改谁，就传谁的地址 —— 要改指针，就传指针的地址\n");

    /* ---------- 3. out 参数：把指针「返回」出来 ---------- */
    printf("3. out 参数：函数把一个位置指针交给调用方\n");
    int data[6] = {1, 3, 8, 5, 2, 4};
    const int *hit;
    find_first_even(data, 6, &hit);       /* 传 &hit，函数里写 *out */
    if (hit != NULL)
        printf("   第一个偶数 = %d，偏移 %ld（hit - data 就是下标）\n",
               *hit, (long)(hit - data));
    int odds[3] = {1, 3, 5};
    find_first_even(odds, 3, &hit);
    printf("   全是奇数时 hit == NULL：%s（所以函数开头先置空很重要）\n",
           (hit == NULL) ? "是" : "否");

    /* ---------- 4. 指针数组 → char ** ---------- */
    printf("4. 指针数组退化成 char **\n");
    const char *words[4] = {"alpha", "beta", "gamma", "delta"};
    const char **pw = words;             /* 数组名退化成指向首元素（首元素是 char *） */
    for (int i = 0; i < 4; i++)
        printf("   pw[%d] = \"%s\"   *(pw + %d) = \"%s\"   首字母 = %c\n",
               i, pw[i], i, *(pw + i), *pw[i]);
    printf("   sizeof(words) = %zu（4 个指针），sizeof(*pw) = %zu（1 个指针）\n",
           sizeof(words), sizeof(*pw));
    printf("   main 的 char **argv 就是这个东西：一组字符串\n");

    /* ---- 扩展：main 的 argc / argv 实战（用本地数组模拟命令行） ---- */
    char *args[] = {"prog", "add", "1", "2", NULL};  /* 真实 argv 末尾也一定是 NULL */
    int argc = 0;
    while (args[argc] != NULL)           /* 靠结尾的 NULL 数出个数 */
        argc++;
    printf("   模拟命令行：prog add 1 2\n");
    printf("   argc = %d（不数结尾那个 NULL）\n", argc);
    for (int i = 0; i < argc; i++)
        printf("   argv[%d] = \"%s\"\n", i, args[i]);
    printf("   标准保证 argv[argc] == NULL，所以也能写成 while (argv[i] != NULL)\n");
    printf("   argv[0] 是程序名（运行时是路径），真正的参数从 argv[1] 开始\n");
    printf("   判断有没有参数要用 argc > 1，别去猜 argv[1] 是不是空\n");

    /* ---------- 5. 二维数组不是 int ** ---------- */
    printf("5. int m[2][3] 不能赋给 int **，int *rows[2] 可以\n");
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int *rows[2] = {m[0], m[1]};         /* 一张真正的指针表 */
    int **pp2 = rows;                    /* ✓ rows 退化成 int * → int ** */
    printf("   m 是一整块 %zu 字节的连续内存，没有指针表\n", sizeof(m));
    printf("   rows 是 %zu 字节的指针表，指向两行\n", sizeof(rows));
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            printf("   pp2[%d][%d] = %d\n", i, j, pp2[i][j]);
    printf("   注意 m[0] 到 m[1] 相隔 %ld 字节，rows[0] 与 rows[1] 指向的位置也相隔 %ld 字节，\n",
           (long)((char *)m[1] - (char *)m[0]),
           (long)((char *)rows[1] - (char *)rows[0]));
    printf("   但 rows 额外多占了 %zu 字节的指针表 —— 这正是 int ** 要求的东西\n", sizeof(rows));
    printf("   所以 (int **)m 强转后会把 1、2 当成地址解引用，必崩（本程序不跑）\n");

    /* ---------- 6. const 与多级指针 ---------- */
    printf("6. const 在二级指针里的位置\n");
    const char *s1 = "red";
    const char *s2 = "green";
    const char *const arr[2] = {s1, s2}; /* 指针本身也不能改 */
    const char *const *pc = arr;         /* 两层都只读，函数参数的标准写法 */
    for (int i = 0; i < 2; i++)
        printf("   pc[%d] = \"%s\"\n", i, pc[i]);
    /* pc[0] = s1;     ✗ 元素是 const 指针，不能改 */
    /* *pc[0] = 'R';   ✗ 指向的字符也是 const，不能改 */
    printf("   读法：const 修饰它左边紧挨着的那个 * 所指向的东西\n");
    printf("   函数收只读字符串数组时写 const char *const * 最清楚\n");

    /* ---------- 7. 三级指针与设计感 ---------- */
    printf("7. 三级指针能写，但基本不该写\n");
    printf("   ***ppp = %d 语法没问题，可读性已经崩了\n", ***ppp);
    printf("   出现 *** 通常意味着该换结构体：\n");
    printf("   struct Result { int *data; size_t len; };  void f(struct Result *out);\n");
    printf("   一个 out 参数 + 结构体，比 int *** 清楚得多\n");

    /* ---------- 8. 小结 ---------- */
    printf("8. 小结：\n");
    printf("    每多一个 * 就多剥一层，**pp 的值才是最终数据\n");
    printf("    需要改「指针本身」时传二级指针：f(&p)\n");
    printf("    out 参数在函数开头先置 NULL，调用方才好判断成败\n");
    printf("    指针数组退化成 char **，这就是 argv 的类型\n");
    printf("    二维数组退化成 int (*)[列数]，int ** 需要真的存在指针表\n");
    printf("    三级以上是设计信号：换结构体，别硬套\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 05_多级指针.c -o bin/05_多级指针
 *     ./bin/05_多级指针
 *
 * 输出如下：
 *
 * 1. 二级、三级指针的拆解
 *    x = 42，p 指向它，pp 指向 p，ppp 指向 pp
 *    *p     = 42（剥 1 层）
 *    **pp   = 42（剥 2 层）
 *    ***ppp = 42（剥 3 层）
 *    p 与 &x 的地址差 = 0，pp 与 &p 的地址差 = 0
 *    差都是 0：指针里存的就是对方的地址
 *    sizeof(int *) = 8，sizeof(int **) = 8，sizeof(int ***) = 8
 *    不管几级，都只是一个地址那么大
 * 2. 想让函数改「指针本身」，就要传 &指针
 *    初始：sel 指向 a，*sel = 1
 *    point_to(&sel, &b) 之后 *sel = 2（b 没变，变的是 sel 的指向）
 *    如果错写成 point_to(sel, &b)，传进去的是 int *，类型不兼容
 *    记住：函数要改谁，就传谁的地址 —— 要改指针，就传指针的地址
 * 3. out 参数：函数把一个位置指针交给调用方
 *    第一个偶数 = 8，偏移 2（hit - data 就是下标）
 *    全是奇数时 hit == NULL：是（所以函数开头先置空很重要）
 * 4. 指针数组退化成 char **
 *    pw[0] = "alpha"   *(pw + 0) = "alpha"   首字母 = a
 *    pw[1] = "beta"   *(pw + 1) = "beta"   首字母 = b
 *    pw[2] = "gamma"   *(pw + 2) = "gamma"   首字母 = g
 *    pw[3] = "delta"   *(pw + 3) = "delta"   首字母 = d
 *    sizeof(words) = 32（4 个指针），sizeof(*pw) = 8（1 个指针）
 *    main 的 char **argv 就是这个东西：一组字符串
 *    模拟命令行：prog add 1 2
 *    argc = 4（不数结尾那个 NULL）
 *    argv[0] = "prog"
 *    argv[1] = "add"
 *    argv[2] = "1"
 *    argv[3] = "2"
 *    标准保证 argv[argc] == NULL，所以也能写成 while (argv[i] != NULL)
 *    argv[0] 是程序名（运行时是路径），真正的参数从 argv[1] 开始
 *    判断有没有参数要用 argc > 1，别去猜 argv[1] 是不是空
 * 5. int m[2][3] 不能赋给 int **，int *rows[2] 可以
 *    m 是一整块 24 字节的连续内存，没有指针表
 *    rows 是 16 字节的指针表，指向两行
 *    pp2[0][0] = 1
 *    pp2[0][1] = 2
 *    pp2[0][2] = 3
 *    pp2[1][0] = 4
 *    pp2[1][1] = 5
 *    pp2[1][2] = 6
 *    注意 m[0] 到 m[1] 相隔 12 字节，rows[0] 与 rows[1] 指向的位置也相隔 12 字节，
 *    但 rows 额外多占了 16 字节的指针表 —— 这正是 int ** 要求的东西
 *    所以 (int **)m 强转后会把 1、2 当成地址解引用，必崩（本程序不跑）
 * 6. const 在二级指针里的位置
 *    pc[0] = "red"
 *    pc[1] = "green"
 *    读法：const 修饰它左边紧挨着的那个 * 所指向的东西
 *    函数收只读字符串数组时写 const char *const * 最清楚
 * 7. 三级指针能写，但基本不该写
 *    ***ppp = 42 语法没问题，可读性已经崩了
 *    出现 *** 通常意味着该换结构体：
 *    struct Result { int *data; size_t len; };  void f(struct Result *out);
 *    一个 out 参数 + 结构体，比 int *** 清楚得多
 * 8. 小结：
 *     每多一个 * 就多剥一层，**pp 的值才是最终数据
 *     需要改「指针本身」时传二级指针：f(&p)
 *     out 参数在函数开头先置 NULL，调用方才好判断成败
 *     指针数组退化成 char **，这就是 argv 的类型
 *     二维数组退化成 int (*)[列数]，int ** 需要真的存在指针表
 *     三级以上是设计信号：换结构体，别硬套
 *
 * 几个容易忽略的对照点：
 *   第 1 条：三行"地址差 = 0"，说明 p 里存的就是 &x、pp 里存的就是 &p，
 *           指针并没有"额外记录什么"；三级指针的 sizeof 依然是 8，
 *           因为***几级都是"一个地址的长度***。
 *   第 2 条：point_to(&sel, &b) 里两个实参都是"地址"：
 *           第一个是"指针的地址"（int **），第二个是"int 的地址"（int *）。
 *           函数里写 *out = target，改的是 sel 本身，a/b 的值一个都没动。
 *   第 3 条：hit - data 得 2，说明 out 参数带出来的是"数组成员内部的地址"，
 *           指针减法自动换算成元素下标，不需要手动除以 sizeof。
 *   第 5 条：两边的"行间距"都是 12 字节，看起来一样 —— 差别不在行内布局，
 *           而在于 rows 需要一张真实的指针表（16 字节），m 不需要。
 *           这也是为什么 int ** 能遍历 rows、却不能遍历 m。
 *   第 6 条：`const char *const *` 有三层含义（char 只读、指针只读、外层可改），
 *           写全了接口意图最清楚，也比 `char **` 少一堆隐式转换警告。
 * ==========================================================================
 */
