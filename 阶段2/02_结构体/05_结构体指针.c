/*
 * 05_结构体指针.c —— 阶段2 / 第 02 课 / 第 05 个文件：结构体指针
 *
 * 编译：gcc -std=c99 -Wall -Wextra 05_结构体指针.c -o bin/05_结构体指针
 * 运行：./bin/05_结构体指针
 *
 * `->` 的语法在阶段1 第 06 课第 06 篇已经打过底。这一篇补齐结构体指针的
 * 完整用法，重点在四个地方：
 *
 *      一、`&` / `*` / `->` 三者的关系，以及两种等价写法的优先级陷阱
 *      二、const 与指针的四种组合（很多人到这里就分不清了）
 *      三、指针数组 `T *a[n]` 和数组指针 `T (*a)[n]` 的区别
 *      四、动态分配：malloc / calloc / realloc / free 的正确姿势
 *
 * ====================== 一、三种写法，一个地址 ======================
 *      Point  pt = { 3, 4 };
 *      Point *p  = &pt;          // p 里存的是 pt 的地址
 *
 * 访问 pt 的成员，有四种写法，值都一样：
 *
 *      pt.x            直接用对象
 *      (*p).x          先解引用拿到对象，再取成员
 *      p->x            语法糖，等价于 (*p).x（最常用）
 *      p[0].x          把 p 当"只有一个元素的数组"看，等价于 (*(p+0)).x
 *
 * ★ 为什么必须有 `->`？因为 `.` 的优先级高于 `*`：
 *      *p.x      →  *(p.x)      ✗ p 是指针，没有 .x 成员，编译错误
 *      (*p).x    →  ✔ 括号把 *p 绑成整体
 *      p->x      →  ✔ 专门为此设计的运算符
 *
 * ★ 反过来，`&` 和 `->` 一起用时不用加括号：
 *      &p->x     等价于 &(p->x)     取成员的地址
 *      &pt.x     同上
 *
 * ★ `p->x` 的 `->` 左边必须是指针；`pt.x` 的 `.` 左边必须是对象。
 *   左边类型变了，运算符就得跟着换 —— 这是结构体指针最常见的报错来源。
 *
 * ====================== 二、指针从哪来：四种来源 ======================
 * (1) **指向一个普通变量**
 *          Point *p = &pt;
 *
 * (2) **指向数组元素**
 *          Point arr[3];
 *          Point *p = arr;          // 等价于 &arr[0]
 *          Point *q = &arr[2];      // 指向第三个元素
 *          Point *r = arr + 1;      // 指针算术，指向第二个
 *
 * (3) **指向堆内存**
 *          Point *p = malloc(sizeof *p);
 *          if (p != NULL) { p->x = 1; free(p); }
 *      ★ `sizeof *p` 比写 `sizeof(Point)` 更抗改：以后换类型不会漏改。
 *
 * (4) **空指针（还没有目标）**
 *          Point *p = NULL;
 *      NULL 表示"不指向任何东西"，解引用它就是崩溃。
 *      所有"可能没有结果"的接口都返回 NULL，调用方**第一步先判空**。
 *
 * ★ 未初始化的指针（野指针）和 NULL 不是一回事：
 *      Point *p;          // 里面是垃圾地址，判空也挡不住
 *      Point *p = NULL;   // ✔ 至少能安全判断
 *   定义指针就初始化，这是零成本的纪律。
 *
 * ====================== 三、const 的四种组合 ======================
 * 把 const 放在 `*` 的左边还是右边，含义完全不同。读法：**const 修饰它右边的东西**。
 *
 *      const Point *p1 = &pt;        // 指向"只读 Point"的指针
 *                                    // 不能 p1->x = 1；可以 p1 = &other
 *      Point *const p2 = &pt;        // 只读的指针，指向 Point
 *                                    // 可以 p2->x = 1；不能 p2 = &other
 *      const Point *const p3 = &pt;  // 两边都只读
 *      Point *p4 = &pt;              // 都能改
 *
 * 记忆法：
 *      const 在 `*` 左边 → 锁**数据**（指向的内容不能改）
 *      const 在 `*` 右边 → 锁**指针**（指向不能改，数据可以改）
 *
 * ★ 函数参数里 99% 的场景是第一种：`void f(const Point *p)`。
 *   它表示"我只是看看，不会改你的对象"，同时能接受 const 和非 const 实参。
 *   第二种（`Point *const`）在参数里几乎没有意义 —— 形参本来就是副本。
 *
 * ★ 把 `const T *` 传给 `T *` 参数会报警告（丢掉 const 限定），
 *   这是编译器在救你：那条路一旦写进去，只读对象就被改了。
 *
 * ====================== 四、作参数：值传递 vs 指针传递 ======================
 *      void by_value(Point p);        // 拷贝整个结构体：改不动调用方
 *      void by_ptr(Point *p);         // 只传地址：能改调用方，也能省拷贝
 *      void by_const_ptr(const Point *p);  // 只读：省拷贝 + 承诺不改
 *
 * 三个判断依据：
 *  1. **要不要改调用方的对象** → 要改就必须传指针
 *  2. **结构体多大** → 超过 2~3 个机器字（16~24 字节），传指针更省
 *  3. **是不是只读** → 只读就加 const，这是接口契约
 *
 * ★ "值传递改不动调用方"不是缺陷，是特性：
 *      想在函数里"复制一份再改"，就该用值传递，调用方的对象天然安全。
 *
 * ====================== 五、返回值：三种安全写法 ======================
 * (1) **返回调用方传进来的对象的地址**（最安全）
 *          Point *pick_bigger(Point *a, Point *b)
 *          {
 *              return a->x >= b->x ? a : b;
 *          }
 *      返回的指针指向调用方的内存，生命周期由调用方掌握。
 *
 * (2) **返回 static 对象的地址**（能用，但不可重入）
 *          Point *origin(void) { static Point o = { 0, 0 }; return &o; }
 *      两次调用拿到同一个对象；多线程下会互相踩。
 *
 * (3) **返回堆内存**（责任转移给调用方）
 *          Point *make_point(int x, int y)
 *          {
 *              Point *p = malloc(sizeof *p);
 *              if (p != NULL) { p->x = x; p->y = y; }
 *              return p;
 *          }
 *      函数名要体现所有权（`make_` / `create_` / `new_`），注释写明"调用方负责 free"。
 *
 * ★ 绝对不能返回**局部变量的地址**：
 *          Point *bad(void) { Point p = { 1, 2 }; return &p; }   // ✗ 悬空
 *   函数返回时栈帧被回收，那块内存随时会被别人覆盖。
 *   编译器一般会警告（-Wreturn-local-addr），但转一手就可能绕过检查。
 *
 * ★ 另一个更隐蔽的版本：返回**指向局部数组元素**的指针、返回**形参的地址**，
 *   以及返回块作用域复合字面量的地址（03 篇第四节），都是同一类错误。
 *
 * ====================== 六、用指针遍历结构体数组 ======================
 *      Student *p   = arr;
 *      Student *end = arr + n;
 *      for (; p < end; p++)
 *          printf("%s\n", p->name);
 *
 * 几个要点：
 *  - `p++` 前进 **sizeof(Student)** 字节，不是 1 字节
 *  - `p < end` 用指针比较，比 `i < n` 少维护一个下标变量
 *  - `p - arr` 得到当前下标（指针相减的类型是 ptrdiff_t）
 *  - 循环里别改 `arr` 本身，另起一个游标指针
 *
 * ★ 遍历时不要边删边加。结构体数组是连续内存，中间删一个要么整体搬移
 *   （`memmove`），要么用"末尾元素覆盖 + 长度减一"的 O(1) 删除法。
 *
 * ====================== 七、指针数组 vs 数组指针 ======================
 * 这两个名字只差一个字，含义完全相反：
 *
 *      Point *parr[3];      // 指针数组：3 个元素，每个都是 Point *
 *      Point (*pa)[3];      // 数组指针：1 个指针，指向"3 个 Point 的数组"
 *
 * 读法看括号：`*` 先和谁结合。
 *      `Point *parr[3]`   → [] 优先级高 → parr 是数组，元素是 Point *
 *      `Point (*pa)[3]`   → 括号先绑 *pa → pa 是指针，指向 Point[3]
 *
 * 大小差异：
 *      sizeof(parr) = 3 × 8 = 24（三个指针）
 *      sizeof(pa)   = 8（一个指针）
 *      sizeof(*pa)  = 24（它指向的整个数组）
 *
 * 典型用途：
 *  - **指针数组**：排序时只交换 8 字节的指针，不搬整个结构体；
 *    或者表示"一组可选对象"（有些槽位是 NULL）
 *  - **数组指针**：指向二维结构体数组的一行
 *          Point grid[2][3];
 *          Point (*row)[3] = grid;      // row 指向第一行
 *          row[1][2].x                  // 第二行第三个元素
 *
 * ★ 传二维数组给函数时，形参必须写成数组指针：
 *          void f(Point a[][3], int rows);   // 等价于 Point (*a)[3]
 *   第二维的长度不能省 —— 编译器要靠它算 `a[i]` 的步长。
 *
 * ====================== 八、动态分配：malloc / calloc / realloc / free ======================
 *      #include <stdlib.h>
 *
 *      Point *p  = malloc(sizeof *p);           // 一个元素，内容未初始化
 *      Point *a  = calloc(n, sizeof *a);        // n 个元素，全部清零
 *      Point *b  = realloc(a, m * sizeof *b);   // 调整大小
 *      free(p);
 *
 * 四条铁律：
 *
 *  1. **每次 malloc 都要检查 NULL**。内存不足会返回 NULL，直接解引用就是崩溃。
 *
 *  2. **malloc 的内容是未初始化的**。想清零用 calloc，或者随后 memset。
 *     这一点和 calloc 的差别，正是很多"结构体里有垃圾值"的来源。
 *
 *  3. **realloc 要用临时变量接**：
 *          Point *tmp = realloc(a, new_n * sizeof *tmp);
 *          if (tmp == NULL) { free(a); return; }    // 原来的 a 还活着
 *          a = tmp;
 *     直接写 `a = realloc(a, ...)`，失败时 a 被置 NULL，原内存就泄漏了。
 *
 *  4. **free 之后立刻置 NULL**：
 *          free(p);  p = NULL;
 *     挡住"重复释放"和"释放后再用"，这两个都是典型的未定义行为。
 *
 * ★ `sizeof *p` 比 `sizeof(Point)` 更安全：改类型时不用改每一处。
 * ★ C 里 malloc 的返回值**不需要强转**（`(Point *)malloc(...)` 是 C++ 的习惯），
 *   强转反而会掩盖"忘了包含 stdlib.h"导致的错误。
 *
 * ====================== 九、常见坑 ======================
 *  1. 定义了指针没初始化就解引用       野指针，判空也救不了
 *  2. `*p.x`                           优先级问题，应写 `(*p).x` 或 `p->x`
 *  3. 指针用 `.`、对象用 `->`          左右类型不匹配，编译错误
 *  4. 返回局部变量/局部数组元素的地址  函数返回后悬空
 *  5. free 之后继续用（use-after-free） 未定义行为，ASAN 能抓
 *  6. 同一块内存 free 两次             double free，直接 abort
 *  7. realloc 直接赋回原指针           失败时原内存泄漏
 *  8. malloc 不检查 NULL              内存不足时崩溃
 *  9. `T *a[n]` 和 `T (*a)[n]` 搞混   一个是指针数组，一个是数组指针
 * 10. 二维数组传参省掉第二维长度      编译器无法计算步长
 * 11. 指针数组里的空槽不判空就解引用  段错误
 *
 * ====================== 十、实践建议 ======================
 *  - 指针定义即初始化（NULL 或真实地址），别留野指针
 *  - 只读参数一律 `const T *`，既省拷贝又是文档
 *  - 接口返回指针时，在注释里写清"指向谁的内存、谁负责释放、何时失效"
 *  - 结构体超过 16~24 字节，传参就用指针
 *  - 需要排序大结构体时，排指针数组；原数组顺序不变，还省搬移
 *  - 动态数组用 calloc 起步，避免"忘了初始化"的垃圾值
 *  - free 之后置 NULL；释放前先想清楚"这块内存还有谁在用"
 *  - 调试内存问题优先开 `-fsanitize=address`，比 printf 猜快得多
 */

#include <stdio.h>
#include <stdlib.h>      /* malloc / calloc / realloc / free */
#include <string.h>      /* snprintf */

/* ====================== 类型 ====================== */

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Student {
    int   id;
    char  name[16];
    float score;
} Student;

/* ====================== 参数传递的三种形态 ====================== */

/* 值传递：改的是副本，返回改完的副本 */
static Point moved_copy(Point p, int dx, int dy)
{
    p.x += dx;
    p.y += dy;
    return p;
}

/* 指针传递：直接改调用方的对象 */
static void move_in_place(Point *p, int dx, int dy)
{
    p->x += dx;
    p->y += dy;
}

/* const 指针：只读，省拷贝 */
static void print_point(const char *label, const Point *p)
{
    printf("   %s = (%d, %d)\n", label, p->x, p->y);
}

/* ====================== 返回指针的两种安全写法 ====================== */

/* 返回调用方传入对象中较大的那个地址 */
static Point *pick_bigger(Point *a, Point *b)
{
    return a->x >= b->x ? a : b;
}

/* 返回 static 对象：能用，但不可重入 */
static Point *origin(void)
{
    static Point o = { 0, 0 };
    return &o;
}

/* 返回堆内存：所有权交给调用方 */
static Point *make_point(int x, int y)
{
    Point *p = malloc(sizeof *p);     /* sizeof *p 比 sizeof(Point) 更抗改 */
    if (p != NULL) {
        p->x = x;
        p->y = y;
    }
    return p;                         /* 调用方负责 free */
}

/* ====================== 指针数组排序：只交换指针 ====================== */

static void sort_ptrs_by_score(Student **ptrs, int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (ptrs[j]->score < ptrs[j + 1]->score) {
                Student *t = ptrs[j];        /* 只交换 8 字节指针 */
                ptrs[j] = ptrs[j + 1];
                ptrs[j + 1] = t;
            }
        }
    }
}

int main(void)
{
    /* ---------- 1. 四种等价写法 ---------- */
    printf("1. 一个地址，四种写法\n");
    Point pt = { 3, 4 };
    Point *p = &pt;

    printf("   pt.x       = %d\n", pt.x);
    printf("   (*p).x     = %d（括号不能省）\n", (*p).x);
    printf("   p->x       = %d（等价，最常用）\n", p->x);
    printf("   p[0].x     = %d（把 p 看成只有一个元素的数组）\n", p[0].x);
    printf("   *p.x 是错的：. 的优先级高于 *，会被当成 *(p.x)\n");

    /* ---------- 2. 指针的四种来源 ---------- */
    printf("2. 指针从哪来\n");
    Point arr[3] = { { 1, 1 }, { 2, 4 }, { 3, 9 } };
    Point *q = arr;                       /* 指向首元素 */
    Point *r = &arr[2];                   /* 指向第三个元素 */

    printf("   q = arr      → q->x = %d, arr[0].x = %d\n", q->x, arr[0].x);
    printf("   r = &arr[2]  → r->x = %d, r - arr = %td\n", r->x, r - arr);
    printf("   (arr + 1)->x = %d（指针算术走 1 个元素）\n", (arr + 1)->x);

    Point *heap = malloc(sizeof *heap);
    if (heap != NULL) {
        heap->x = 7;
        heap->y = 8;
        printf("   malloc 的指针：heap->x = %d（用完必须 free）\n", heap->x);
        free(heap);
        heap = NULL;                     /* 释放后立即置空 */
    }

    Point *nullp = NULL;
    printf("   NULL 指针：nullp == NULL → %s（解引用前必须先判空）\n",
           nullp == NULL ? "真" : "假");

    /* ---------- 3. const 的四种组合 ---------- */
    printf("3. const 与指针：左边锁数据，右边锁指针\n");
    Point cpt = { 1, 2 };

    const Point *pc = &cpt;              /* 不能改 pc->x，但能改指向 */
    Point *const cp = &cpt;              /* 能改 cp->x，但不能改指向 */
    const Point *const cpc = &cpt;       /* 两个都不能改 */

    cp->x = 10;                          /* ✔ 通过 cp 改数据 */
    printf("   Point *const cp：cp->x = 10 后 cpt.x = %d\n", cpt.x);
    printf("   const Point *pc：pc->x = 20 编译不过，但能读 pc->x = %d\n", pc->x);
    pc = &arr[0];                        /* ✔ 指向可以改 */
    printf("   pc = &arr[0] 之后 pc->x = %d；cp = &arr[0] 不行（指向锁死）\n",
           pc->x);

    const Point *ro = &cpt;
    print_point("print_point 收 const Point *", ro);
    printf("   函数参数里 99%% 的场景是 const Point *：只读 + 省拷贝\n");
    (void)cpc;                           /* 三个都声明了，这里标记已用 */

    /* ---------- 4. 参数：值传递 vs 指针传递 ---------- */
    printf("4. 作参数：值传递改副本，指针传递改原件\n");
    Point base = { 10, 20 };

    Point moved = moved_copy(base, 5, 5);
    print_point("base 本身没变", &base);
    print_point("moved_copy 返回的副本", &moved);

    move_in_place(&base, 5, 5);
    print_point("move_in_place 之后 base", &base);
    printf("   要改调用方就必须传指针；只读就传 const 指针\n");

    /* ---------- 5. 返回值：三种写法 ---------- */
    printf("5. 返回指针：三种安全写法\n");
    Point p1 = { 5, 1 }, p2 = { 2, 9 };
    Point *big = pick_bigger(&p1, &p2);
    printf("   pick_bigger(&p1, &p2) → (%d, %d)（指向调用方的对象）\n",
           big->x, big->y);

    Point *o1 = origin();
    Point *o2 = origin();
    printf("   origin() 两次调用是同一个地址吗？%s（static 对象）\n",
           o1 == o2 ? "是" : "否");

    Point *made = make_point(6, 7);
    if (made != NULL) {
        printf("   make_point(6, 7) → (%d, %d)，调用方负责 free\n",
               made->x, made->y);
        free(made);
        made = NULL;
    }
    printf("   千万别返回局部变量的地址：栈帧回收后就是悬空指针\n");

    /* ---------- 6. 指针遍历数组 ---------- */
    printf("6. 指针遍历：p++ 前进 sizeof(元素) 字节\n");
    Student students[3] = {
        { 101, "alice", 90.5f },
        { 102, "bob",   78.0f },
        { 103, "carol", 88.5f },
    };
    int n = (int)(sizeof students / sizeof students[0]);

    for (Student *s = students; s < students + n; s++) {
        printf("   [%td] #%d %-6s %.1f\n", s - students, s->id, s->name,
               (double)s->score);
    }
    printf("   p++ 走 %zu 字节，不是 1 字节\n", sizeof(Student));

    /* ---------- 7. 指针数组 vs 数组指针 ---------- */
    printf("7. 指针数组 T *a[n] vs 数组指针 T (*a)[n]\n");

    Student *parr[3];                    /* 3 个指针 */
    for (int i = 0; i < n; i++)
        parr[i] = &students[i];

    printf("   sizeof(parr) = %zu（3 个指针）；sizeof(parr[0]) = %zu\n",
           sizeof parr, sizeof parr[0]);

    sort_ptrs_by_score(parr, n);         /* 只交换指针，原数组不动 */
    printf("   按分数排序指针后：");
    for (int i = 0; i < n; i++)
        printf("%s(%.1f)%s", parr[i]->name, (double)parr[i]->score,
               i == n - 1 ? "\n" : " -> ");
    printf("   原数组顺序没变：students[0] 仍是 %s\n", students[0].name);
    printf("   元素大时排指针数组，比搬整个结构体划算得多\n");

    Point grid[2][3] = {
        { { 0, 0 }, { 1, 0 }, { 2, 0 } },
        { { 0, 1 }, { 1, 1 }, { 2, 1 } },
    };
    Point (*row)[3] = grid;              /* 数组指针：指向一整行 */
    printf("   sizeof(row) = %zu（一个指针）；sizeof(*row) = %zu（一整行）\n",
           sizeof row, sizeof *row);
    printf("   row[1][2] = (%d, %d)（第二行第三个）\n",
           row[1][2].x, row[1][2].y);
    printf("   二维数组传参要写 Point a[][3]，第二维不能省\n");

    /* ---------- 8. 动态分配 ---------- */
    printf("8. 动态分配：malloc / calloc / realloc / free\n");

    int cap = 2;
    Student *list = calloc((size_t)cap, sizeof *list);   /* 清零 */
    if (list == NULL) {
        printf("   分配失败，直接返回\n");
        return 1;
    }
    printf("   calloc(2, %zu) 成功，list[0].name 是空串吗？%s\n",
           sizeof *list, list[0].name[0] == '\0' ? "是" : "否");

    list[0].id = 1;
    snprintf(list[0].name, sizeof list[0].name, "%s", "alice");
    list[0].score = 90.5f;

    /* realloc：一定要用临时变量接，失败时原内存还在 */
    int new_cap = 4;
    Student *tmp = realloc(list, (size_t)new_cap * sizeof *tmp);
    if (tmp == NULL) {
        free(list);
        printf("   realloc 失败，原内存已释放\n");
        return 1;
    }
    list = tmp;
    tmp = NULL;

    list[1].id = 2;
    snprintf(list[1].name, sizeof list[1].name, "%s", "bob");
    list[1].score = 78.0f;

    printf("   realloc 到 4 个元素后：");
    for (int i = 0; i < 2; i++)
        printf("#%d %s ", list[i].id, list[i].name);
    printf("\n   malloc 的内容是垃圾值，calloc 才清零；realloc 可能搬地址\n");

    free(list);
    list = NULL;
    printf("   free(list); list = NULL; —— 挡住重复释放和释放后再用\n");

    /* ---------- 9. 小结 ---------- */
    printf("9. 小结：\n");
    printf("    一个地址四种写法：pt.x / (*p).x / p->x / p[0].x\n");
    printf("    *p.x 是错的，. 优先级高于 *，要写 (*p).x 或 p->x\n");
    printf("    指针来源：变量、数组元素、堆、NULL；定义就初始化\n");
    printf("    const 在 * 左边锁数据，在右边锁指针\n");
    printf("    改调用方传指针，只读传 const 指针，大结构体别值传递\n");
    printf("    返回指针只能指向：调用方内存、static、堆；不能指向局部变量\n");
    printf("    p++ 走一个元素（sizeof 字节），不是 1 字节\n");
    printf("    T *a[n] 是指针数组，T (*a)[n] 是数组指针，看括号\n");
    printf("    malloc 要判空、calloc 会清零、realloc 用临时变量、free 后置 NULL\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 05_结构体指针.c -o bin/05_结构体指针
 *     ./bin/05_结构体指针
 *
 * 输出如下：
 *
 * 1. 一个地址，四种写法
 *    pt.x       = 3
 *    (*p).x     = 3（括号不能省）
 *    p->x       = 3（等价，最常用）
 *    p[0].x     = 3（把 p 看成只有一个元素的数组）
 *    *p.x 是错的：. 的优先级高于 *，会被当成 *(p.x)
 * 2. 指针从哪来
 *    q = arr      → q->x = 1, arr[0].x = 1
 *    r = &arr[2]  → r->x = 3, r - arr = 2
 *    (arr + 1)->x = 2（指针算术走 1 个元素）
 *    malloc 的指针：heap->x = 7（用完必须 free）
 *    NULL 指针：nullp == NULL → 真（解引用前必须先判空）
 * 3. const 与指针：左边锁数据，右边锁指针
 *    Point *const cp：cp->x = 10 后 cpt.x = 10
 *    const Point *pc：pc->x = 20 编译不过，但能读 pc->x = 10
 *    pc = &arr[0] 之后 pc->x = 1；cp = &arr[0] 不行（指向锁死）
 *    print_point 收 const Point * = (10, 2)
 *    函数参数里 99% 的场景是 const Point *：只读 + 省拷贝
 * 4. 作参数：值传递改副本，指针传递改原件
 *    base 本身没变 = (10, 20)
 *    moved_copy 返回的副本 = (15, 25)
 *    move_in_place 之后 base = (15, 25)
 *    要改调用方就必须传指针；只读就传 const 指针
 * 5. 返回指针：三种安全写法
 *    pick_bigger(&p1, &p2) → (5, 1)（指向调用方的对象）
 *    origin() 两次调用是同一个地址吗？是（static 对象）
 *    make_point(6, 7) → (6, 7)，调用方负责 free
 *    千万别返回局部变量的地址：栈帧回收后就是悬空指针
 * 6. 指针遍历：p++ 前进 sizeof(元素) 字节
 *    [0] #101 alice  90.5
 *    [1] #102 bob    78.0
 *    [2] #103 carol  88.5
 *    p++ 走 24 字节，不是 1 字节
 * 7. 指针数组 T *a[n] vs 数组指针 T (*a)[n]
 *    sizeof(parr) = 24（3 个指针）；sizeof(parr[0]) = 8
 *    按分数排序指针后：alice(90.5) -> carol(88.5) -> bob(78.0)
 *    原数组顺序没变：students[0] 仍是 alice
 *    元素大时排指针数组，比搬整个结构体划算得多
 *    sizeof(row) = 8（一个指针）；sizeof(*row) = 24（一整行）
 *    row[1][2] = (2, 1)（第二行第三个）
 *    二维数组传参要写 Point a[][3]，第二维不能省
 * 8. 动态分配：malloc / calloc / realloc / free
 *    calloc(2, 24) 成功，list[0].name 是空串吗？是
 *    realloc 到 4 个元素后：#1 alice #2 bob
 *    malloc 的内容是垃圾值，calloc 才清零；realloc 可能搬地址
 *    free(list); list = NULL; —— 挡住重复释放和释放后再用
 * 9. 小结：
 *     一个地址四种写法：pt.x / (*p).x / p->x / p[0].x
 *     *p.x 是错的，. 优先级高于 *，要写 (*p).x 或 p->x
 *     指针来源：变量、数组元素、堆、NULL；定义就初始化
 *     const 在 * 左边锁数据，在右边锁指针
 *     改调用方传指针，只读传 const 指针，大结构体别值传递
 *     返回指针只能指向：调用方内存、static、堆；不能指向局部变量
 *     p++ 走一个元素（sizeof 字节），不是 1 字节
 *     T *a[n] 是指针数组，T (*a)[n] 是数组指针，看括号
 *     malloc 要判空、calloc 会清零、realloc 用临时变量、free 后置 NULL
 * ==========================================================================
 */
