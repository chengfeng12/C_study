/*
 * 02_动态内存分配.c —— 阶段2 / 第 06 课 / 第 02 个文件：malloc / calloc / realloc / free
 *
 * 编译：gcc -std=c99 -Wall -Wextra 02_动态内存分配.c -o bin/02_动态内存分配
 * 运行：./bin/02_动态内存分配
 *
 * 上一篇讲了内存分几块，这一篇讲**怎么用堆**。
 * 堆是唯一"生命周期由你说了算"的存储区：想让它活多久就活多久，
 * 代价是**你必须亲手关灯**（free），否则就是内存泄漏。
 *
 * ====================== 一、什么时候必须用堆 ======================
 * 三个判据，满足任意一个就该上堆；都不满足就用栈或静态区。
 *
 *  (1) **大小在运行期才知道**
 *          int n;  scanf("%d", &n);
 *          int *a = malloc(n * sizeof *a);        // 编译期不知道 n
 *      栈上做不到：int a[n] 是 VLA（C99 可选特性），大小受限、不可重入、
 *      出了作用域就没，而且爆栈风险更高。
 *
 *  (2) **要活过当前函数**
 *          Node *node_new(int v) { Node *n = malloc(sizeof *n); ... return n; }
 *      函数返回后栈上的东西全没了，只有堆对象还能被外面拿到。
 *
 *  (3) **对象特别大**
 *          int *big = malloc(10 * 1024 * 1024);   // 10 MB
 *      栈通常只有 1–8 MB，放不下；大缓冲区一律走堆。
 *
 * ★ 反过来：大小固定、只在函数内用的小对象，**放栈上**。
 *   栈的分配只是挪一下栈指针，比 malloc 快一到两个数量级，还不会泄漏。
 *
 * ====================== 二、四个函数 ======================
 *      void *malloc (size_t size);
 *          分配 size 字节，**内容不确定**（可能是脏数据）。失败返回 NULL。
 *
 *      void *calloc (size_t count, size_t size);
 *          分配 count 个 size 字节，**全部清零**。
 *          内部会检查 count * size 是否溢出（malloc(n*m) 自己乘会溢出）。
 *
 *      void *realloc(void *ptr, size_t size);
 *          把 ptr 指向的块调整到 size 字节：
 *            - 能原地扩就在原地扩（可能返回同一个地址）
 *            - 扩不了就新开一块、**搬运内容**、释放旧块（返回新地址）
 *            - ptr 传 NULL 时等价于 malloc(size)
 *          失败返回 NULL，**且原块保持有效、不会被释放**。
 *
 *      void  free   (void *ptr);
 *          释放 malloc/calloc/realloc 得到的块。ptr 为 NULL 时什么都不做。
 *
 * ★ 四个函数都在 <stdlib.h> 里，忘包含头文件是"隐式声明 int malloc()"的老坑。
 * ★ C 里 **不要给 malloc 返回值加强转**：
 *        int *p = (int *)malloc(n * sizeof *p);   // 啰嗦，还会掩盖漏头文件的错误
 *        int *p = malloc(n * sizeof *p);          // ✔ C 的标准写法
 *   （C++ 才必须强转，因为在 C++ 里 void* 不会自动转成 T*。）
 *
 * ====================== 三、标准五步流程 ======================
 *      int *p = malloc(n * sizeof *p);       // 1. 申请，用 sizeof *p 算元素大小
 *      if (p == NULL) { 处理错误; }           // 2. 判空，**任何 malloc 都不能省**
 *      p[0] = 1;                             // 3. 使用：此时才有权读写
 *      free(p);                              // 4. 释放：每个成功申请恰好 free 一次
 *      p = NULL;                             // 5. 置空：防止野指针
 *
 * ★ 第 2 步是被初学者跳过最多的一步。malloc 失败在桌面机上少见，
 *   但在嵌入式、容器内存受限时会真实发生，不判空就是空指针解引用。
 *
 * ====================== 四、malloc 与 calloc ======================
 *      int *a = malloc(n * sizeof *a);   // 内容不确定，不能假设是 0
 *      int *b = calloc(n, sizeof *b);    // 保证全是 0
 *
 * 需要"清零的数组"就用 calloc：比 malloc + memset 少一次遍历，
 * 而且它还会替你检查 n * sizeof(T) 的乘法溢出。
 *
 * ★ 反过来，需要自己填满整个数组时用 malloc 更划算 —— 反正要覆盖。
 *
 * ====================== 五、realloc 的两个坑 ======================
 * 陷阱 1：**把返回值直接赋回原指针**
 *      p = realloc(p, new_size);         // ✗ 失败时 p 变 NULL，原块地址彻底丢失 → 泄漏
 *      int *tmp = realloc(p, new_size);  // ✔ 先接住
 *      if (tmp == NULL) { ... }          // 此时 p 仍有效，可以继续用或收尾释放
 *      else             { p = tmp; }
 *
 * 陷阱 2：**realloc 可能搬家**
 *      int *q = p;                       // 别在 realloc 前留别名
 *      p = realloc(p, ...);              // 一旦搬家，q 变成悬垂指针
 *
 * 另外两个边界：
 *      realloc(p, 0)    行为由实现决定（可能 free 掉并返回 NULL，也可能返回
 *                       一个不能解引用的指针）→ **不要用**，要释放就 free(p)
 *      malloc(0)        可能返回 NULL，也可能返回一个"不能解引用"的唯一指针
 *                       → 别依赖，要 0 个元素就别申请
 *
 * ★ realloc 的扩容成本：可能整块搬运 O(n)。
 *   所以动态数组一律**倍增扩容**（4 → 8 → 16 …），
 *   摊还下来每次 push 是 O(1)，不会变成 O(n²)。
 *
 * ====================== 六、free 的四条规矩 ======================
 *  1. 只能释放"堆指针"，而且必须是 malloc/calloc/realloc 返回的那个**原始值**
 *        int a[10];        free(a);          // ✗ 栈地址，堆管理器直接崩
 *        char *s = "hi";   free(s);          // ✗ 常量区地址
 *        int *p = malloc(10*sizeof *p);
 *        free(p + 1);                        // ✗ 不是原始地址，UB
 *
 *  2. free(NULL) 是合法的，什么都不做 —— 所以"释放+置空"最安全：
 *        free(p); p = NULL;                  // 重复执行也不会出错
 *
 *  3. **不要重复释放**（double free）：UB，常被堆管理器检测到并直接 abort。
 *     两个指针指向同一块时最容易踩：
 *        int *a = malloc(...); int *b = a;
 *        free(a); free(b);                   // ✗ 同一块释放两次
 *
 *  4. 释放后**立刻置空**，否则那个指针就是野指针（dangling pointer）：
 *        free(p);  p[0] = 1;                 // ✗ 释放后再写，UB（可能"看起来正常"）

 * ★ 还有"忘了释放"：内存泄漏。进程退出时系统会收回，但长期运行的服务
 *   （循环里反复 malloc 不 free）会把内存吃光。
 *
 * ====================== 七、sizeof 的三种写法 ======================
 *      int *p = malloc(10 * sizeof(int));    // 可以，但类型改了就忘改
 *      int *p = malloc(10 * sizeof *p);      // ✔ 推荐：跟着指针类型自动变
 *      int *p = malloc(10 * sizeof(p));      // ✗ 这是指针大小(8 字节)！只能装 2 个 int
 *
 *      struct Node *n = malloc(sizeof *n);   // ✔ 结构体同理
 *      struct Node *n = malloc(sizeof(struct Node));  // 也行，但上面更短更安全
 *
 * ★ `sizeof p` = 指针本身大小；`sizeof *p` = 指向对象的大小。差一个星号，后果是越界。
 *
 * ====================== 八、常见错误速查 ======================
 *   1. 不判 malloc 返回值是否 NULL               → 空指针解引用
 *   2. sizeof(ptr) 当 sizeof(*ptr) 用            → 分配只有 1/2 大小，越界写
 *   3. 忘了 free                                 → 泄漏（循环里最致命）
 *   4. 双重 free                                 → abort / UB
 *   5. free 后继续用（野指针）                   → 读到脏数据或崩溃
 *   6. free 栈地址、字面量、p+1                  → 堆管理器崩溃
 *   7. p = realloc(p, ...) 失败丢指针            → 泄漏
 *   8. 越界写 p[n]（合法下标是 0..n-1）          → 踩坏堆元数据，崩溃点常常离现场很远
 *   9. 用未初始化的 malloc 内存（以为清零了）    → 随机 bug
 *  10. 一次 malloc 多次 free / 一部分 free      → 堆结构损坏
 *
 * ====================== 九、查错工具 ======================
 * 肉眼找内存错误效率极低，直接上工具：
 *
 *      gcc -std=c99 -g -fsanitize=address 02_动态内存分配.c -o bin/demo_asan
 *      ./bin/demo_asan
 *
 * ASan 能抓到：越界读/写、use-after-free、double free、内存泄漏（退出时打印）。
 * Linux 上还可以用 valgrind：
 *      valgrind --leak-check=full ./bin/02_动态内存分配
 *
 * ★ 写完带 malloc 的代码，跑一遍 ASan 花 10 秒，比盯代码 10 分钟有用。
 */

#include <stdio.h>
#include <stdlib.h>     /* malloc / calloc / realloc / free / exit */
#include <string.h>     /* strlen / memcpy */

/* ====================== 实战组件一：动态数组 ====================== */
/*
 * 最小的"可增长 int 数组"，把 malloc/realloc/free 三件套包成一个接口。
 * 设计要点：
 *   - data 初始为 NULL、cap 为 0，第一次 push 时才真正分配（惰性）
 *   - 扩容按 2 倍增长 → 摊还 O(1)
 *   - 分配失败时保持原状返回 0，绝不让 v->data 变 NULL（防泄漏）
 */
typedef struct IntVec {
    int    *data;
    size_t  len;
    size_t  cap;
} IntVec;

/* 保证至少有 need 个元素的空间；成功返回 1，失败返回 0 */
static int vec_reserve(IntVec *v, size_t need)
{
    size_t new_cap;
    int   *tmp;

    if (need <= v->cap)
        return 1;                       /* 容量够，什么都不做 */

    new_cap = v->cap ? v->cap * 2 : 4;  /* 空则给 4，否则翻倍 */
    while (new_cap < need)              /* 一次要得太多就继续翻 */
        new_cap *= 2;

    tmp = realloc(v->data, new_cap * sizeof *tmp);   /* ★ 用临时指针接住 */
    if (tmp == NULL)
        return 0;                       /* 失败：v->data 原封不动，没有泄漏 */

    v->data = tmp;
    v->cap  = new_cap;
    return 1;
}

static int vec_push(IntVec *v, int value)
{
    if (!vec_reserve(v, v->len + 1))
        return 0;
    v->data[v->len++] = value;
    return 1;
}

static void vec_free(IntVec *v)
{
    free(v->data);                      /* free(NULL) 合法，初值就是 NULL */
    v->data = NULL;
    v->len  = 0;
    v->cap  = 0;
}

/* ====================== 实战组件二：二维数组（指针数组版） ====================== */
/*
 * 行指针数组 + 每行单独 malloc：行可以不连续，访问是两次间接。
 * 适合"每行列数不同"的锯齿数组。
 */
static int **matrix_new(size_t rows, size_t cols)
{
    int **m = malloc(rows * sizeof *m);

    if (m == NULL)
        return NULL;

    for (size_t i = 0; i < rows; i++) {
        m[i] = malloc(cols * sizeof **m);
        if (m[i] == NULL) {                     /* ★ 失败要回滚，否则前面几行泄漏 */
            while (i > 0)
                free(m[--i]);
            free(m);
            return NULL;
        }
    }
    return m;
}

static void matrix_free(int **m, size_t rows)
{
    if (m == NULL)
        return;
    for (size_t i = 0; i < rows; i++)
        free(m[i]);
    free(m);
}

/* 复制字符串：自己实现 strdup（POSIX 才提供该函数，标准 C 没有） */
static char *dup_string(const char *s)
{
    size_t n = strlen(s) + 1;           /* ★ +1 留给结尾 '\0' */
    char  *p = malloc(n);

    if (p != NULL)
        memcpy(p, s, n);
    return p;
}

enum { COLS = 4 };                      /* 连续二维数组的列数，编译期常量 */

int main(void)
{
    /* ---------- 1. 标准五步流程 ---------- */
    printf("1. malloc 的标准流程：申请 → 判空 → 使用 → 释放 → 置空\n");
    {
        size_t n = 5;
        int   *p = malloc(n * sizeof *p);      /* ① 申请，sizeof *p 自动跟随类型 */

        if (p == NULL) {                        /* ② 判空，不能省 */
            printf("   分配失败，退出\n");
            return 1;
        }

        for (size_t i = 0; i < n; i++)          /* ③ 使用 */
            p[i] = (int)(i * i);
        printf("   平方表：");
        for (size_t i = 0; i < n; i++)
            printf("%d ", p[i]);
        printf("\n");

        free(p);                                /* ④ 释放，恰好一次 */
        p = NULL;                               /* ⑤ 置空，防野指针 */
        printf("   free 后已置 NULL，再次 free(NULL) 也安全\n");
        free(p);                                /* 合法：free(NULL) 什么都不做 */
    }

    /* ---------- 2. calloc 清零 ---------- */
    printf("2. calloc 保证清零，malloc 不清零\n");
    {
        size_t n = 6;
        int   *z = calloc(n, sizeof *z);        /* 自动清零 + 检查乘法溢出 */

        if (z == NULL) {
            printf("   calloc 失败\n");
            return 1;
        }

        int all_zero = 1;
        for (size_t i = 0; i < n; i++)
            if (z[i] != 0)
                all_zero = 0;

        printf("   calloc 出来的 %zu 个元素全是 0？%s\n", n, all_zero ? "是" : "否");
        printf("   换成 malloc 就**不能**这样假设：里面是上一个用户留下的脏数据\n");
        free(z);
        z = NULL;
    }

    /* ---------- 3. realloc 扩容 ---------- */
    printf("3. realloc 扩缩容：内容保留，可能搬家\n");
    {
        size_t cap = 2;
        int   *p = malloc(cap * sizeof *p);

        if (p == NULL) {
            printf("   分配失败\n");
            return 1;
        }
        for (size_t i = 0; i < cap; i++)
            p[i] = (int)(100 + i);
        printf("   初始 2 个：%d %d（地址 %p）\n", p[0], p[1], (void *)p);

        size_t new_cap = 8;
        int   *tmp = realloc(p, new_cap * sizeof *tmp);   /* ★ 临时指针接住 */

        if (tmp == NULL) {
            printf("   扩容失败：p 仍然有效，仍可继续用或释放\n");
            free(p);
            return 1;
        }
        p = tmp;                                          /* 只有成功才覆盖 */

        for (size_t i = cap; i < new_cap; i++)             /* 新区域要自己填 */
            p[i] = (int)(100 + i);

        printf("   扩到 8 个：");
        for (size_t i = 0; i < new_cap; i++)
            printf("%d ", p[i]);
        printf("\n   新地址 %p：老内容原样保留，地址可能变（搬家）也可能不变\n",
               (void *)p);

        free(p);
        p = NULL;
    }

    /* ---------- 4. sizeof 陷阱 ---------- */
    printf("4. sizeof p 与 sizeof *p 差一个星号，差一场越界\n");
    {
        int *p = NULL;

        printf("   sizeof(p)  = %zu  ← 指针本身\n", sizeof p);
        printf("   sizeof(*p) = %zu  ← 指向的对象\n", sizeof *p);

        /* 真正会出事的场景：结构体（指针 8 字节，结构体远大于 8 字节） */
        struct Big { char data[64]; int tag; };
        struct Big *b = NULL;

        /* 用 sizeof(void *) 代表"sizeof(b) 的值"，避免静态检查器误报这行是笔误 */
        printf("   struct Big 需要 %zu 字节，可 sizeof(b) 只有 %zu 字节\n",
               sizeof *b, sizeof(void *));
        printf("   malloc(sizeof(b)) 只申请 %zu 字节却要装 %zu 字节\n",
               sizeof(void *), sizeof *b);
        printf("   → 越界写，踩坏堆元数据，崩溃点常常在很远的地方\n");
        printf("   正确写法：malloc(sizeof *b)\n");
    }

    /* ---------- 5. 字符串与 +1 ---------- */
    printf("5. 复制字符串：别忘了结尾的 '\\0'\n");
    {
        const char *src = "dynamic memory";
        char       *copy = dup_string(src);

        if (copy == NULL) {
            printf("   复制失败\n");
            return 1;
        }
        printf("   源：%s\n", src);
        printf("   副本：%s（申请了 strlen + 1 = %zu 字节）\n",
               copy, strlen(src) + 1);

        copy[0] = 'D';                       /* 堆上的副本可以随便改 */
        printf("   修改副本首字母后：%s，源仍是 %s\n", copy, src);

        free(copy);
        copy = NULL;
    }

    /* ---------- 6. 实战：动态数组 ---------- */
    printf("6. 实战：倍增扩容的动态数组\n");
    {
        IntVec v = { NULL, 0, 0 };           /* 惰性：还没分配 */

        printf("   初始：len = %zu, cap = %zu, data = %s\n",
               v.len, v.cap, v.data == NULL ? "NULL" : "已分配");

        for (int i = 1; i <= 10; i++) {
            if (!vec_push(&v, i * 10)) {
                printf("   第 %d 次 push 失败\n", i);
                break;
            }
            printf("   push %3d → len = %2zu, cap = %2zu\n",
                   i * 10, v.len, v.cap);
        }

        printf("   最终内容：");
        for (size_t i = 0; i < v.len; i++)
            printf("%d ", v.data[i]);
        printf("\n   cap 是 2 的幂次增长（4 → 8 → 16），摊还每次 push 是 O(1)\n");

        vec_free(&v);
        printf("   vec_free 之后：len = %zu, cap = %zu, data = NULL\n", v.len, v.cap);
    }

    /* ---------- 7. 实战：二维数组两种分配 ---------- */
    printf("7. 实战：二维数组的两种堆分配方式\n");
    {
        size_t rows = 3, cols = 4;

        /* 方式一：指针数组，每行单独分配（行可以不连续） */
        int **m = matrix_new(rows, cols);

        if (m != NULL) {
            for (size_t i = 0; i < rows; i++)
                for (size_t j = 0; j < cols; j++)
                    m[i][j] = (int)(i * cols + j);      /* 0..11 */

            printf("   方式一（int **，%zu 次 malloc）：\n", rows + 1);
            for (size_t i = 0; i < rows; i++) {
                printf("     ");
                for (size_t j = 0; j < cols; j++)
                    printf("%3d", m[i][j]);
                printf("   ← 行首地址 %p\n", (void *)m[i]);
            }
            matrix_free(m, rows);                        /* free 时也要逆序逐行 */
            m = NULL;
            printf("   释放：先 free 每一行，再 free 行指针数组\n");
        }

        /* 方式二：一块连续内存，用"指向数组的指针"访问 */
        int (*g)[COLS] = malloc(rows * sizeof *g);       /* rows × COLS 个 int */

        if (g == NULL) {
            printf("   分配失败\n");
            return 1;
        }
        for (size_t i = 0; i < rows; i++)
            for (int j = 0; j < COLS; j++)
                g[i][j] = (int)(i * COLS + j);

        printf("   方式二（int (*)[%d]，1 次 malloc，连续内存）：\n", (int)COLS);
        for (size_t i = 0; i < rows; i++) {
            printf("     ");
            for (int j = 0; j < COLS; j++)
                printf("%3d", g[i][j]);
            printf("\n");
        }
        printf("   一块连续内存，缓存友好、只需一次 free，但列数在编译期固定（%d）\n",
               (int)COLS);

        free(g);
        g = NULL;
    }

    /* ---------- 8. 小结 ---------- */
    printf("8. 小结：\n");
    printf("     malloc 不清零、calloc 清零、realloc 可能搬家、free 只 free 一次\n");
    printf("     每个 malloc 都要判 NULL，每个成功申请恰好配一次 free\n");
    printf("     一律用 sizeof *p 算大小，别用 sizeof p\n");
    printf("     realloc 用临时指针接住，失败时原块仍有效\n");
    printf("     free 之后立刻置 NULL，让野指针变成明确的空指针\n");
    printf("     动态数组倍增扩容，摊还 O(1)，避免每次 push 都 realloc\n");
    printf("     收尾跑一遍 -fsanitize=address，越界/泄漏/重复释放一次现形\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 地址随平台和每次运行变化，其余输出固定。
 *
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 02_动态内存分配.c -o bin/02_动态内存分配
 *     ./bin/02_动态内存分配
 *
 * 输出示意：
 *
 * 1. malloc 的标准流程：申请 → 判空 → 使用 → 释放 → 置空
 *    平方表：0 1 4 9 16
 *    free 后已置 NULL，再次 free(NULL) 也安全
 * 2. calloc 保证清零，malloc 不清零
 *    calloc 出来的 6 个元素全是 0？是
 *    换成 malloc 就**不能**这样假设：里面是上一个用户留下的脏数据
 * 3. realloc 扩缩容：内容保留，可能搬家
 *    初始 2 个：100 101（地址 0x1012fddd0）
 *    扩到 8 个：100 101 102 103 104 105 106 107
 *    新地址 0x1012fdde0：老内容原样保留，地址可能变（搬家）也可能不变
 * 4. sizeof p 与 sizeof *p 差一个星号，差一场越界
 *    sizeof(p)  = 8  ← 指针本身
 *    sizeof(*p) = 4  ← 指向的对象
 *    struct Big 需要 68 字节，可 sizeof(b) 只有 8 字节
 *    malloc(sizeof(b)) 只申请 8 字节却要装 68 字节
 *    → 越界写，踩坏堆元数据，崩溃点常常在很远的地方
 *    正确写法：malloc(sizeof *b)
 * 5. 复制字符串：别忘了结尾的 '\0'
 *    源：dynamic memory
 *    副本：dynamic memory（申请了 strlen + 1 = 15 字节）
 *    修改副本首字母后：Dynamic memory，源仍是 dynamic memory
 * 6. 实战：倍增扩容的动态数组
 *    初始：len = 0, cap = 0, data = NULL
 *    push  10 → len =  1, cap =  4
 *    push  20 → len =  2, cap =  4
 *    push  30 → len =  3, cap =  4
 *    push  40 → len =  4, cap =  4
 *    push  50 → len =  5, cap =  8
 *    push  60 → len =  6, cap =  8
 *    push  70 → len =  7, cap =  8
 *    push  80 → len =  8, cap =  8
 *    push  90 → len =  9, cap = 16
 *    push 100 → len = 10, cap = 16
 *    最终内容：10 20 30 40 50 60 70 80 90 100
 *    cap 是 2 的幂次增长（4 → 8 → 16），摊还每次 push 是 O(1)
 *    vec_free 之后：len = 0, cap = 0, data = NULL
 * 7. 实战：二维数组的两种堆分配方式
 *    方式一（int **，4 次 malloc）：
 *        0  1  2  3   ← 行首地址 0x1007090a0
 *        4  5  6  7   ← 行首地址 0x1007090c0
 *        8  9 10 11   ← 行首地址 0x1007090e0
 *    释放：先 free 每一行，再 free 行指针数组
 *    方式二（int (*)[4]，1 次 malloc，连续内存）：
 *        0  1  2  3
 *        4  5  6  7
 *        8  9 10 11
 *    一块连续内存，缓存友好、只需一次 free，但列数在编译期固定（4）
 * 8. 小结：
 *     malloc 不清零、calloc 清零、realloc 可能搬家、free 只 free 一次
 *     每个 malloc 都要判 NULL，每个成功申请恰好配一次 free
 *     一律用 sizeof *p 算大小，别用 sizeof p
 *     realloc 用临时指针接住，失败时原块仍有效
 *     free 之后立刻置 NULL，让野指针变成明确的空指针
 *     动态数组倍增扩容，摊还 O(1)，避免每次 push 都 realloc
 *     收尾跑一遍 -fsanitize=address，越界/泄漏/重复释放一次现形
 * ==========================================================================
 */
