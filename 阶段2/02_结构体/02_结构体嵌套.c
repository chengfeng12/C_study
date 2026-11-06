/*
 * 02_结构体嵌套.c —— 阶段2 / 第 02 课 / 第 02 个文件：结构体嵌套
 *
 * 编译：gcc -std=c99 -Wall -Wextra 02_结构体嵌套.c -o bin/02_结构体嵌套
 * 运行：./bin/02_结构体嵌套
 *
 * 01 篇第 5 节只演示了一层嵌套（`Rect r = { .lt = { 1, 2 }, ... }`）。
 * 这一篇把嵌套讲透：怎么声明、怎么初始化、怎么逐层访问、
 * 内存里到底是怎么排的，以及最容易踩的一个坑——值内嵌 vs 指针成员。
 *
 * ====================== 一、嵌套的三种形态 ======================
 * "嵌套"就是拿结构体当成员。按成员的类型，分三种：
 *
 * (1) **值内嵌**：成员是另一个结构体**本身**
 *          struct Date      { int year, month, day; };
 *          struct Timestamp { struct Date date; struct Time time; };
 *      内存里内层结构体**直接展开**在外层里面，一块连续内存。
 *      整体赋值时，内层成员是**逐个拷贝**的（深拷贝）。
 *
 * (2) **数组内嵌**：成员是同一种结构体的数组
 *          struct Schedule { struct Timestamp slots[3]; };
 *      三个 Timestamp 首尾相接排在 Schedule 里，没有指针。
 *
 * (3) **指针内嵌**：成员是"指向另一个结构体的指针"
 *          struct Tree { struct Node *root; };
 *      只存一个地址，被指向的对象在别处（栈上、堆上、数组里）。
 *      整体赋值时**只拷指针**（浅拷贝），两个结构体指向同一个对象。
 *
 * ★ 三种形态的区别，本质只有一个问题：**成员的数据放在哪**。
 *      值/数组内嵌 → 放在自己肚子里，生命周期跟着自己，拷贝是独立的
 *      指针内嵌   → 放在别处，生命周期不归自己管，拷贝后共享
 *   这个区别决定了后面第六节的深浅拷贝问题，也是嵌套最容易出错的地方。
 *
 * ====================== 二、声明顺序：内层必须先"完整" ======================
 * 编译器要算外层结构体的大小，就必须知道每个成员的完整定义，
 * 所以内层结构体必须**先定义完**，外层才能用它当成员：
 *
 *      struct Date { int year, month, day; };     // ① 先完整定义
 *      struct Timestamp {
 *          struct Date date;                      // ② 才能值内嵌
 *          struct Date *pdate;                    // ③ 指针当然也可以
 *      };
 *
 * 反过来不行：
 *      struct Timestamp { struct Date date; };    // ✗ struct Date 还没定义，
 *      struct Date      { int year, month, day; };  //   编译器不知道它多大
 *
 * ★ 例外：**指针成员只要"不完整类型"就够了**，因为指针大小固定。
 *   前置声明（forward declaration）只写标签、不写成员，叫不完整类型：
 *
 *      struct Later;                    // 前置声明：不完整类型
 *      struct Ref { struct Later *p; }; // ✔ 只有指针，不完整也能编译
 *      struct Later { int v; };         // 稍后补全，Ref 不用改
 *
 *   这条规则支撑了两件大事：
 *    - **自引用**：链表节点里放 `struct Node *next`（自己的指针）
 *    - **互相引用**：A 里有 B 的指针，B 里有 A 的指针，总有一个要先声明
 *
 * ★ 但**不能放自己的值**：`struct Node { struct Node next; };` 是死循环，
 *   大小会无限大，编译直接报 "field has incomplete type"。
 *
 * ====================== 三、初始化：四种写法 ======================
 * 假设：
 *      struct Date      { int year, month, day; };
 *      struct Time      { int hour, minute, second; };
 *      struct Timestamp { struct Date date; struct Time time; };
 *
 * (1) **按成员顺序，一层层往里套花括号**（最直观）
 *          struct Timestamp a = { { 2026, 11, 6 }, { 9, 30, 0 } };
 *          //                      └─ date ─┘   └─ time ─┘
 *
 * (2) **逐层指定初始化器**（推荐，顺序自由、不怕加成员）
 *          struct Timestamp b = {
 *              .time = { .hour = 9, .minute = 30 },
 *              .date = { .year = 2026, .month = 11, .day = 6 },
 *          };
 *      注意这里故意把 time 写在前面：**指定的顺序可以和外层成员顺序不同**。
 *
 * (3) **"扁平"指定初始化器**（C99 允许，省一层花括号）
 *          struct Timestamp c = { .date.year = 2026, .time.hour = 9 };
 *      写法是"外层成员.内层成员"，gcc/clang 都支持。
 *      没提到的成员全部为 0 —— 适合"只关心几个字段"的场景。
 *      ★ 混用时注意：后面出现的指定符会覆盖前面的值，按书写顺序生效。
 *
 * (4) **内层是数组**
 *          struct Schedule s = {
 *              .slots = {
 *                  { { 2026, 11, 6 }, { 9, 0, 0 } },
 *                  { { 2026, 11, 6 }, { 13, 30, 0 } },
 *              },
 *          };
 *      数组元素也可以指定下标：`.slots[2] = { ... }`。
 *
 * ★ 想全部清零：`struct Timestamp t = { 0 };`，一层就够了 —— 0 会递归填满
 *   所有标量成员，不用写 `{ { 0 }, { 0 } }`。
 *
 * ★ 用 -Wextra 编译时，**部分初始化内层**容易触发 missing-field-initializers：
 *          struct Timestamp t = { { 2026 } };     // ⚠ 内层的 month/day 没写
 *   要么写全，要么用指定初始化器（指定写法不报这个警告）。
 *
 * ====================== 四、访问：一层一层剥 ======================
 * (1) **`.` 链**：值内嵌就用点，几层就几个点
 *          t.date.year
 *          r.lt.x
 *      读法：先拿到 t，再取成员 date（一个 Date 对象），再取 year。
 *
 * (2) **`->` 链**：外层是指针，第一层用 ->，后面用 .
 *          Timestamp *pt = &t;
 *          pt->date.year        // 先解引用拿到 Timestamp，再 .date，再 .year
 *          pt->time.minute
 *      也可以写成 `(*pt).date.year`，括号不能省（`.` 的优先级高于 `*`）。
 *
 * (3) **混合**：内层是指针，第二层又要用 ->
 *          struct Tree { struct Node *root; };
 *          tree.root->value     // tree.root 是指针，取 value 要用 ->
 *          tree->root->value    // 外层也是指针时，两个 ->
 *      记忆法：**左边是对象就用 `.`，左边是指针就用 `->`**，一个一个来。
 *
 * ★ 常见错误：
 *          pt.date.year         // ✗ pt 是指针，不能直接点
 *          *pt.date.year        // ✗ 解析成 *(pt.date.year)，早报错了
 *          (*pt).date.year      // ✔ 或者 pt->date.year
 *
 * (4) **能取地址**：嵌套成员的地址就是外层对象地址加偏移
 *          &t.time          // Date/Time 成员的起始地址
 *          &t.date.day      // 最内层标量成员的地址
 *      这让"把一个子结构体传给函数"变得很自然：直接传 `&t.time`。
 *
 * ====================== 五、内存布局：内层是"摊平"进去的 ======================
 * （对齐规则、特殊成员、赋值成本的完整推导见 03 篇。）
 * 值内嵌不是"存一个指针"，而是把内层的成员**原样展开**在外层的偏移处：
 *
 *      struct Timestamp {
 *          struct Date date;      // 偏移 0，占 12 字节
 *          struct Time time;      // 偏移 12，占 12 字节
 *      };                         // sizeof = 24
 *
 * 所以 `sizeof(struct Timestamp) == sizeof(Date) + sizeof(Time)`（无额外填充时）。
 *
 * ★ 但填充仍然存在，而且**每个结构体各自对齐**：
 *      struct Inner { char c; int i; };    // sizeof = 8（c 后面有 3 字节填充）
 *      struct Outer {
 *          char         tag;               // 偏移 0
 *          struct Inner in;                // 偏移 4（tag 后填 3 字节）
 *          char         tail;              // 偏移 12
 *      };                                  // sizeof = 16（tail 后填 3 字节）
 *      sizeof(Outer) = 16，不是 1 + 8 + 1 = 10。
 *
 *   规律：**外层把内层当成一个整体来对齐**，内层自己的填充也一并带进来。
 *   所以"拆开写几个小结构体"往往会比"一个扁平的大结构体"更占内存 ——
 *   这也是内核、协议头里经常手动排成员顺序的原因。
 *
 * ★ 用 `offsetof(struct Outer, in)` 和 `offsetof(struct Outer, tail)`
 *   能看到真实偏移（需要 <stddef.h>）。具体数值依平台，但规律一致。
 *
 * ====================== 六、值内嵌 vs 指针成员：深拷贝与浅拷贝 ======================
 * 这是嵌套里最重要的区别，也是面试和 bug 的高发区。
 *
 * (1) **值内嵌 → 整体赋值是独立的**
 *          struct HolderValue { struct Inner in; };
 *          HolderValue v2 = v1;        // 把 v1.in 的内容复制进 v2.in
 *          v2.in.i = 99;               // 只改 v2 的副本
 *          // v1.in.i 依然是原值
 *
 * (2) **指针成员 → 整体赋值只是拷地址**
 *          struct HolderPtr { struct Inner *in; };
 *          HolderPtr p2 = p1;          // p2.in 和 p1.in 指向同一个 Inner
 *          p2.in->i = 99;              // p1.in->i 也变成 99
 *
 * 记忆：**赋值只拷贝结构体自己的字节**。值内嵌时内层数据就在自己的字节里，
 * 所以被拷走；指针成员时自己的字节里只有一个地址，拷走的也只是地址。
 *
 * ★ 由此引出三个必须想清楚的问题：
 *   1. 谁分配、谁释放？指针成员如果指向 malloc 的内存，`free` 只能有一次，
 *      而两个拷贝都以为自己该负责 → 双重释放（double free）。
 *   2. 生命周期够不够长？指针指向栈上的局部变量，函数返回后就悬空。
 *   3. 结构体里还有字符串（`char *name`）时，赋值同样是"共享"，不是"复制一份名字"。
 *
 * ★ 需要真正独立的拷贝时，写一个 `xxx_clone()`：值成员直接拷，
 *   指针成员重新 malloc 再 memcpy/memmove。这就是 C 里的"深拷贝"。
 *
 * ====================== 七、常见用途 ======================
 *  - **时间戳**：Date + Time → Timestamp
 *  - **几何体**：Point → Rect / Circle / Line，都是"点 + 点"或"点 + 半径"
 *  - **配置分层**：Config { Network net; Log log; Path path; }，每块单独定义
 *  - **日志/事件**：LogEntry { Timestamp ts; int level; char msg[64]; }
 *  - **树的节点**：Node { int value; Node *left, *right; }（指针内嵌）
 *  - **带元数据的数组**：Buffer { size_t len; char data[256]; }，嵌套在消息里
 *
 * ====================== 八、常见坑 ======================
 *  1. 内层还没定义就值内嵌         incomplete type，编译器不知道多大
 *  2. 自引用时放了自己的值         大小无限递归，编译报错；只能放指针
 *  3. 前置声明之后直接定义变量     不完整类型不能定义变量、不能取成员
 *  4. 初始化少写一层花括号         内层成员会错位赋值，-Wextra 会提示
 *  5. 用 `{ 2026 }` 初始化嵌套      内层其余成员为 0，但要确认这是你要的
 *  6. 忘记 `->` 与 `.` 的切换      左边指针用 ->，左边对象用 .，混一层就报错
 *  7. `*pt.date.year`              优先级问题：`.` 高于 `*`，必须写 `(*pt).date.year`
 *  8. 以为整体赋值会深拷贝指针成员  只拷地址，两个结构体共享同一对象
 *  9. 嵌套后按扁平结构体算大小     每个内层各自对齐，sizeof 往往比想的大
 * 10. 把嵌套成员地址传出去        只要外层对象生命周期结束，内层地址同样失效
 *
 * ====================== 九、实践建议 ======================
 *  - 内层结构体先定义、再被值内嵌；类型定义放头文件，顺序依赖一目了然
 *  - 优先用**指定初始化器**，尤其嵌套时：`.date.year = 2026` 比数花括号可靠
 *  - 大结构体传参用 `const Timestamp *`，别整块拷贝
 *  - 成员是指针时，在注释里写清"谁拥有、谁释放"，并在类型名上体现
 *    （`Owner` / `Ref` / `View` 这类后缀很有效）
 *  - 需要独立副本时写显式的 `clone` 函数，别指望 `=` 帮你深拷贝
 *  - 关心内存布局时用 `offsetof` + `sizeof` 验证，别靠心算填充
 *  - 跨文件/跨平台传输的数据结构，考虑 `#pragma pack` 或显式序列化，别直接 fwrite
 */

#include <stdio.h>
#include <stddef.h>      /* offsetof */

/* ====================== 内层：先定义 ====================== */

typedef struct Date {
    int year;
    int month;
    int day;
} Date;

typedef struct Time {
    int hour;
    int minute;
    int second;
} Time;

/* ====================== 值内嵌：两层 ====================== */

typedef struct Timestamp {
    Date date;           /* 偏移 0，12 字节 */
    Time time;           /* 偏移 12，12 字节 */
} Timestamp;

/* ====================== 值内嵌：几何 ====================== */

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Rect {
    Point lt;            /* 左上 */
    Point rb;            /* 右下 */
} Rect;

/* ====================== 数组内嵌 ====================== */

typedef struct Schedule {
    Timestamp slots[3];  /* 三个时间戳首尾相接 */
} Schedule;

/* ====================== 指针内嵌：前置声明 + 指针 ====================== */

struct Later;                       /* 不完整类型：只有标签，没有成员 */

typedef struct Ref {
    struct Later *p;                /* ✔ 指针大小固定，不完整类型也能用 */
} Ref;

struct Later {                      /* 稍后补全，Ref 不用改 */
    int v;
};

/* ====================== 值内嵌 vs 指针成员 ====================== */

typedef struct Inner {
    char c;
    int  i;
} Inner;

typedef struct HolderValue {
    Inner in;                       /* 值内嵌：Inner 摊平在 HolderValue 里 */
} HolderValue;

typedef struct HolderPtr {
    Inner *in;                      /* 指针内嵌：只存地址 */
} HolderPtr;

/* ====================== 布局演示：内层带填充 ====================== */

typedef struct Outer {
    char  tag;                      /* 偏移 0 */
    Inner in;                       /* 偏移 4（tag 后填 3 字节） */
    char  tail;                     /* 偏移 12 */
} Outer;

/* ====================== 使用嵌套的几个函数 ====================== */

static void print_date(const Date *d)
{
    printf("%04d-%02d-%02d", d->year, d->month, d->day);
}

static void print_time(const Time *t)
{
    printf("%02d:%02d:%02d", t->hour, t->minute, t->second);
}

/* 外层是指针，取内层用 -> 然后 . */
static void print_ts(const Timestamp *ts)
{
    print_date(&ts->date);
    printf(" ");
    print_time(&ts->time);
}

/* 直接操作内层成员：t->time.hour */
static void bump_hour(Timestamp *t, int hours)
{
    t->time.hour += hours;
}

static int rect_width(const Rect *r)
{
    return r->rb.x - r->lt.x;
}

static int rect_area(const Rect *r)
{
    return (r->rb.x - r->lt.x) * (r->rb.y - r->lt.y);
}

int main(void)
{
    /* ---------- 1. 三种嵌套形态 ---------- */
    printf("1. 三种嵌套形态\n");
    printf("   值内嵌：Timestamp { Date date; Time time; }  → 内层摊平在自己肚子里\n");
    printf("   数组内嵌：Schedule { Timestamp slots[3]; }    → 三个时间戳首尾相接\n");
    printf("   指针内嵌：Ref { struct Later *p; }            → 只存地址，对象在别处\n");

    /* ---------- 2. 声明顺序与不完整类型 ---------- */
    printf("2. 声明顺序：内层必须先完整定义\n");
    printf("   struct Later; 前置声明之后，Ref 里只能放 struct Later *\n");
    struct Later later = { 42 };
    Ref ref = { &later };
    printf("   ref.p->v = %d（先声明标签、再补全定义，合法）\n", ref.p->v);
    printf("   反过来：内层没定义就值内嵌 → 编译器不知道多大，直接报错\n");

    /* ---------- 3. 四种初始化写法 ---------- */
    printf("3. 初始化：四种写法\n");

    Timestamp a = { { 2026, 11, 6 }, { 9, 30, 0 } };          /* ① 按序套花括号 */
    printf("   ① 按序：");
    print_ts(&a);
    printf("\n");

    Timestamp b = {                                            /* ② 逐层指定 */
        .time = { .hour = 9, .minute = 30, .second = 0 },
        .date = { .year = 2026, .month = 11, .day = 6 },
    };
    printf("   ② 指定（time 写在前面也行）：");
    print_ts(&b);
    printf("\n");

    Timestamp c = { .date.year = 2026, .date.month = 11,     /* ③ 扁平指定 */
                    .time.hour = 9, .time.minute = 30 };
    printf("   ③ 扁平 .date.year：");
    print_ts(&c);
    printf("（未指定的成员为 0）\n");

    Timestamp zero = { 0 };                                   /* ④ 全清零 */
    printf("   ④ { 0 } 递归清零：");
    print_ts(&zero);
    printf("\n");
    printf("   嵌套时优先用指定初始化器，加成员也不会错位\n");

    /* ---------- 4. 逐层访问 ---------- */
    printf("4. 逐层访问：左边的类型决定用 . 还是 ->\n");
    Timestamp *pt = &a;

    printf("   a.date.year          = %d\n", a.date.year);
    printf("   pt->date.year        = %d\n", pt->date.year);
    printf("   (*pt).date.year      = %d（和上面等价，括号不能省）\n",
           (*pt).date.year);
    printf("   pt->time.minute      = %d\n", pt->time.minute);

    bump_hour(pt, 2);
    printf("   bump_hour(pt, 2) 之后：");
    print_ts(&a);
    printf("\n");

    printf("   取内层地址：&a.time 传给函数，就是传一个 Time *\n");
    printf("   &a.date.day 的偏移 = %zu\n",
           (size_t)((const char *)&a.date.day - (const char *)&a));

    /* ---------- 5. 内存布局 ---------- */
    printf("5. 内存布局：内层摊平，各自对齐\n");
    printf("   sizeof(Date)      = %zu\n", sizeof(Date));
    printf("   sizeof(Time)      = %zu\n", sizeof(Time));
    printf("   sizeof(Timestamp) = %zu（= Date + Time）\n", sizeof(Timestamp));
    printf("   offsetof(Timestamp, date) = %zu, offsetof(Timestamp, time) = %zu\n",
           offsetof(Timestamp, date), offsetof(Timestamp, time));

    printf("   --- 内层自己带填充时 ---\n");
    printf("   sizeof(Inner)            = %zu（char + 3 填充 + int）\n", sizeof(Inner));
    printf("   offsetof(Outer, tag)     = %zu\n", offsetof(Outer, tag));
    printf("   offsetof(Outer, in)      = %zu（tag 后填到 4 字节边界）\n",
           offsetof(Outer, in));
    printf("   offsetof(Outer, tail)    = %zu\n", offsetof(Outer, tail));
    printf("   sizeof(Outer)            = %zu（1 + 8 + 1 被填充成 %zu）\n",
           sizeof(Outer), sizeof(Outer));
    printf("   外层把内层当整体对齐，内层的填充也一起带进来\n");

    /* ---------- 6. 值内嵌 vs 指针成员 ---------- */
    printf("6. 值内嵌 vs 指针成员：一次赋值，两种命运\n");

    HolderValue v1 = { { 'A', 1 } };
    HolderValue v2 = v1;                 /* 整体赋值：Inner 被复制一份 */
    v2.in.i = 99;
    printf("   值内嵌：v2.in.i = %d，v1.in.i = %d（各自的副本，互不影响）\n",
           v2.in.i, v1.in.i);

    Inner shared = { 'B', 2 };
    HolderPtr p1 = { &shared };
    HolderPtr p2 = p1;                   /* 整体赋值：只拷了指针 */
    p2.in->i = 88;
    printf("   指针成员：p2.in->i = %d，p1.in->i = %d（指向同一个 Inner）\n",
           p2.in->i, p1.in->i);
    printf("   赋值只拷贝结构体自己的字节：值内嵌拷内容，指针成员只拷地址\n");
    printf("   所以指针成员要额外想清楚：谁分配、谁释放、生命周期够不够长\n");

    /* ---------- 7. 数组内嵌 ---------- */
    printf("7. 数组内嵌：Schedule { Timestamp slots[3]; }\n");
    Schedule sch = {
        .slots = {
            { { 2026, 11, 6 }, { 9, 0, 0 } },
            { { 2026, 11, 6 }, { 13, 30, 0 } },
            { { 2026, 11, 7 }, { 10, 15, 0 } },
        },
    };

    for (int i = 0; i < 3; i++) {
        printf("   slot[%d] = ", i);
        print_ts(&sch.slots[i]);         /* 数组元素 -> 取地址 -> 传指针 */
        printf("\n");
    }
    printf("   sizeof(Schedule) = %zu（3 × sizeof(Timestamp)）\n", sizeof(Schedule));

    /* ---------- 8. 几何例子：两层嵌套 + 计算 ---------- */
    printf("8. 几何例子：Rect { Point lt; Point rb; }\n");
    Rect r = { .lt = { 1, 2 }, .rb = { 5, 6 } };
    printf("   r.lt = (%d, %d), r.rb = (%d, %d)\n", r.lt.x, r.lt.y, r.rb.x, r.rb.y);
    printf("   rect_width(&r) = %d, rect_area(&r) = %d\n",
           rect_width(&r), rect_area(&r));
    printf("   嵌套成员可以直接取地址：&r.lt 就是 Point *\n");

    /* ---------- 9. 小结 ---------- */
    printf("9. 小结：\n");
    printf("    嵌套三形态：值内嵌 / 数组内嵌 / 指针内嵌，区别在于数据放在哪\n");
    printf("    内层必须先完整定义；只有指针成员可以用不完整类型\n");
    printf("    自引用只能放自己的指针，放自己的值会无限递归\n");
    printf("    初始化优先用指定初始化器，.date.year 这种扁平写法也可以\n");
    printf("    访问逐层剥：左边是对象用 .，左边是指针用 ->\n");
    printf("    值内嵌是摊平的：sizeof 累加，内层填充也会带进来\n");
    printf("    整体赋值只拷自己的字节：值内嵌=深拷贝，指针成员=共享\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 02_结构体嵌套.c -o bin/02_结构体嵌套
 *     ./bin/02_结构体嵌套
 *
 * 输出如下：
 *
 * 1. 三种嵌套形态
 *    值内嵌：Timestamp { Date date; Time time; }  → 内层摊平在自己肚子里
 *    数组内嵌：Schedule { Timestamp slots[3]; }    → 三个时间戳首尾相接
 *    指针内嵌：Ref { struct Later *p; }            → 只存地址，对象在别处
 * 2. 声明顺序：内层必须先完整定义
 *    struct Later; 前置声明之后，Ref 里只能放 struct Later *
 *    ref.p->v = 42（先声明标签、再补全定义，合法）
 *    反过来：内层没定义就值内嵌 → 编译器不知道多大，直接报错
 * 3. 初始化：四种写法
 *    ① 按序：2026-11-06 09:30:00
 *    ② 指定（time 写在前面也行）：2026-11-06 09:30:00
 *    ③ 扁平 .date.year：2026-11-00 09:30:00（未指定的成员为 0）
 *    ④ { 0 } 递归清零：0000-00-00 00:00:00
 *    嵌套时优先用指定初始化器，加成员也不会错位
 * 4. 逐层访问：左边的类型决定用 . 还是 ->
 *    a.date.year          = 2026
 *    pt->date.year        = 2026
 *    (*pt).date.year      = 2026（和上面等价，括号不能省）
 *    pt->time.minute      = 30
 *    bump_hour(pt, 2) 之后：2026-11-06 11:30:00
 *    取内层地址：&a.time 传给函数，就是传一个 Time *
 *    &a.date.day 的偏移 = 8
 * 5. 内存布局：内层摊平，各自对齐
 *    sizeof(Date)      = 12
 *    sizeof(Time)      = 12
 *    sizeof(Timestamp) = 24（= Date + Time）
 *    offsetof(Timestamp, date) = 0, offsetof(Timestamp, time) = 12
 *    --- 内层自己带填充时 ---
 *    sizeof(Inner)            = 8（char + 3 填充 + int）
 *    offsetof(Outer, tag)     = 0
 *    offsetof(Outer, in)      = 4（tag 后填到 4 字节边界）
 *    offsetof(Outer, tail)    = 12
 *    sizeof(Outer)            = 16（1 + 8 + 1 被填充成 16）
 *    外层把内层当整体对齐，内层的填充也一起带进来
 * 6. 值内嵌 vs 指针成员：一次赋值，两种命运
 *    值内嵌：v2.in.i = 99，v1.in.i = 1（各自的副本，互不影响）
 *    指针成员：p2.in->i = 88，p1.in->i = 88（指向同一个 Inner）
 *    赋值只拷贝结构体自己的字节：值内嵌拷内容，指针成员只拷地址
 *    所以指针成员要额外想清楚：谁分配、谁释放、生命周期够不够长
 * 7. 数组内嵌：Schedule { Timestamp slots[3]; }
 *    slot[0] = 2026-11-06 09:00:00
 *    slot[1] = 2026-11-06 13:30:00
 *    slot[2] = 2026-11-07 10:15:00
 *    sizeof(Schedule) = 72（3 × sizeof(Timestamp)）
 * 8. 几何例子：Rect { Point lt; Point rb; }
 *    r.lt = (1, 2), r.rb = (5, 6)
 *    rect_width(&r) = 4, rect_area(&r) = 16
 *    嵌套成员可以直接取地址：&r.lt 就是 Point *
 * 9. 小结：
 *     嵌套三形态：值内嵌 / 数组内嵌 / 指针内嵌，区别在于数据放在哪
 *     内层必须先完整定义；只有指针成员可以用不完整类型
 *     自引用只能放自己的指针，放自己的值会无限递归
 *     初始化优先用指定初始化器，.date.year 这种扁平写法也可以
 *     访问逐层剥：左边是对象用 .，左边是指针用 ->
 *     值内嵌是摊平的：sizeof 累加，内层填充也会带进来
 *     整体赋值只拷自己的字节：值内嵌=深拷贝，指针成员=共享
 * ==========================================================================
 */
