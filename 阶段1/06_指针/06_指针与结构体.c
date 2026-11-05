/*
 * 06_指针与结构体.c —— 阶段1 / 第 06 课 / 第 06 个文件：指针与结构体
 *
 * 编译：gcc -std=c99 -Wall -Wextra 06_指针与结构体.c -o bin/06_指针与结构体
 * 运行：./bin/06_指针与结构体
 *
 * 本篇的链表**不用 malloc**，而是用一块静态数组当"节点池"，
 * 这样本节只讲结构体指针本身，不掺动态内存（留给后面的内存管理章节）。
 *
 * ====================== 一、结构体指针与 -> ======================
 *      struct Point { int x, y; } p = {3, 4};
 *      struct Point *pp = &p;
 *
 *      (*pp).x     先解引用拿到结构体，再取成员
 *      pp->x       同上，语法糖
 *
 * 为什么需要 ->？因为 `.` 的优先级高于 `*`：
 *
 *      *pp.x   等于 *(pp.x)   ✗ pp 是指针，没有 .x 这个成员，编译错误
 *
 * 所以必须写成 (*pp).x 或者 pp->x。C 的答案就是给你一个 ->。
 *
 *      pp->x  ≡  (*pp).x
 *      &pp->x ≡  &((*pp).x)       注意 -> 和 . 一样，优先级高于 &
 *
 * 记忆：**指针用 ->，对象用 . **。看到 -> 左边一定是个指针。
 *
 * ====================== 二、结构体传参：值 vs 指针 ======================
 *      void by_value(struct Point p);          // 整个结构体被复制一份
 *      void by_pointer(struct Point *p);       // 只传 8 字节地址
 *
 * 三个理由选指针：
 *
 *  (1) 要修改调用方的数据 —— 值传递改的是副本（和 01 篇的 swap 一个道理）
 *  (2) 结构体很大 —— 复制成本随成员数量增长，指针恒定 8 字节
 *  (3) 要保持身份 —— 复制出来的副本和原对象是两个东西
 *
 * 只读参数一定要加 const：
 *
 *      void print_point(const struct Point *p);
 *
 * 这既表达了意图，又能接受 `const` 对象作实参。
 *
 * ====================== 三、结构体数组与指针遍历 ======================
 *      struct Student cls[3];
 *      struct Student *p = cls;
 *
 *      cls[i]     第 i 个元素（结构体本身）
 *      p + 1      往后一个结构体（步长 = sizeof(struct Student)）
 *      p->score   (*p).score
 *
 * 指针遍历的写法和数组完全一致，因为结构体数组也是连续内存：
 *
 *      for (const struct Student *p = cls; p < cls + n; p++)
 *          printf("%s %d\n", p->name, p->score);
 *
 * 注意：这里 p + 1 走的是 sizeof(结构体)，可能因为**内存对齐**比成员总和更大。
 *      struct { char c; int i; } 一般占 8 字节而不是 5 字节，多出的 3 字节是填充。
 *      所以算元素个数永远用 sizeof(cls) / sizeof(cls[0])，不要手算成员大小之和。
 *
 * ====================== 四、自引用结构体：链表的全部秘密 ======================
 *      typedef struct Node {
 *          int val;
 *          struct Node *next;      // ★ 指向"同一种结构体"
 *      } Node;
 *
 * 自引用的三个要点：
 *
 *  (1) 成员必须是指针，不能是结构体自己
 *          struct Node { struct Node next; };   ✗ 大小算不出来，无限递归
 *          struct Node { struct Node *next; };  ✓ 指针大小固定 8 字节
 *
 *  (2) 结构体内部还不能用 typedef 的新名字，要写 `struct Node *`
 *          （因为 typedef 要等整个声明结束才生效）
 *
 *  (3) 链表 = 一串这样连起来的节点，最后一个是 NULL
 *
 *          head ──▶ [1|●] ──▶ [2|●] ──▶ [3|NULL]
 *
 *  (4) 链式 -> 必须每步判空
 *          head->next->val      head 或 head->next 为空时，直接崩
 *          if (head != NULL && head->next != NULL) return head->next->val;
 *      注意 && 和 || 是**短路求值**：前一半为假，后一半根本不执行，
 *      所以这种「先判外层、再判内层」的写法本身就是安全的。
 *
 * 头插法（O(1)，最常用）：
 *
 *      Node *push_front(Node *head, int v)
 *      {
 *          Node *n = new_node(v);
 *          n->next = head;         // 新节点指向原来的头
 *          return n;               // 新节点成为新头
 *      }
 *
 * 反转链表（面试常客，只改指针不搬数据）：
 *
 *      while (head != NULL) {
 *          Node *next = head->next;   // 先记住下一个，否则断了就找不到
 *          head->next = prev;         // 掉头
 *          prev = head;               // 往前走
 *          head = next;
 *      }
 *      return prev;                   // 新的头
 *
 * ====================== 五、结构体里的指针成员：浅拷贝的坑 ======================
 * 结构体赋值（=）和传值都是**逐字节复制**，叫浅拷贝：
 *
 *      typedef struct { char *label; int *data; } Box;
 *
 *      Box b1 = {"box-1", payload};
 *      Box b2 = b1;               // 复制的是指针的**值**（地址），不是它指向的内容
 *
 * 结果：b1.data 和 b2.data 指向同一块内存，改一个另一个也变。
 * 如果这块内存是 malloc 来的，还会导致**重复 free**。
 *
 * 规则：
 *  - 结构体里有指针成员 → 拷贝要自己写深拷贝函数
 *  - 谁 malloc 谁 free，并且只 free 一次
 *  - 只读的字符串用 `const char *` 指过去没问题（本来就不该改）
 *
 * ====================== 六、常见坑 ======================
 *  1. 对指针用 . 而不是 ->          pp.x ✗，要 pp->x
 *  2. 对结构体用 ->                 p->x ✗（p 不是指针），要 p.x
 *  3. *pp.x 写成想当然              实际是 *(pp.x)，必须写 (*pp).x 或 pp->x
 *  4. 传值却想改外面                改的是副本
 *  5. 结构体数组用 sizeof 手算大小  有对齐填充，必须 sizeof(a)/sizeof(a[0])
 *  6. 自引用写成非指针成员          大小无法确定，编译不过
 *  7. 浅拷贝后 double free          两个结构体指向同一块堆内存
 *  8. 链表遍历时把 next 提前丢了     反转/删除节点前必须先存 next
 *  9. 忘记给 next 赋 NULL           链表尾部变成野指针，遍历不停止
 * 10. 结构体指针没初始化就 ->        野指针，直接崩
 *
 * ====================== 七、实践建议 ======================
 *  - 结构体参数一律 `const T *`（只读）或 `T *`（要改），尽量别传值
 *  - 返回结构体本身没问题（小结构体），大结构体让调用方传指针
 *  - 链表操作前先画三格图：prev / cur / next，代码跟着图写
 *  - 结构体里放指针成员时，注释里写清"谁负责 free"
 *  - 遍历链表用 `for (const Node *p = head; p; p = p->next)`，条件直接写 p
 *  - 大小一律 sizeof 推导，别手算（对齐可能让结果和你算的不一样）
 */

#include <stdio.h>

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    char name[16];
    int score;
} Student;

/* 自引用结构体：链表节点。成员是指针，指回同一种类型 */
typedef struct Node {
    int val;
    struct Node *next;
} Node;

/* 带指针成员的结构体，用来演示浅拷贝 */
typedef struct {
    const char *label;
    int *data;
} Box;

/* ---- 静态节点池：代替 malloc，本篇只关心指针，不关心堆 ---- */
#define POOL_SIZE 8
static Node pool[POOL_SIZE];
static int pool_used;

static Node *node_new(int v)
{
    if (pool_used >= POOL_SIZE)           /* 池子满了就返回 NULL */
        return NULL;
    Node *n = &pool[pool_used++];
    n->val = v;
    n->next = NULL;
    return n;
}

static Node *push_front(Node *head, int v)
{
    Node *n = node_new(v);
    if (n == NULL)
        return head;
    n->next = head;                       /* 新节点指向原来的头 */
    return n;                             /* 新节点成为新头 */
}

static void print_list(const char *tag, const Node *head)
{
    printf("   %s：", tag);
    for (const Node *p = head; p != NULL; p = p->next)
        printf("%d -> ", p->val);
    printf("NULL\n");
}

static Node *reverse_list(Node *head)
{
    Node *prev = NULL;
    while (head != NULL) {
        Node *next = head->next;          /* ★ 先存住下一个，否则断链就找不回 */
        head->next = prev;                /* 掉头 */
        prev = head;                      /* prev 前移 */
        head = next;                      /* head 前移 */
    }
    return prev;                          /* 原来的尾巴成了新头 */
}

/* 取第 2 个节点的值：链式 -> 每走一步都要先判空 */
static int second_or_zero(const Node *head)
{
    if (head == NULL || head->next == NULL)  /* ★ 短路：前一半为真，后一半不执行 */
        return 0;
    return head->next->val;
}

/* 值传递：改的是副本 */
static void move_by_value(Point p, int dx, int dy)
{
    p.x += dx;
    p.y += dy;
}

/* 指针传递：改的是调用方的结构体 */
static void move(Point *p, int dx, int dy)
{
    p->x += dx;
    p->y += dy;
}

/* 只读访问：加 const 表明不改，也才能接 const 对象 */
static void print_point(const Point *p)
{
    printf("(%d, %d)\n", p->x, p->y);
}

int main(void)
{
    /* ---------- 1. 结构体指针与 -> ---------- */
    printf("1. 结构体指针：-> 就是 (*p). 的语法糖\n");
    Point p1 = {3, 4};
    Point *pp = &p1;
    printf("   (*pp).x = %d，pp->x = %d —— 两种写法完全等价\n", (*pp).x, pp->x);
    printf("   为什么不能写 *pp.x：. 的优先级高于 *，会被读成 *(pp.x)\n");
    pp->x = 30;                           /* 通过指针改原结构体 */
    printf("   pp->x = 30 之后，p1.x = %d（改的是原对象）\n", p1.x);
    printf("   sizeof(Point) = %zu 字节（两个 int）\n", sizeof(Point));

    /* ---------- 2. 传值 vs 传指针 ---------- */
    printf("2. 结构体传参：值拷贝 vs 传地址\n");
    Point a = {1, 1}, b = {1, 1};
    move_by_value(a, 10, 10);
    printf("   move_by_value(a, 10, 10) → a = ");
    print_point(&a);
    printf("      （没变，函数改的是副本）\n");
    move(&b, 10, 10);
    printf("   move(&b, 10, 10)         → b = ");
    print_point(&b);
    printf("      （变了，函数改的是原对象）\n");
    printf("   大结构体更要传指针：复制成本随成员增长，指针恒为 %zu 字节\n", sizeof(Point *));

    /* ---------- 3. 结构体数组与指针遍历 ---------- */
    printf("3. 结构体数组：下标与指针两种遍历\n");
    Student cls[3] = {{"Tom", 90}, {"Jerry", 85}, {"Spike", 77}};
    int n = (int)(sizeof(cls) / sizeof(cls[0]));
    for (int i = 0; i < n; i++)
        printf("   [下标] %-6s %d\n", cls[i].name, cls[i].score);
    for (const Student *p = cls; p < cls + n; p++)
        printf("   [指针] %-6s %d   步长 %ld 字节\n", p->name, p->score,
               (long)((char *)(p + 1) - (char *)p));
    printf("   sizeof(cls) = %zu，sizeof(cls[0]) = %zu，元素个数 = %d\n",
           sizeof(cls), sizeof(cls[0]), n);
    printf("   步长是 sizeof(Student)，含对齐填充，所以别手算成员大小之和\n");

    /* ---------- 4. 自引用结构体：单链表 ---------- */
    printf("4. 自引用结构体 = 链表（这里用静态池代替 malloc）\n");
    Node *head = NULL;
    for (int i = 1; i <= 5; i++)
        head = push_front(head, i);
    print_list("头插 1..5", head);
    printf("   每个节点长这样：[val | next]，最后一个 next 是 NULL\n");
    head = reverse_list(head);
    print_list("反转后   ", head);
    printf("   反转只改 next 指针，一个字节的数据都没搬\n");
    printf("   真实工程里节点用 malloc 逐个申请，释放时也要逐个 free\n");

    /* 链式 -> 的判空：head->next->val 在 head 为空时直接崩 */
    Node *empty = NULL;
    Node *single = push_front(NULL, 9);   /* 只有一个节点，next 是 NULL */
    printf("   链式 -> 每步都要判空：空表=%d，单节点=%d，五节点=%d\n",
           second_or_zero(empty), second_or_zero(single), second_or_zero(head));
    printf("   直接写 head->next->val 不判空，head 或 head->next 为空时就崩\n");

    /* ---------- 5. 结构体里的指针成员：浅拷贝 ---------- */
    printf("5. 结构体赋值是浅拷贝：指针成员会被共享\n");
    int payload[2] = {1, 2};
    Box b1 = {"box-1", payload};
    Box b2 = b1;                          /* 逐字节复制，只复制了指针的值 */
    printf("   b1.data 与 b2.data 相等吗？%s\n",
           (b1.data == b2.data) ? "相等，指向同一块内存" : "不等");
    b2.data[0] = 99;
    printf("   改 b2.data[0] = 99 之后，b1.data[0] = %d\n", b1.data[0]);
    printf("   想真正独立，就得自己写深拷贝（再申请一块内存，把内容复制过去）\n");
    printf("   如果 data 是 malloc 来的，浅拷贝后两边都 free 就是 double free\n");

    /* ---------- 6. 小结 ---------- */
    printf("6. 小结：\n");
    printf("    指针用 ->，对象用 .；pp->x 等价 (*pp).x\n");
    printf("    结构体参数优先传指针，只读就加 const\n");
    printf("    结构体数组连续存放，指针遍历的步长是 sizeof(结构体)\n");
    printf("    自引用必须是指针成员，这就是链表/树的基础\n");
    printf("    结构体赋值是浅拷贝，指针成员共享，必要时写深拷贝\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 06_指针与结构体.c -o bin/06_指针与结构体
 *     ./bin/06_指针与结构体
 *
 * 输出如下：
 *
 * 1. 结构体指针：-> 就是 (*p). 的语法糖
 *    (*pp).x = 3，pp->x = 3 —— 两种写法完全等价
 *    为什么不能写 *pp.x：. 的优先级高于 *，会被读成 *(pp.x)
 *    pp->x = 30 之后，p1.x = 30（改的是原对象）
 *    sizeof(Point) = 8 字节（两个 int）
 * 2. 结构体传参：值拷贝 vs 传地址
 *    move_by_value(a, 10, 10) → a = (1, 1)
 *       （没变，函数改的是副本）
 *    move(&b, 10, 10)         → b = (11, 11)
 *       （变了，函数改的是原对象）
 *    大结构体更要传指针：复制成本随成员增长，指针恒为 8 字节
 * 3. 结构体数组：下标与指针两种遍历
 *    [下标] Tom    90
 *    [下标] Jerry  85
 *    [下标] Spike  77
 *    [指针] Tom    90   步长 20 字节
 *    [指针] Jerry  85   步长 20 字节
 *    [指针] Spike  77   步长 20 字节
 *    sizeof(cls) = 60，sizeof(cls[0]) = 20，元素个数 = 3
 *    步长是 sizeof(Student)，含对齐填充，所以别手算成员大小之和
 * 4. 自引用结构体 = 链表（这里用静态池代替 malloc）
 *    头插 1..5：5 -> 4 -> 3 -> 2 -> 1 -> NULL
 *    每个节点长这样：[val | next]，最后一个 next 是 NULL
 *    反转后   ：1 -> 2 -> 3 -> 4 -> 5 -> NULL
 *    反转只改 next 指针，一个字节的数据都没搬
 *    真实工程里节点用 malloc 逐个申请，释放时也要逐个 free
 *    链式 -> 每步都要判空：空表=0，单节点=0，五节点=2
 *    直接写 head->next->val 不判空，head 或 head->next 为空时就崩
 * 5. 结构体赋值是浅拷贝：指针成员会被共享
 *    b1.data 与 b2.data 相等吗？相等，指向同一块内存
 *    改 b2.data[0] = 99 之后，b1.data[0] = 99
 *    想真正独立，就得自己写深拷贝（再申请一块内存，把内容复制过去）
 *    如果 data 是 malloc 来的，浅拷贝后两边都 free 就是 double free
 * 6. 小结：
 *     指针用 ->，对象用 .；pp->x 等价 (*pp).x
 *     结构体参数优先传指针，只读就加 const
 *     结构体数组连续存放，指针遍历的步长是 sizeof(结构体)
 *     自引用必须是指针成员，这就是链表/树的基础
 *     结构体赋值是浅拷贝，指针成员共享，必要时写深拷贝
 *
 * 几个容易忽略的对照点：
 *   第 1 条：(*pp).x 必须加括号，因为 . 的优先级高于 *；
 *           -> 就是专门为"指针取成员"造的语法糖，本质完全一样。
 *   第 2 条：同一个 move 逻辑，值传递版本对 a 毫无影响，
 *           指针版本把 b 改成了 (11, 11) —— 和 01 篇的 swap 是同一个道理。
 *   第 3 条：sizeof(Student) = 20（16 字节 name + 4 字节 score），
 *           所以指针 p + 1 走 20 字节。如果结构体成员是 char + int，
 *           会因为对齐变成 8 而不是 5，所以元素个数只能靠 sizeof 相除。
 *   第 4 条：头插 1..5 得到 5 4 3 2 1（每次插到最前面），
 *           反转后回到 1 2 3 4 5。全程只有 next 指针在变，
 *           节点的 val 和内存位置一个字都没动。
 *   第 5 条：b2 = b1 复制的是"指针的值"，于是两者指向同一块 payload；
 *           改一处两处都变。这就是浅拷贝，堆内存场景下还会导致 double free。
 * ==========================================================================
 */
