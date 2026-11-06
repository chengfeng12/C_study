/*
 * 01_结构体基础.c —— 阶段2 / 第 02 课 / 第 01 个文件：结构体的声明、定义与使用
 *
 * 编译：gcc -std=c99 -Wall -Wextra 01_结构体基础.c -o bin/01_结构体基础
 * 运行：./bin/01_结构体基础
 *
 * 阶段1 第 06 课第 06 篇讲的是"指针与结构体"（->、传参、链表、浅拷贝），
 * 它默认你已经会写结构体。这一篇补上那个前提——最基础的三件事：
 *
 *      声明：struct Point { int x; int y; };     描述一种"形状"
 *      定义：struct Point p;                     真正分配一块内存
 *      使用：p.x、p = q、传参、返回、放进数组
 *
 * ====================== 一、为什么需要结构体 ======================
 * 没有结构体时，描述"一个学生"只能靠几组平行数组：
 *
 *      int   ids[3]   = { 1, 2, 3 };
 *      char  names[3][16] = { "alice", "bob", "carol" };
 *      float scores[3] = { 90.5f, 78.0f, 88.5f };
 *
 * 三个问题：
 *
 *  1. **关系靠下标维持**：ids[1]、names[1]、scores[1] 必须是同一个人。
 *     删掉一个学生、或者按分数排序，所有数组都得同步搬——漏一个就全错。
 *
 *  2. **函数没法"返回一个学生"**：只能返回一个值，其余用出参带出来。
 *
 *  3. **看不出哪些数据是一伙的**：读代码的人要自己脑补对应关系。
 *
 * 结构体把"一伙的数据"打包成一个整体：
 *
 *      struct Student {
 *          int   id;
 *          char  name[16];
 *          float score;
 *      };
 *
 * 从此 `struct Student` 就是一个**新的类型**，像 int 一样能定义变量、
 * 能赋值、能传参、能返回、能放进数组。
 *
 * ====================== 二、声明：只描述形状，不分配内存 ======================
 *
 *      struct Point {
 *          int x;
 *          int y;
 *      };                      // ← 这个分号不能少
 *
 * 三条要点：
 *
 *  (1) **声明不是定义变量**。上面这段只是告诉编译器"有这个形状"，
 *      一个字节都没分配。`sizeof(struct Point)` 能算，但没有变量存在。
 *
 *  (2) **`struct Point` 是一个整体类型名**，不能拆开写 `Point`（除非 typedef）。
 *      `struct` 关键字在 C 里不能省，这是 C 和 C++ 最明显的差别之一。
 *
 *  (3) **标签（tag）在独立的命名空间**。下面两行可以同时存在，互不冲突：
 *          struct Point { int x, y; };
 *          int Point = 10;              // 普通标识符 Point
 *      但 `struct Point p;` 里的 Point 指的是标签，与变量 Point 无关。
 *
 * ★ 成员之间用分号分隔，最后一个成员后面也要有分号：
 *      struct Bad { int x; int y }     // ✗ 少分号，报错位置可能很靠后
 *   漏分号时编译器往往在下一行才报错（"expected ';' at end of declaration list"），
 *   看到这类错误先回头检查上一行的结构体声明。
 *
 * ★ 不完整类型（前置声明）：只写 `struct Node;` 不写成员，叫不完整类型。
 *   它能用来声明指针（指针大小固定），但不能定义变量、不能取成员：
 *      struct Node;                     // 不完整类型
 *      struct Node *p;                  // ✔ 指针可以
 *      struct Node  n;                  // ✗ 大小未知
 *   自引用结构体（链表节点）就靠这一条：结构体内部只能放**自己的指针**。
 *
 * ====================== 三、定义：从"形状"到"内存" ======================
 * (1) **先声明，再定义变量**
 *          struct Point { int x, y; };
 *          struct Point p;              // 栈上一块内存，成员值不确定
 *
 * (2) **声明的同时定义变量**（少用，可读性差）
 *          struct Point { int x, y; } p1, p2;
 *
 * (3) **typedef 起别名**（工程里最常用）
 *          typedef struct Point { int x, y; } Point;
 *          Point p;                     // 不用再写 struct
 *      注意别名和标签可以同名，两者在不同命名空间，不冲突。
 *
 * (4) **匿名结构体 + typedef**（一次性类型，没有标签）
 *          typedef struct { int id; char name[16]; } User;
 *          User u;
 *      代价：**没法自引用**。因为没有标签，结构体内部无法指回自己：
 *          typedef struct { User *next; } User;    // ✗ User 此时还不存在
 *      要写链表节点，必须给标签：`typedef struct Node { struct Node *next; } Node;`
 *
 * (5) **定义指针 / 数组 / const 变量**
 *          Point  *pp = &p;             // 指向结构体的指针（4 或 8 字节）
 *          Point   pts[3];              // 结构体数组：3 个元素挨着排
 *          const Point  cp = { 1, 2 };  // 只读结构体：成员不能再改
 *          const Point *pcp = &cp;      // 指向只读结构体的指针
 *      `const Point *` 和 `Point *const` 不是一回事：
 *          前者"指向的内容只读"，后者"指针本身的指向不能改"。
 *
 * ★ 定义出来的变量，成员是**不确定值**（和局部 int 一样，不是 0）：
 *      struct Point p;              // p.x、p.y 是垃圾值
 *      struct Point q = { 0, 0 };   // ✔ 显式清零
 *   别指望"结构体默认是 0"，那是 Java/C# 的规则，不是 C 的。
 *
 * ====================== 四、初始化与赋值：先分清这两件事 ======================
 * 一句话区别：
 *      初始化  定义变量的**同时**给初值：`Point p = { 1, 2 };`
 *      赋值    变量**已经存在**，用新值覆盖：`p = (Point){ 5, 6 };`
 * 前者只能出现在定义处，后者可以出现在任何语句里。
 *
 * ---- 初始化的 5 种写法 ----
 *      struct Point a = { 3, 4 };            // ① 按成员顺序
 *      struct Point b = { .y = 4, .x = 3 };  // ② C99 指定初始化器，顺序自由、可跳过
 *      struct Point c = { 3, 0 };            // ③ 只给第一个也要补全，写 { 3 } 会被 -Wextra 警告
 *      struct Point d = { 0 };               // ④ 全部清零的惯用法
 *      struct Rect  r = { .lt = { 1, 2 },    // ⑤ 嵌套结构体：逐层指定
 *                         .rb = { 5, 6 } };
 *   嵌套的完整讨论（声明顺序、内存布局、深浅拷贝）见 02 篇。
 *   另外 `Point e = a;` 是**拷贝初始化**：用同类型对象的值初始化新对象，
 *   本质和整体赋值一样，都是逐成员拷贝。
 *
 * 指定初始化器（designated initializer）的三个好处：
 *  - 成员多的时候不用数位置
 *  - 结构体加成员、调顺序时，初始化语句不会悄悄错位
 *  - 只初始化关心的几个成员，其余自动为 0
 *
 * ★ 数组也能用同样的写法：`int a[5] = { [2] = 9 };`
 *
 * ---- 赋值的 6 种写法 ----
 * (1) **逐成员赋值**（最原始，也最明确）
 *          p.x = 1;
 *          p.y = 2;
 *
 * (2) **整体赋值**（同类型之间，逐成员拷贝）
 *          q = p;
 *   结构体是 C 里**唯一能整体赋值的聚合类型**（数组不行，只能逐元素或 memcpy）。
 *   注意是"拷贝"：q 和 p 是两块内存，改 q 不影响 p。
 *
 * (3) **复合字面量赋值**（C99）
 *          p = (Point){ 5, 6 };                    // 现场造一个匿名对象再拷过去
 *          print_point("临时点", (Point){ 7, 8 }); // 直接当实参，不必先定义变量
 *   复合字面量就是"没有名字的结构体常量"，生命周期到所在代码块结束。
 *
 * (4) **函数返回值赋值**
 *          p = make_point(1, 2);
 *   返回值本身是一份拷贝，赋给 p 之后就是 p 自己的值。
 *
 * (5) **memcpy 按字节拷贝**
 *          memcpy(&q, &p, sizeof q);       // 需要 <string.h>
 *   同类型时效果和 `q = p` 一样，但 memcpy 不看类型，写错了编译器不拦。
 *   整体赋值能做的事，就不要用 memcpy。
 *
 * (6) **memset 清零**
 *          memset(&p, 0, sizeof p);
 *   连填充字节一起写成 0。只对"数值/字符"成员安全：
 *   指针成员会被填成全 0 位模式，主流平台上等于 NULL，但标准并不保证。
 *
 * ★ 不能整体赋值的情况：**类型不同**。哪怕成员一模一样也不行：
 *      struct A { int x; };  struct B { int x; };
 *      struct A a;  struct B b;
 *      a = b;                  // ✗ 编译错误：不兼容的类型
 *
 * ---- 整体比较是非法的 ----
 *      if (p == q) { }          // ✗ 编译错误：结构体不能用 ==
 * 必须逐成员比。为什么 C 不给结构体定义 == ？因为填充字节（见第六节）
 * 的内容不确定，逐字节比较可能对"逻辑上相等"的两个结构体返回不等。
 * 标准干脆不提供，逼你自己写比较函数，明确"什么算相等"。
 * 用 memcmp 也有同样风险：填充字节不同就会误判不等。
 *
 * ====================== 五、使用：七种常见场合 ======================
 * (1) **`.` 访问成员**
 *          p.x = 3;
 *          int s = p.x + p.y;
 *
 * (2) **`->` 通过指针访问**（等价于 `(*pp).x`）
 *          Point *pp = &p;
 *          (*pp).x     先解引用再取成员，括号不能省
 *          pp->x       同上，语法糖
 *      `*pp.x` 是错的：`.` 的优先级高于 `*`，会被解析成 `*(pp.x)`。
 *
 * (3) **嵌套访问**：一层一层点下去
 *          r.lt.x          嵌套结构体的成员
 *          pr->lt.y        指针 + 嵌套：先 -> 进 Rect，再 . 取 Point，再 . 取 y
 *
 * (4) **整体传参 / 指针传参**
 *          void print(struct Point p);            // 值传递：整个结构体拷一份
 *          void show(const struct Point *p);      // 指针传递：只拷地址，const 承诺不改
 *      小结构体（几个 int）值传递很自然；大结构体传 `const T *` 避免拷贝。
 *
 * (5) **作返回值**
 *          struct Point make(int x, int y);       // 值返回：拷一份给调用方
 *
 * (6) **放进数组 / 遍历**
 *          struct Point pts[3] = { {1, 1}, {2, 2}, {3, 3} };
 *          for (int i = 0; i < 3; i++) pts[i].x += 10;
 *      `pts[i].x` 里 [] 优先级最高：先取元素，再取成员。
 *      结构体数组的完整用法（长度、传参、排序）见 04 篇。
 *
 * (7) **作复合字面量实参**（配合赋值第 (3) 种，省掉临时变量）
 *          print_point("临时点", (Point){ 7, 8 });
 *
 * ====================== 六、sizeof 与填充：为什么大小对不上 ======================
 * （对齐的三条规则、位域/柔性数组、赋值成本与浅拷贝，见 03 篇。）
 *      struct Mixed { char c; int i; char d; };
 *
 * 1 + 4 + 1 = 6？多数平台上 `sizeof(struct Mixed)` 是 **12**。
 *
 * 原因：**对齐（alignment）**。int 要求起始地址是 4 的倍数，于是编译器插入填充：
 *
 *      偏移 0   c        1 字节
 *      偏移 1-3 填充     3 字节
 *      偏移 4   i        4 字节
 *      偏移 8   d        1 字节
 *      偏移 9-11 填充    3 字节（让整个结构体大小是最大对齐的整数倍）
 *
 * 换一下成员顺序就能省：
 *      struct Tight { int i; char c; char d; };    // 通常 8 字节
 *
 * 结论：
 *  - `sizeof(结构体)` ≥ 成员大小之和，多出来的是填充
 *  - **成员顺序影响大小**，把大的放前面通常更省
 *  - 填充字节的内容是**不确定的**，所以：不能用 memcmp 比结构体、
 *    不能用结构体的字节直接算哈希、不能 fwrite 到文件后跨平台读
 *  - `offsetof(struct Mixed, i)` 能看到成员的真实偏移（需要 <stddef.h>）
 *
 * ====================== 七、常见坑 ======================
 *  1. 结构体声明末尾漏分号          报错常出现在下一行，容易找错地方
 *  2. 用 `Point` 却忘了 typedef      C 里必须写 `struct Point`
 *  3. 以为成员默认是 0               局部变量是垃圾值，务必显式初始化
 *  4. 用 == 比较两个结构体           编译不过；逐成员比，或写专门的比较函数
 *  5. 用 memcmp 比结构体             填充字节不确定，可能误判不等
 *  6. 大结构体值传递                  每次调用都拷一份，改用 `const T *`
 *  7. 匿名结构体里想自引用            没有标签就指不回自己，必须给 tag
 *  8. 以为 sizeof 等于成员之和        对齐填充会让它变大，且随平台变化
 *  9. 结构体里放 `char *` 就问深浅拷贝  整体赋值只拷指针，源释放后就是悬空指针
 *     （详见阶段1 第 06 课第 06 篇第五节）
 * 10. 在头文件里定义变量而非声明      多个 .c 包含就重复定义；头文件里只放类型和 extern 声明
 *
 * ====================== 八、实践建议 ======================
 *  - 一律 `typedef struct Xxx { ... } Xxx;`，用的时候不用写 struct
 *  - 定义后立刻初始化：`Xxx v = { 0 };` 是最便宜的保险
 *  - 成员顺序从大到小排，能减少填充（除非可读性更重要）
 *  - 写一个 `xxx_equal(a, b)` 显式定义"相等"，别用 memcmp
 *  - 超过 2~3 个机器字的结构体，传参用 `const Xxx *`
 *  - 成员里有指针时，先想清楚：谁分配、谁释放、赋值是拷贝还是共享
 *  - 结构体类型定义放头文件，变量定义放 .c 文件
 */

#include <stdio.h>
#include <stddef.h>      /* offsetof */
#include <string.h>      /* memcpy / memset */

/* ====================== 声明：只描述形状 ====================== */

struct Point {
    int x;
    int y;
};
typedef struct Point Point;          /* 之后可以只写 Point */

/* 声明的同时 typedef 更常见，等价于上面两行：
 *     typedef struct Point { int x; int y; } Point;
 */

/* ---- 嵌套：结构体的成员可以是另一个结构体 ---- */
struct Rect {
    Point lt;                        /* 左上 */
    Point rb;                        /* 右下 */
};
typedef struct Rect Rect;

/* ---- 自引用：成员只能放"自己的指针"，且必须写 struct Node ---- */
struct Node {
    int value;
    struct Node *next;
};
typedef struct Node Node;

/* ---- 匿名结构体 + typedef：没有标签，因此不能自引用 ---- */
typedef struct {
    int  id;
    char name[16];
} User;

/* ====================== 定义 + 使用 ====================== */

/* 值返回：拷一份给调用方 */
static Point make_point(int x, int y)
{
    Point p = { x, y };
    return p;
}

/* 值传递：整个结构体拷一份进来，函数里改不影响外面 */
static void print_point(const char *label, Point p)
{
    printf("   %s = (%d, %d)\n", label, p.x, p.y);
}

/* 指针传递：大结构体的常规做法，const 表示只读 */
static void print_rect(const Rect *r)
{
    printf("   rect: lt = (%d, %d), rb = (%d, %d), 宽 = %d, 高 = %d\n",
           r->lt.x, r->lt.y, r->rb.x, r->rb.y,
           r->rb.x - r->lt.x, r->rb.y - r->lt.y);
}

static int rect_area(Rect r)
{
    return (r.rb.x - r.lt.x) * (r.rb.y - r.lt.y);
}

/* 显式定义"相等"：逐成员比较，不碰填充字节 */
static int point_equal(Point a, Point b)
{
    return a.x == b.x && a.y == b.y;
}

int main(void)
{
    /* ---------- 1. 为什么需要结构体 ---------- */
    printf("1. 平行数组 vs 结构体\n");
    int   ids[3]      = { 1, 2, 3 };
    char  names[3][16] = { "alice", "bob", "carol" };
    float scores[3]   = { 90.5f, 78.0f, 88.5f };

    for (int i = 0; i < 3; i++)
        printf("   #%d %-6s %.1f（三个数组靠下标 %d 对齐）\n",
               ids[i], names[i], (double)scores[i], i);
    printf("   一旦按分数排序，三个数组都得同步搬，漏一个就错位\n");
    printf("   结构体把 id/name/score 打包成一个整体，排序时整体交换\n");

    /* ---------- 2. 声明只是"形状"，不分配内存 ---------- */
    printf("2. 声明不分配内存\n");
    printf("   sizeof(struct Point) = %zu 字节（2 个 int）\n", sizeof(struct Point));
    printf("   声明之后可以算大小，但此时还没有任何变量存在\n");
    printf("   struct 关键字不能省：C 里必须写 struct Point（除非 typedef）\n");

    /* ---------- 3. 定义与初始化：初始化的 5 种写法 ---------- */
    printf("3. 定义与初始化（初始化的 5 种写法）\n");
    Point a = { 3, 4 };                  /* ① 按成员顺序 */
    Point b = { .y = 7, .x = 1 };        /* ② C99 指定初始化器：顺序自由 */
    Point c = { 5, 0 };                  /* ③ 只给 x 要补 y：写 { 5 } 会被 -Wextra 警告 */
    Point zero = { 0 };                  /* ④ 全部清零的惯用法 */

    print_point("① a = { 3, 4 }", a);
    print_point("② b = { .y = 7, .x = 1 }", b);
    print_point("③ c = { 5, 0 }", c);
    print_point("④ zero = { 0 }", zero);

    const Point cp = { 1, 2 };           /* ⑤ const：只读结构体 */
    printf("   ⑤ const Point cp = { 1, 2 } → (%d, %d)，之后 cp.x = 3 编译不过\n",
           cp.x, cp.y);
    printf("   Point p; 只定义不初始化，成员是垃圾值，不是 0\n");
    printf("   嵌套结构体的初始化见第 5 节（.lt = { 1, 2 }）\n");

    /* ---------- 4. 赋值：六种写法 ---------- */
    printf("4. 赋值：六种写法\n");

    Point d;                             /* 先定义，再赋值 */
    d.x = 1;                             /* ① 逐成员赋值 */
    d.y = 2;
    print_point("① 逐成员 d.x = 1, d.y = 2", d);

    d = a;                               /* ② 整体赋值：逐成员拷贝 */
    print_point("② 整体赋值 d = a", d);
    d.x = 100;
    print_point("   改 d.x 之后 d", d);
    print_point("   a 不受影响（拷贝，不是共享）", a);

    d = (Point){ 7, 8 };                 /* ③ 复合字面量赋值（C99） */
    print_point("③ d = (Point){ 7, 8 }", d);
    print_point("   (Point){ 9, 9 } 直接当实参", (Point){ 9, 9 });

    d = make_point(1, 2);                /* ④ 函数返回值赋值 */
    print_point("④ d = make_point(1, 2)", d);

    Point e;
    memcpy(&e, &a, sizeof e);            /* ⑤ memcpy：按字节拷贝 */
    print_point("⑤ memcpy(&e, &a, sizeof e)", e);

    memset(&e, 0, sizeof e);             /* ⑥ memset：连填充字节一起清零 */
    print_point("⑥ memset(&e, 0, sizeof e)", e);

    printf("   初始化只能写在定义处，赋值面向已经存在的变量\n");
    printf("   类型不同（哪怕成员一模一样）也不能互相赋值\n");

    printf("   整体比较不行：if (a == d) 编译错误，必须逐成员比\n");
    printf("   point_equal(a, d) = %d\n", point_equal(a, d));
    printf("   point_equal(a, a) = %d（用 memcmp 有填充字节误判的风险）\n",
           point_equal(a, a));

    /* ---------- 5. 使用：`.` / `->` / 嵌套 / 数组 / 传参 / 返回 ---------- */
    printf("5. 使用：成员访问 / 指针 / 嵌套 / 数组 / 传参 / 返回\n");

    Point p5 = { 4, 5 };
    Point *pp = &p5;
    printf("   p5.x = %d，(*pp).x = %d，pp->x = %d（三者等价）\n",
           p5.x, (*pp).x, pp->x);
    pp->y += 10;                         /* 通过指针改原对象 */
    print_point("pp->y += 10 之后 p5", p5);

    Rect r = { .lt = { 1, 2 }, .rb = { 5, 6 } };   /* 嵌套的指定初始化 */
    print_rect(&r);
    printf("   rect_area(r) = %d\n", rect_area(r));

    Point pts[3] = { { 1, 1 }, { 2, 2 }, { 3, 3 } };
    printf("   数组：");
    for (int i = 0; i < 3; i++) {
        pts[i].x += 10;                  /* 整体是数组，元素是结构体 */
        printf("(%d, %d)%s", pts[i].x, pts[i].y, i == 2 ? "\n" : " ");
    }

    Point made = make_point(8, 9);       /* 值返回：拿到的是一份拷贝 */
    print_point("make_point(8, 9)", made);

    /* ---------- 6. sizeof 与填充 ---------- */
    printf("6. sizeof 与对齐填充\n");
    struct Mixed { char c; int i; char d; };
    struct Tight { int i; char c; char d; };

    printf("   struct Mixed { char c; int i; char d; } → sizeof = %zu\n",
           sizeof(struct Mixed));
    printf("   1 + 4 + 1 = 6，但实际是 %zu：中间和末尾都插了填充\n",
           sizeof(struct Mixed));
    printf("   struct Tight { int i; char c; char d; } → sizeof = %zu\n",
           sizeof(struct Tight));
    printf("   成员顺序影响大小：大的放前面通常更省\n");
    printf("   offsetof(Mixed, c) = %zu, offsetof(Mixed, i) = %zu, offsetof(Mixed, d) = %zu\n",
           offsetof(struct Mixed, c), offsetof(struct Mixed, i),
           offsetof(struct Mixed, d));
    printf("   填充字节内容不确定 → 不能 memcmp、不能直接算哈希、不能裸写文件\n");

    /* ---------- 7. 自引用与匿名结构体 ---------- */
    printf("7. 自引用结构体 / 匿名结构体\n");
    Node n1 = { 1, NULL };               /* 静态"节点池"，不用 malloc */
    Node n2 = { 2, NULL };
    Node n3 = { 3, NULL };
    n1.next = &n2;
    n2.next = &n3;

    printf("   链表：");
    for (const Node *p = &n1; p != NULL; p = p->next)
        printf("%d%s", p->value, p->next ? " -> " : "\n");
    printf("   struct Node { int value; struct Node *next; } 只能放自己的指针\n");
    printf("   结构体内部不能放自己（大小会无限递归）\n");

    User users[2] = {
        { .id = 1, .name = "alice" },     /* 匿名结构体也要按成员顺序/指定初始化 */
        { .id = 2, .name = "bob" },
    };
    for (int i = 0; i < 2; i++)
        printf("   user #%d %s\n", users[i].id, users[i].name);
    printf("   匿名 + typedef 省掉标签，但没有标签就无法自引用\n");

    /* ---------- 8. 小结 ---------- */
    printf("8. 小结：\n");
    printf("    声明 struct X { ... }; 只描述形状，不分配内存，末尾必须有分号\n");
    printf("    定义 X x; 才真正分配内存；局部变量成员是垃圾值，要显式初始化\n");
    printf("    typedef struct X { ... } X; 之后可以省掉 struct 关键字\n");
    printf("    初始化 5 种：按序、指定初始化器、部分、{0}、嵌套\n");
    printf("    赋值 6 种：逐成员、整体、复合字面量、memcpy、memset、函数返回值\n");
    printf("    使用：p.x / pp->x / 嵌套 / 数组 / 值传参 / 指针传参 / 返回值\n");
    printf("    结构体不能 ==，逐成员比；别用 memcmp，填充字节不确定\n");
    printf("    sizeof 含填充，成员顺序会影响大小\n");
    printf("    自引用只能放自己的指针，且必须写 struct 标签\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 01_结构体基础.c -o bin/01_结构体基础
 *     ./bin/01_结构体基础
 *
 * 输出如下：
 *
 * 1. 平行数组 vs 结构体
 *    #1 alice  90.5（三个数组靠下标 0 对齐）
 *    #2 bob    78.0（三个数组靠下标 1 对齐）
 *    #3 carol  88.5（三个数组靠下标 2 对齐）
 *    一旦按分数排序，三个数组都得同步搬，漏一个就错位
 *    结构体把 id/name/score 打包成一个整体，排序时整体交换
 * 2. 声明不分配内存
 *    sizeof(struct Point) = 8 字节（2 个 int）
 *    声明之后可以算大小，但此时还没有任何变量存在
 *    struct 关键字不能省：C 里必须写 struct Point（除非 typedef）
 * 3. 定义与初始化（初始化的 5 种写法）
 *    ① a = { 3, 4 } = (3, 4)
 *    ② b = { .y = 7, .x = 1 } = (1, 7)
 *    ③ c = { 5, 0 } = (5, 0)
 *    ④ zero = { 0 } = (0, 0)
 *    ⑤ const Point cp = { 1, 2 } → (1, 2)，之后 cp.x = 3 编译不过
 *    Point p; 只定义不初始化，成员是垃圾值，不是 0
 *    嵌套结构体的初始化见第 5 节（.lt = { 1, 2 }）
 * 4. 赋值：六种写法
 *    ① 逐成员 d.x = 1, d.y = 2 = (1, 2)
 *    ② 整体赋值 d = a = (3, 4)
 *       改 d.x 之后 d = (100, 4)
 *       a 不受影响（拷贝，不是共享） = (3, 4)
 *    ③ d = (Point){ 7, 8 } = (7, 8)
 *       (Point){ 9, 9 } 直接当实参 = (9, 9)
 *    ④ d = make_point(1, 2) = (1, 2)
 *    ⑤ memcpy(&e, &a, sizeof e) = (3, 4)
 *    ⑥ memset(&e, 0, sizeof e) = (0, 0)
 *    初始化只能写在定义处，赋值面向已经存在的变量
 *    类型不同（哪怕成员一模一样）也不能互相赋值
 *    整体比较不行：if (a == d) 编译错误，必须逐成员比
 *    point_equal(a, d) = 0
 *    point_equal(a, a) = 1（用 memcmp 有填充字节误判的风险）
 * 5. 使用：成员访问 / 指针 / 嵌套 / 数组 / 传参 / 返回
 *    p5.x = 4，(*pp).x = 4，pp->x = 4（三者等价）
 *    pp->y += 10 之后 p5 = (4, 15)
 *    rect: lt = (1, 2), rb = (5, 6), 宽 = 4, 高 = 4
 *    rect_area(r) = 16
 *    数组：(11, 1) (12, 2) (13, 3)
 *    make_point(8, 9) = (8, 9)
 * 6. sizeof 与对齐填充
 *    struct Mixed { char c; int i; char d; } → sizeof = 12
 *    1 + 4 + 1 = 6，但实际是 12：中间和末尾都插了填充
 *    struct Tight { int i; char c; char d; } → sizeof = 8
 *    成员顺序影响大小：大的放前面通常更省
 *    offsetof(Mixed, c) = 0, offsetof(Mixed, i) = 4, offsetof(Mixed, d) = 8
 *    填充字节内容不确定 → 不能 memcmp、不能直接算哈希、不能裸写文件
 * 7. 自引用结构体 / 匿名结构体
 *    链表：1 -> 2 -> 3
 *    struct Node { int value; struct Node *next; } 只能放自己的指针
 *    结构体内部不能放自己（大小会无限递归）
 *    user #1 alice
 *    user #2 bob
 *    匿名 + typedef 省掉标签，但没有标签就无法自引用
 * 8. 小结：
 *     声明 struct X { ... }; 只描述形状，不分配内存，末尾必须有分号
 *     定义 X x; 才真正分配内存；局部变量成员是垃圾值，要显式初始化
 *     typedef struct X { ... } X; 之后可以省掉 struct 关键字
 *     初始化 5 种：按序、指定初始化器、部分、{0}、嵌套
 *     赋值 6 种：逐成员、整体、复合字面量、memcpy、memset、函数返回值
 *     使用：p.x / pp->x / 嵌套 / 数组 / 值传参 / 指针传参 / 返回值
 *     结构体不能 ==，逐成员比；别用 memcmp，填充字节不确定
 *     sizeof 含填充，成员顺序会影响大小
 *     自引用只能放自己的指针，且必须写 struct 标签
 * ==========================================================================
 */
