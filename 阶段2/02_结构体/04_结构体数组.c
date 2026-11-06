/*
 * 04_结构体数组.c —— 阶段2 / 第 02 课 / 第 04 个文件：结构体数组
 *
 * 编译：gcc -std=c99 -Wall -Wextra 04_结构体数组.c -o bin/04_结构体数组
 * 运行：./bin/04_结构体数组
 *
 * 01 篇只在"使用"里出现过一行 `Point pts[3]`，02 篇讲的是"结构体里有数组"。
 * 这一篇反过来：**数组里装结构体**。
 *
 *      结构体数组 = 一批同类型的记录，内存里连续排列
 *
 * 这是 C 里表示"一张表"最直接的方式：学生名单、成绩表、顶点数组、
 * 任务队列、缓冲区池……都是结构体数组。
 *
 * ====================== 一、声明与定义 ======================
 * 把普通数组的元素类型换成结构体即可：
 *
 *      struct Point pts[3];                 // 3 个 Point，未初始化
 *      struct Point pts[3] = {              // 完整初始化
 *          { 1, 1 }, { 2, 2 }, { 3, 3 }
 *      };
 *
 * 四种初始化写法：
 *
 * (1) **按顺序逐个元素给值**
 *          Point pts[3] = { { 1, 1 }, { 2, 2 }, { 3, 3 } };
 *
 * (2) **指定下标**（C99；跳过某些元素时特别有用）
 *          Point pts[3] = { [0] = { 1, 1 }, [2] = { 3, 3 } };
 *          // pts[1] 自动变成 { 0, 0 }
 *
 * (3) **元素内部再用指定初始化器**（成员多时最清楚）
 *          Student one[2] = {
 *              { .id = 1, .name = "alice", .score = 90.5f },
 *              { .id = 2, .name = "bob",   .score = 78.0f },
 *          };
 *
 * (4) **全部清零**
 *          Student all[5] = { 0 };          // 5 个元素的所有成员都是 0
 *
 * ★ 长度别手写数字，用 sizeof 算：
 *      int n = (int)(sizeof students / sizeof students[0]);
 *   以后加删元素，这一行不用改。
 *
 * ★ 局部数组不初始化时，**每个元素的每个成员都是垃圾值**，
 *   和单个结构体一样，别指望"数组会自动清零"。
 *
 * ====================== 二、访问：下标、指针、首元素 ======================
 * (1) **下标 + 点**（最常用）
 *          pts[i].x
 *      `[]` 优先级高于 `.`：先取元素，再取成员。
 *
 * (2) **指针 + 箭头**
 *          Point *p = &pts[i];
 *          p->x                 // 等价于 pts[i].x
 *
 * (3) **数组名当指针用**
 *          Point *p = pts;      // 等价于 &pts[0]
 *          p->x                 // 等价于 pts[0].x
 *          pts->x               // 同上，首元素的成员
 *          (pts + 1)->x         // 等价于 pts[1].x
 *
 * (4) **指针遍历**
 *          for (Point *p = pts; p < pts + n; p++)
 *              printf("%d\n", p->x);
 *      终止条件写 `p < pts + n`，写成 `<=` 就越界了。
 *
 * ★ 注意 `pts` 和 `&pts` 的区别：
 *      pts      类型是 `Point *`，+1 走一个 Point
 *      &pts     类型是 `Point (*)[3]`（数组指针），+1 走整个数组
 *   两者**地址值相同、类型不同**，指针算术的结果差很多。
 *
 * ====================== 三、内存布局：连续、等距 ======================
 * 结构体数组是一整块连续内存，每个元素大小相同（都含填充）：
 *
 *      sizeof(arr)            = n × sizeof(元素)
 *      &arr[1] - &arr[0]      = sizeof(元素) 字节
 *
 * 由此得到几个结论：
 *  - 可以整体 memset / memcpy（同类型时）
 *  - 第 i 个元素的地址 = 首地址 + i × sizeof(元素)，下标就是指针算术的语法糖
 *  - 元素越大，数组越占内存，**排序和搬移的成本也越高**（要拷整块）
 *
 * ★ sizeof(元素) 含填充，而数组会把这个浪费**乘以 n**：
 *      struct Entry { char flag; int value; };   // sizeof = 8，不是 5
 *   10 万个元素就白扔 300KB。所以 03 篇的布局优化，在数组里收益是乘 n 的。
 *
 * ★ 求长度只在数组**还在作用域里**时有效：
 *      int n = sizeof arr / sizeof arr[0];       // ✔ 数组类型完整
 *   一旦传进函数，arr 退化成指针，`sizeof arr` 就变成 8（指针大小）。
 *
 * ====================== 四、传参与返回 ======================
 * (1) **传参**：数组退化为指针，**长度必须单独传**
 *          void print_all(const Student *a, int n);
 *      `void print_all(Student a[])` 和 `Student *a` 完全等价 ——
 *      方括号里写 `[5]` 也只是注释，编译器一律当指针。
 *
 * (2) **加 const**：只读就写 `const Student *a`
 *      既能接受普通数组，也明确表态"我不改你的元素"。
 *
 * (3) **不能返回数组**。三种替代：
 *        - 调用方传缓冲区，函数往里填：`void fill_points(Point *out, int n);`
 *        - 返回指针（指向 static / 堆 / 调用方的数组）
 *        - 把数组包进结构体，返回结构体（结构体可以按值返回）：
 *              typedef struct { Point v[3]; } PointSet;
 *              PointSet make_triangle(void);     // ✔ 合法，整块拷回
 *
 * ====================== 五、常见操作 ======================
 * (1) **遍历统计**：求和、平均
 *          long sum = 0;
 *          for (int i = 0; i < n; i++) sum += a[i].value;
 *
 * (2) **找最大/最小：记下标，不是记值**
 *          int best = 0;
 *          for (int i = 1; i < n; i++)
 *              if (a[i].score > a[best].score) best = i;
 *      记下标的好处：既拿到值，又知道它原来的位置。
 *
 * (3) **线性查找：返回指针**
 *          for (int i = 0; i < n; i++)
 *              if (a[i].id == key) return &a[i];
 *          return NULL;
 *      返回指针而不是下标：调用方可以直接用 `p->name`，也能判断 NULL。
 *
 * ====================== 六、排序：交换整个元素，还是交换指针 ======================
 * 结构体数组最常见的操作就是排序。两种做法，代价差别很大：
 *
 * (1) **原地交换元素**（简单，但每次交换要拷 2 份 sizeof(元素)）
 *          if (a[j].score < a[j+1].score) {
 *              Student t = a[j];
 *              a[j] = a[j+1];
 *              a[j+1] = t;
 *          }
 *      元素 24 字节 → 每次交换搬 72 字节。元素越大越亏。
 *
 * (2) **qsort + 比较函数**（标准库，内部也是搬元素，但经过高度优化）
 *          qsort(a, n, sizeof a[0], cmp);
 *
 *          int cmp(const void *pa, const void *pb)
 *          {
 *              const Student *x = pa, *y = pb;    // void * 能隐式转成任意对象指针
 *              if (x->score < y->score) return -1;
 *              if (x->score > y->score) return  1;
 *              return 0;
 *          }
 *      比较函数必须遵守约定：<0 排前面、0 相等、>0 排后面。
 *      浮点数别用 `x->score - y->score` 当返回值 —— 相减会丢精度、还会溢出。
 *
 * (3) **结构体很大时：排指针数组 / 索引数组**
 *          const Student *ptrs[5];
 *          for (int i = 0; i < 5; i++) ptrs[i] = &students[i];
 *          // 排序时只交换 8 字节的指针，原数组不动
 *      代价是多一层间接、原数组顺序不变（这有时正是你要的）。
 *
 * ★ qsort 的函数指针参数，语法见 09 篇（函数指针）第五节。
 *
 * ====================== 七、字符串字段：char 数组 vs char 指针 ======================
 * 结构体数组里放字符串，两种写法，拷贝语义完全不同：
 *
 *      struct A { char  name[16]; };    // 数组：内容在自己肚子里
 *      struct B { char *name;     };    // 指针：只存一个地址
 *
 *      A a1 = { "alice" }, a2 = a1;     // a2.name 是独立副本
 *      a2.name[0] = 'A';                // a1.name 还是 "alice"
 *
 *      B b1 = { buf }, b2 = b1;         // b2.name 和 b1.name 指向同一块
 *      b2.name[0] = 'A';                // b1.name 也变了
 *
 * 数组字段的赋值/拷贝规则：
 *  - **不能** `s.name = "bob";`（数组不能整体赋值）
 *  - 要改内容用 `strcpy` / `snprintf`，并确认长度够
 *  - 结构体整体赋值时，char 数组会被**逐元素拷贝**（真正的深拷贝）
 *
 * ★ 长度控制：`snprintf(s.name, sizeof s.name, "%s", src)` 最稳，
 *   至少不会越界写（超长会截断，这是可接受的失败方式）。
 *
 * ====================== 八、常见坑 ======================
 *  1. 函数里 `sizeof arr` 当数组长度         退化后是 8 字节指针
 *  2. 传参不传长度                           函数无法知道有几个元素
 *  3. `p <= pts + n` 遍历                    多走一步，越界读
 *  4. 用 `[]` 写 `void f(Student a[5])` 以为有长度检查   等价于指针，没有检查
 *  5. 结构体数组直接 `=` 整体赋值            数组不能赋值，只能逐元素或 memcpy
 *  6. `s.name = "bob"`                       数组不能整体赋值，用 strcpy/snprintf
 *  7. 排序时只交换部分字段                    会破坏"一条记录"的完整性
 *  8. 大元素数组用冒泡排序                    每次交换拷两份整块，改指针/索引
 *  9. 忘记元素含填充                         按成员和估算数组总大小，会少算
 * 10. 返回局部数组的指针                     栈帧销毁，悬空
 *
 * ====================== 九、实践建议 ======================
 *  - 长度用 `sizeof arr / sizeof arr[0]` 算，别写魔法数字
 *  - 所有"数组 + 长度"的接口，长度紧跟数组参数，类型用 `int` 或 `size_t` 统一
 *  - 只读入参一律 `const T *`，明确不改元素
 *  - 元素大（>24 字节）又要频繁排序，就排指针数组或索引数组
 *  - 字符串字段优先用 `char[N]` + `snprintf`，省掉一次分配和一次释放
 *  - 需要一个"固定大小的表"返回给调用方时，包进结构体按值返回
 *  - 数组和长度是天生一对：**永远一起传，永远一起检查**
 */

#include <stdio.h>
#include <string.h>      /* snprintf */
#include <stdlib.h>      /* qsort */

/* ====================== 记录类型 ====================== */

typedef struct Student {
    int   id;
    char  name[16];      /* 数组字段：名字真的装在这里 */
    float score;
} Student;

typedef struct Point {
    int x;
    int y;
} Point;

/* 演示"填充在数组里被乘以 n" */
typedef struct Entry {
    char flag;
    int  value;
} Entry;

/* 包进结构体，才能按值返回一个数组 */
typedef struct PointSet {
    Point v[3];
} PointSet;

/* ====================== 对结构体数组的常见操作 ====================== */

static void print_students(const char *title, const Student *a, int n)
{
    printf("   %s\n", title);
    for (int i = 0; i < n; i++)
        printf("      [%d] #%d %-6s %.1f\n", i, a[i].id, a[i].name,
               (double)a[i].score);
}

static double average_score(const Student *a, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += a[i].score;
    return n > 0 ? sum / n : 0.0;
}

/* 找最高分：记下标，既拿到值也知道位置 */
static int best_index(const Student *a, int n)
{
    int best = 0;
    for (int i = 1; i < n; i++)
        if (a[i].score > a[best].score)
            best = i;
    return best;
}

/* 线性查找：返回元素指针，找不到返回 NULL */
static const Student *find_by_id(const Student *a, int n, int id)
{
    for (int i = 0; i < n; i++)
        if (a[i].id == id)
            return &a[i];
    return NULL;
}

/* 冒泡排序：交换的是**整个元素** */
static void sort_by_score_desc(Student *a, int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (a[j].score < a[j + 1].score) {
                Student t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
        }
    }
}

/* qsort 的比较函数：<0 排前、0 相等、>0 排后；浮点别用减法 */
static int cmp_score_asc(const void *pa, const void *pb)
{
    const Student *x = (const Student *)pa;
    const Student *y = (const Student *)pb;

    if (x->score < y->score) return -1;
    if (x->score > y->score) return  1;
    return 0;
}

/* 调用方给缓冲区，函数负责填：数组不能返回，这是标准做法 */
static void fill_points(Point *out, int n)
{
    for (int i = 0; i < n; i++) {
        out[i].x = i;
        out[i].y = i * i;
    }
}

/* 把数组包进结构体，就能按值返回 */
static PointSet make_triangle(void)
{
    PointSet s = { { { 0, 0 }, { 4, 0 }, { 0, 3 } } };
    return s;
}

int main(void)
{
    /* ---------- 1. 定义与初始化 ---------- */
    printf("1. 定义与初始化\n");

    Point pts[3] = { { 1, 1 }, { 2, 4 }, { 3, 9 } };          /* ① 按序 */
    Point sparse[3] = { [0] = { 10, 10 }, [2] = { 30, 30 } };  /* ② 指定下标 */
    Point none[3] = { 0 };                                     /* ④ 全清零 */

    Student students[5] = {                                    /* ③ 元素内指定成员 */
        { .id = 101, .name = "alice", .score = 90.5f },
        { .id = 102, .name = "bob",   .score = 78.0f },
        { .id = 103, .name = "carol", .score = 88.5f },
        { .id = 104, .name = "dave",  .score = 95.0f },
        { .id = 105, .name = "eve",   .score = 66.5f },
    };

    /* 按序原样填满：不写内层花括号也能编译，但 -Wextra 会建议加上 */
    Student students1[5] = {
        { 101, "alice", 90.5f }, { 102, "bob", 78.0f },
        { 103, "carol", 88.5f }, { 104, "dave", 95.0f },
        { 105, "eve", 66.5f },
    };

    int n = (int)(sizeof students / sizeof students[0]);
    printf("   ① 按序：pts[1] = (%d, %d)\n", pts[1].x, pts[1].y);
    printf("   ② 指定下标：sparse[1] = (%d, %d)（没指定就补 0）\n",
           sparse[1].x, sparse[1].y);
    printf("   ③ 元素内指定成员：students[0] = #%d %s\n",
           students[0].id, students[0].name);
    printf("   ④ { 0 }：none[2] = (%d, %d)\n", none[2].x, none[2].y);
    printf("   长度：sizeof(students) / sizeof(students[0]) = %d\n", n);
    printf("   按序填充同样能填满：students1[4] = #%d %s\n",
           students1[4].id, students1[4].name);

    /* ---------- 2. 三种访问方式 ---------- */
    printf("2. 访问：下标 / 指针 / 首元素\n");
    printf("   pts[2].x          = %d\n", pts[2].x);

    Point *p = &pts[2];
    printf("   (&pts[2])->x      = %d\n", p->x);

    printf("   pts->x            = %d（首元素）\n", pts->x);
    printf("   (pts + 1)->x      = %d（等价于 pts[1].x）\n", (pts + 1)->x);

    printf("   指针遍历：");
    for (Point *q = pts; q < pts + 3; q++)
        printf("(%d,%d)%s", q->x, q->y, q == pts + 2 ? "\n" : " ");
    printf("   注意终止条件写 q < pts + n，写成 <= 就越界\n");

    /* ---------- 3. 内存布局 ---------- */
    printf("3. 内存布局：连续、等距\n");
    printf("   sizeof(Student) = %zu 字节（int + char[16] + float）\n",
           sizeof(Student));
    printf("   sizeof(students) = %zu = %d × %zu\n",
           sizeof(students), n, sizeof(Student));
    printf("   &students[1] - &students[0] = %zu 字节（正好一个元素）\n",
           (size_t)((const char *)&students[1] - (const char *)&students[0]));
    printf("   下标就是指针算术：地址 = 首地址 + i × sizeof(元素)\n");

    Entry entries[3] = { { 'a', 1 }, { 'b', 2 }, { 'c', 3 } };
    printf("   sizeof(Entry) = %zu，但成员只有 %zu 字节\n",
           sizeof(Entry), sizeof(char) + sizeof(int));
    printf("   3 个元素共 %zu 字节，其中 %zu 字节是填充（每个元素都带着）\n",
           sizeof(entries), sizeof(entries) - 3 * (sizeof(char) + sizeof(int)));
    printf("   元素里有填充时，浪费会被乘以 n\n");

    /* ---------- 4. 传参与返回 ---------- */
    printf("4. 传参与返回\n");
    printf("   传 const Student *a + int n：数组退化，长度必须单独给\n");
    print_students("print_students(students, 5)", students, n);

    Point filled[3];
    fill_points(filled, 3);                 /* 调用方给缓冲区 */
    printf("   fill_points(out, 3) → ");
    for (int i = 0; i < 3; i++)
        printf("(%d,%d)%s", filled[i].x, filled[i].y, i == 2 ? "\n" : " ");

    PointSet tri = make_triangle();         /* 包进结构体，可以按值返回 */
    printf("   make_triangle() → (%d,%d) (%d,%d) (%d,%d)\n",
           tri.v[0].x, tri.v[0].y, tri.v[1].x, tri.v[1].y, tri.v[2].x, tri.v[2].y);

    /* ---------- 5. 统计与查找 ---------- */
    printf("5. 统计与查找\n");
    printf("   平均分 = %.1f\n", average_score(students, n));

    int best = best_index(students, n);
    printf("   最高分：students[%d] = %s %.1f（记下标，位置也拿到了）\n",
           best, students[best].name, (double)students[best].score);

    const Student *hit = find_by_id(students, n, 103);
    if (hit != NULL)
        printf("   找 id=103 → %s（返回元素指针，可直接用 ->）\n", hit->name);
    else
        printf("   找 id=103 → 没找到\n");

    hit = find_by_id(students, n, 999);
    printf("   找 id=999 → %s（找不到返回 NULL，调用方必须判空）\n",
           hit == NULL ? "NULL" : "非空");

    /* ---------- 6. 排序 ---------- */
    printf("6. 排序：交换整个元素 vs qsort\n");

    Student by_bubble[5];
    Student by_qsort[5];
    for (int i = 0; i < n; i++) {           /* 元素整体赋值，char 数组也被复制 */
        by_bubble[i] = students[i];
        by_qsort[i]  = students[i];
    }

    sort_by_score_desc(by_bubble, n);       /* 冒泡：每次交换拷两份元素 */
    print_students("sort_by_score_desc（冒泡，降序）", by_bubble, n);
    printf("      每次交换拷 2 × %zu 字节；元素越大越亏\n", sizeof(Student));

    qsort(by_qsort, (size_t)n, sizeof by_qsort[0], cmp_score_asc);
    print_students("qsort + cmp_score_asc（升序）", by_qsort, n);
    printf("      比较函数遵守 <0 / 0 / >0 的约定，浮点别用相减\n");

    printf("   原数组没被改动：students[0] 仍是 %s %.1f\n",
           students[0].name, (double)students[0].score);

    /* ---------- 7. 字符串字段：数组 vs 指针 ---------- */
    printf("7. 字符串字段：char 数组是复制，char 指针是共享\n");

    Student copied[5];
    for (int i = 0; i < n; i++)
        copied[i] = students[i];            /* 整体赋值：name 数组被逐元素拷贝 */
    copied[0].name[0] = 'A';
    printf("   改 copied[0].name[0] 后 students[0].name = %s（独立副本）\n",
           students[0].name);
    printf("   char name[16] 是数组字段，赋值时内容被真正复制\n");

    char buf[] = "dave";                    /* 必须是可写数组，不能是字面量 */
    struct PStudent { char *name; int id; } q1 = { buf, 4 };
    struct PStudent q2 = q1;                /* 只拷指针 */
    q2.name[0] = 'D';
    printf("   char *name 是指针字段，改 q2 后 q1.name = %s（同一块内存）\n",
           q1.name);

    char renamed[16];
    snprintf(renamed, sizeof renamed, "%s", "bobby");   /* 数组字段改内容只能靠函数 */
    printf("   改数组字段：snprintf(s.name, sizeof s.name, ...) → %s\n", renamed);
    printf("   s.name = \"bob\" 这种写法是非法的：数组不能整体赋值\n");

    /* ---------- 8. 小结 ---------- */
    printf("8. 小结：\n");
    printf("    结构体数组 = 连续存放的一批记录，下标就是指针算术\n");
    printf("    长度为 sizeof arr / sizeof arr[0]，传参时必须另外传\n");
    printf("    访问三种：a[i].x、p->x、pts->x（首元素）\n");
    printf("    元素含填充，数组会把浪费乘以 n，布局优化收益更大\n");
    printf("    函数不能返回数组：传缓冲区、返指针，或包进结构体返回值\n");
    printf("    排序可原地交换元素，也可 qsort；元素大就排指针/索引数组\n");
    printf("    char name[N] 字段赋值是真复制，char *name 字段赋值只拷地址\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 04_结构体数组.c -o bin/04_结构体数组
 *     ./bin/04_结构体数组
 *
 * 输出如下：
 *
 * 1. 定义与初始化
 *    ① 按序：pts[1] = (2, 4)
 *    ② 指定下标：sparse[1] = (0, 0)（没指定就补 0）
 *    ③ 元素内指定成员：students[0] = #101 alice
 *    ④ { 0 }：none[2] = (0, 0)
 *    长度：sizeof(students) / sizeof(students[0]) = 5
 *    按序填充同样能填满：students1[4] = #105 eve
 * 2. 访问：下标 / 指针 / 首元素
 *    pts[2].x          = 3
 *    (&pts[2])->x      = 3
 *    pts->x            = 1（首元素）
 *    (pts + 1)->x      = 2（等价于 pts[1].x）
 *    指针遍历：(1,1) (2,4) (3,9)
 *    注意终止条件写 q < pts + n，写成 <= 就越界
 * 3. 内存布局：连续、等距
 *    sizeof(Student) = 24 字节（int + char[16] + float）
 *    sizeof(students) = 120 = 5 × 24
 *    &students[1] - &students[0] = 24 字节（正好一个元素）
 *    下标就是指针算术：地址 = 首地址 + i × sizeof(元素)
 *    sizeof(Entry) = 8，但成员只有 5 字节
 *    3 个元素共 24 字节，其中 9 字节是填充（每个元素都带着）
 *    元素里有填充时，浪费会被乘以 n
 * 4. 传参与返回
 *    传 const Student *a + int n：数组退化，长度必须单独给
 *    print_students(students, 5)
 *       [0] #101 alice  90.5
 *       [1] #102 bob    78.0
 *       [2] #103 carol  88.5
 *       [3] #104 dave   95.0
 *       [4] #105 eve    66.5
 *    fill_points(out, 3) → (0,0) (1,1) (2,4)
 *    make_triangle() → (0,0) (4,0) (0,3)
 * 5. 统计与查找
 *    平均分 = 83.7
 *    最高分：students[3] = dave 95.0（记下标，位置也拿到了）
 *    找 id=103 → carol（返回元素指针，可直接用 ->）
 *    找 id=999 → NULL（找不到返回 NULL，调用方必须判空）
 * 6. 排序：交换整个元素 vs qsort
 *    sort_by_score_desc（冒泡，降序）
 *       [0] #104 dave   95.0
 *       [1] #101 alice  90.5
 *       [2] #103 carol  88.5
 *       [3] #102 bob    78.0
 *       [4] #105 eve    66.5
 *       每次交换拷 2 × 24 字节；元素越大越亏
 *    qsort + cmp_score_asc（升序）
 *       [0] #105 eve    66.5
 *       [1] #102 bob    78.0
 *       [2] #103 carol  88.5
 *       [3] #101 alice  90.5
 *       [4] #104 dave   95.0
 *       比较函数遵守 <0 / 0 / >0 的约定，浮点别用相减
 *    原数组没被改动：students[0] 仍是 alice 90.5
 * 7. 字符串字段：char 数组是复制，char 指针是共享
 *    改 copied[0].name[0] 后 students[0].name = alice（独立副本）
 *    char name[16] 是数组字段，赋值时内容被真正复制
 *    char *name 是指针字段，改 q2 后 q1.name = Dave（同一块内存）
 *    改数组字段：snprintf(s.name, sizeof s.name, ...) → bobby
 *    s.name = "bob" 这种写法是非法的：数组不能整体赋值
 * 8. 小结：
 *     结构体数组 = 连续存放的一批记录，下标就是指针算术
 *     长度为 sizeof arr / sizeof arr[0]，传参时必须另外传
 *     访问三种：a[i].x、p->x、pts->x（首元素）
 *     元素含填充，数组会把浪费乘以 n，布局优化收益更大
 *     函数不能返回数组：传缓冲区、返指针，或包进结构体返回值
 *     排序可原地交换元素，也可 qsort；元素大就排指针/索引数组
 *     char name[N] 字段赋值是真复制，char *name 字段赋值只拷地址
 * ==========================================================================
 */
