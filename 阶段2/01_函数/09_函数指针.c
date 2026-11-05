/*
 * 09_函数指针.c —— 阶段2 / 第 01 课 / 第 09 个文件：函数指针
 *
 * 编译：gcc -std=c99 -Wall -Wextra 09_函数指针.c -o bin/09_函数指针
 * 运行：./bin/09_函数指针
 *
 * 上一篇讲"返回指针的函数"（指针函数），这一篇讲它的镜像：
 * 把**函数本身**当成数据来存、来传、来调用。
 *
 *      指针函数  int  *f(void);      一个函数，返回 int *
 *      函数指针  int (*f)(void);     一个指针，指向"返回 int 的函数"
 *
 * 函数指针是 C 实现"多态"和"回调"的唯一手段：
 * 标准库的 qsort、bsearch、signal 的第二个参数，全是函数指针。
 *
 * 阶段1 第 06 课第 04 篇已经讲过声明读法、typedef 和 qsort 的用法；
 * 这一篇补的是工程侧：回调接口怎么设计、分发表怎么组织、函数指针能不能塞进 void *。
 *
 * ====================== 一、函数名在表达式里就是地址 ======================
 * 函数编译后是一段机器码，放在代码段里，有自己的入口地址。
 *
 *      int add(int a, int b) { return a + b; }
 *
 *      add            // 在表达式里：退化为"指向该函数的指针"
 *      &add           // 显式取地址，和 add 完全等价（函数名不是数组，& 不多余但不改变值）
 *      *add           // 也等价于 add —— 解引用再退化的结果还是它自己
 *
 * 所以下面三种初始化完全一样：
 *
 *      int (*fp)(int, int) = add;
 *      int (*fp)(int, int) = &add;
 *      int (*fp)(int, int) = *add;
 *
 * ★ 和数组的区别：数组名不是指针（是数组类型），函数名在**所有**表达式里
 *   都退化成指针（除了 sizeof 和一元 & 的操作数）—— 而 sizeof(函数) 非法。
 *
 * ====================== 二、声明怎么读 ======================
 *      int (*fp)(int, int);
 *
 * 右左法则：
 *      fp 向右 → ) 停住，掉头向左 → * 说明 fp 是**指针**
 *      (*fp) 向右 → (int, int) 说明指向的是**函数**，两个 int 参数
 *      再向左 → int，说明该函数返回 int
 *      连起来：fp 是指向"接收两个 int、返回 int 的函数"的指针
 *
 * 和它的近亲对比：
 *      int *fp(int, int);      返回 int * 的函数（指针函数，08 篇）
 *      int (*fp)(int, int);    指向函数的指针（本篇）
 *      int (*fp[3])(int, int); 数组，3 个元素，每个都是函数指针
 *      int (*(*fp)(void))(int); fp 是函数指针，指向"返回函数指针"的函数
 *
 * 最后两个已经很难读了，所以工程里**一律用 typedef 起名**：
 *
 *      typedef int (*BinOp)(int, int);      // BinOp 是一种"函数指针类型"
 *      BinOp fp = add;
 *      BinOp table[3] = { add, sub, mul };
 *      BinOp pick(char c);                  // 返回函数指针的函数，一目了然
 *
 * ★ typedef 的位置很关键：`typedef int *BinOp(int,int)` 定义的是
 *   "返回 int * 的函数类型"，不是函数指针。少写一对括号，意思就反了。
 *
 * ====================== 三、怎么调用：三种写法等价 ======================
 *      fp(a, b);        // 现代写法，最常用
 *      (*fp)(a, b);     // 老代码风格，强调"先解引用再调用"
 *      (**fp)(a, b);    // 也合法：函数指针解引用多少次都还是它自己
 *
 * 三者生成的代码一样。推荐第一种，但读到 `(*fp)()` 要知道它是同一个意思。
 *
 * ★ 调用前必须判空：函数指针为 NULL 时调用 = 跳到地址 0，直接崩溃。
 *   和普通指针一样，`if (fp != NULL)` 是最便宜的保护。
 *
 * ====================== 四、作参数 = 回调（callback） ======================
 * 这是函数指针最重要的用途：**把"要做什么"作为参数传进去**。
 *
 *      int apply(int a, int b, BinOp op)      // op 是"待会儿要调的函数"
 *      {
 *          return op(a, b);
 *      }
 *
 *      apply(3, 4, add);      // 8 行代码，能算加、减、乘、最大值……
 *
 * 为什么有用：调用方决定行为，被调用方只负责流程。
 * 排序是最典型的例子 —— 排序算法只关心"谁在前"，不关心"按什么比"：
 *
 *      void mysort(int *a, int n, int (*cmp)(int, int));
 *      mysort(a, n, cmp_asc);     // 升序
 *      mysort(a, n, cmp_desc);    // 降序：一行代码换一种行为
 *
 * ★ 签名必须**完全一致**，包括返回类型和每个参数类型：
 *      比较器要求 int (*)(int, int)，你传 int (*)(const int *, const int *)
 *      编译器会报 "incompatible pointer type" 或 "expected ... but argument is of type ..."。
 *   强转能骗过编译器，但调用约定一旦不一致就是 UB。
 *
 * ★ 比较器的返回值约定（qsort 也是这样）：
 *      < 0   第一个参数排在前面
 *      = 0   两者相等（顺序无所谓）
 *      > 0   第一个参数排在后面
 *   注意：不要用 `a - b` 写比较器 —— int 相减会溢出。
 *   教学示例里为了简短常用，工程里应写 `(a > b) - (a < b)`。
 *
 * ====================== 五、函数指针数组 = 分发表（dispatch table） ======================
 * 一堆同类操作配一个字符串名字，最容易写成又长又深的 switch：
 *
 *      switch (code) { case 1: ... case 2: ... case 99: ... }   // 几十个分支
 *
 * 换成"名字 → 函数"的表，查找和扩展都简单：
 *
 *      struct OpEntry { const char *name; BinOp fn; };
 *      static const struct OpEntry ops[] = {
 *          { "add", add }, { "sub", sub }, { "mul", mul },
 *      };
 *
 * 好处：
 *   - 加一个操作 = 加一行数据，不用改控制流
 *   - 表和实现放在一起，一眼看全
 *   - 可以运行时查表（配置驱动、命令行子命令、状态机）
 *
 * 代价：
 *   - 查表是线性的（表大就用排序 + 二分，或哈希）
 *   - 表里的函数签名必须一致，参数个数不同的操作塞不进同一张表
 *     （这类场景要用"统一参数包"：`int (*fn)(const char **argv, int argc)`）
 *   - 查不到时要明确处理：返回错误码，别静默调用 NULL
 *
 * ====================== 六、返回函数指针 ======================
 * 工厂模式的最小形态：根据条件挑一个函数交出去。
 *
 * 不用 typedef（难读，但要知道怎么读）：
 *      int (*pick(char c))(int, int);
 *          ↑ 从 pick 开始：pick 是函数，参数 char
 *          ↑ 向左 * → 返回指针；这个指针指向"接收 (int,int)、返回 int"的函数
 *
 * 用 typedef（推荐）：
 *      typedef int (*BinOp)(int, int);
 *      BinOp pick(char c);
 *
 * 用途：插件注册、状态机的下一状态、把"选哪个算法"推迟到运行时。
 *
 * ★ 返回的函数指针可能为 NULL（没有匹配项），调用方必须判空 —— 和 08 篇
 *   返回 malloc 的指针是同一类"返回值可能是空"的问题。
 *
 * ====================== 七、函数指针和 void * 不是一回事 ======================
 *
 *      void *p = (void *)add;        // ISO C 不允许（gcc 不报错，-Wpedantic 才提示）
 *
 * 原因：C 标准只保证"对象指针"和 void * 之间可以互转；**函数指针**属于
 * 另一类，标准没有保证能从 void * 转回来。在"代码段和数据段分开"的
 * 架构上（哈佛架构、部分嵌入式）两者甚至宽度都不同。
 *
 * 工程规则：
 *   - 存函数指针，就用具体的函数指针类型（或 typedef）
 *   - 通用容器里要放函数指针，包一层结构体，别转 void *
 *   - POSIX 的 dlsym 返回 void *，转成函数指针是 POSIX 的扩展保证，不是 C 的
 *
 * ====================== 八、常见坑 ======================
 *  1. 忘记判空就调用                 NULL 调用直接崩溃
 *  2. 签名不一致，强转后调用         调用约定不同 → UB，且编译器不再保护你
 *  3. 比较器用 `a - b`                相减溢出，排序结果错乱
 *  4. typedef 少写括号                `typedef int *F(int,int)` 是"返回 int* 的函数"
 *  5. 把函数指针当数组做算术          `fp + 1` 没有意义（标准不允许函数指针算术）
 *  6. 用 == 比较两个函数指针          只有指向同一个函数才为真；想比较"是不是同一份实现"要小心多份副本（static 函数）
 *  7. 表里留空槽却不检查             调用 NULL 项 = 崩溃
 *  8. 以为 `(*fp)()` 和 `fp()` 有性能差别   两者生成同样的调用指令
 *
 * ====================== 九、实践建议 ======================
 *  - 一律 typedef：`typedef int (*BinOp)(int, int);` 之后代码立刻可读
 *  - 回调函数的参数只读就加 const，别让回调偷偷改调用方的数据
 *  - 所有"接受函数指针"的接口，在文档里写清：何时调用、能调几次、能否为 NULL
 *  - 表驱动优先于长 switch；表的最后加一条"未命中"处理
 *  - 比较器遵守 qsort 的返回值约定，别返回 bool
 *  - 需要"带上下文"的回调（C 没有闭包），用 qsort_r 风格：多传一个 void *ctx
 */

#include <stdio.h>
#include <string.h>

/* ---- 三个同签名的操作：能被同一种函数指针指向 ---- */
static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }

/* ---- typedef：给"函数指针类型"起名，后面的代码都能少写括号 ---- */
typedef int (*BinOp)(int, int);

/* ---- 作参数：apply 只负责"调用"，不关心 op 具体算什么 ---- */
static int apply(int a, int b, BinOp op)
{
    if (op == NULL)                  /* 防御：接口允许传空指针 */
        return 0;
    return op(a, b);                 /* 等价于 (*op)(a, b) */
}

/* ---- 比较器：遵守 <0 / 0 / >0 的约定，且不做可能溢出的减法 ---- */
static int cmp_asc(int a, int b)  { return (a > b) - (a < b); }
static int cmp_desc(int a, int b) { return (a < b) - (a > b); }

/* ---- 同一份排序代码，换个比较器就是另一种顺序 ---- */
static void mysort(int *a, int n, int (*cmp)(int, int))
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (cmp(a[j], a[j + 1]) > 0) {
                int t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
        }
    }
}

/* ---- 分发表：名字 → 函数，替代长 switch ---- */
struct OpEntry {
    const char *name;
    BinOp       fn;
};

static const struct OpEntry ops[] = {
    { "add", add },
    { "sub", sub },
    { "mul", mul },
};

/* ok 用来告诉调用方"到底查到没有"，避免用魔数当错误码 */
static int dispatch(const char *name, int a, int b, int *ok)
{
    size_t n = sizeof ops / sizeof ops[0];

    for (size_t i = 0; i < n; i++) {
        if (strcmp(ops[i].name, name) == 0) {
            *ok = 1;
            return ops[i].fn(a, b);
        }
    }
    *ok = 0;
    return 0;
}

/* ---- 返回函数指针：两种等价写法，对比可读性 ---- */
static BinOp pick_op(char c)                       /* 用 typedef：一眼看懂 */
{
    switch (c) {
    case '+': return add;
    case '-': return sub;
    case '*': return mul;
    default:  return NULL;                         /* 没有匹配项 */
    }
}

static int (*pick_op_raw(char c))(int, int)        /* 不用 typedef：读起来很费劲 */
{
    return pick_op(c);                             /* 逻辑复用上面的版本 */
}

/* ---- 打印一个数组，减少 main 里的重复代码 ---- */
static void print_ints(const char *label, const int *a, int n)
{
    printf("   %s: [", label);
    for (int i = 0; i < n; i++)
        printf("%s%d", i ? ", " : "", a[i]);
    printf("]\n");
}

int main(void)
{
    /* ---------- 1. 函数名就是地址 ---------- */
    printf("1. 函数名在表达式里退化为指针\n");
    BinOp fp = add;
    printf("   fp = add 后，fp == add → %s，fp == &add → %s\n",
           fp == add ? "真" : "假", fp == &add ? "真" : "假");
    printf("   add、&add、*add 三者值相同，都是函数入口地址\n");
    printf("   sizeof(函数) 非法；sizeof(函数指针) = %zu 字节\n", sizeof(BinOp));

    /* ---------- 2. 声明怎么读 ---------- */
    printf("2. 声明：int (*fp)(int, int) 才是函数指针\n");
    printf("   int  *f(int, int);    ← 指针函数（返回 int *）\n");
    printf("   int (*f)(int, int);   ← 函数指针（本篇）\n");
    printf("   int (*f[3])(int,int); ← 3 个函数指针组成的数组\n");

    /* ---------- 3. 三种调用写法 ---------- */
    printf("3. 三种调用写法等价\n");
    printf("   fp(3, 4)        = %d\n", fp(3, 4));
    printf("   (*fp)(3, 4)     = %d\n", (*fp)(3, 4));
    printf("   (**fp)(3, 4)    = %d\n", (**fp)(3, 4));
    printf("   空指针调用会跳到地址 0：接口里务必先判空\n");

    /* ---------- 4. 作参数 = 回调 ---------- */
    printf("4. 回调：同一个 apply，换不同函数就是不同行为\n");
    printf("   apply(7, 5, add) = %d\n", apply(7, 5, add));
    printf("   apply(7, 5, sub) = %d\n", apply(7, 5, sub));
    printf("   apply(7, 5, mul) = %d\n", apply(7, 5, mul));
    printf("   apply(7, 5, NULL) = %d（防御性返回 0，而不是崩溃）\n",
           apply(7, 5, NULL));

    /* ---------- 5. 比较器：排序算法不关心排序规则 ---------- */
    printf("5. 回调实战：mysort 只做交换，顺序由比较器决定\n");
    int a1[] = { 5, 2, 9, 1, 7 };
    int a2[] = { 5, 2, 9, 1, 7 };
    int n = (int)(sizeof a1 / sizeof a1[0]);

    print_ints("原始", a1, n);
    mysort(a1, n, cmp_asc);
    print_ints("cmp_asc 升序", a1, n);
    mysort(a2, n, cmp_desc);
    print_ints("cmp_desc 降序", a2, n);
    printf("   比较器返回 <0 / 0 / >0，表示“前 < 后 / 相等 / 前 > 后”\n");
    printf("   别用 a - b：int 相减可能溢出，排序结果会错\n");

    /* ---------- 6. 分发表 ---------- */
    printf("6. 分发表：名字 → 函数，替代长 switch\n");
    const char *want[] = { "add", "sub", "mul", "div" };

    for (size_t i = 0; i < sizeof want / sizeof want[0]; i++) {
        int ok = 0;
        int r = dispatch(want[i], 8, 2, &ok);
        if (ok)
            printf("   dispatch(\"%s\", 8, 2) = %d\n", want[i], r);
        else
            printf("   dispatch(\"%s\", 8, 2) → 未命中，返回错误\n", want[i]);
    }
    printf("   加一个操作只加一行表项；但每项签名必须一致\n");

    /* ---------- 7. 返回函数指针 ---------- */
    printf("7. 返回函数指针：把“选哪个函数”推迟到运行时\n");
    const char signs[] = { '+', '-', '*' };

    for (size_t i = 0; i < sizeof signs; i++) {
        BinOp op = pick_op(signs[i]);
        if (op != NULL)
            printf("   pick_op('%c') → 调用结果 %d\n", signs[i], op(6, 3));
        else
            printf("   pick_op('%c') → NULL\n", signs[i]);
    }
    printf("   pick_op('%%') = %s（没匹配到，返回 NULL，调用方判空）\n",
           pick_op('%') == NULL ? "NULL" : "非空");
    printf("   不用 typedef 的原始声明：int (*pick_op_raw(char))(int, int)\n");
    printf("   pick_op_raw('+') 调用结果 = %d\n", pick_op_raw('+')(6, 3));

    /* ---------- 8. 函数指针 vs void * ---------- */
    printf("8. 函数指针不能随便转 void *\n");
    printf("   C 标准只保证对象指针与 void * 互转；函数指针是另一类\n");
    printf("   gcc 不报错，但 -Wpedantic 会提示，换平台可能就出事\n");
    printf("   要通用存函数，用具体类型或包一层结构体\n");

    /* ---------- 9. 小结 ---------- */
    printf("9. 小结：\n");
    printf("    函数名在表达式里退化为指针，add == &add == *add\n");
    printf("    int (*f)(int,int) 是函数指针；int *f(int,int) 是指针函数\n");
    printf("    一律 typedef 成 BinOp，声明立刻可读\n");
    printf("    作参数就是回调，签名必须完全一致，调用前判空\n");
    printf("    函数指针数组 = 分发表，比长 switch 好扩展\n");
    printf("    返回函数指针时，没匹配到就返回 NULL，别返回野地址\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 09_函数指针.c -o bin/09_函数指针
 *     ./bin/09_函数指针
 *
 * 输出如下：
 *
 * 1. 函数名在表达式里退化为指针
 *    fp = add 后，fp == add → 真，fp == &add → 真
 *    add、&add、*add 三者值相同，都是函数入口地址
 *    sizeof(函数) 非法；sizeof(函数指针) = 8 字节
 * 2. 声明：int (*fp)(int, int) 才是函数指针
 *    int  *f(int, int);    ← 指针函数（返回 int *）
 *    int (*f)(int, int);   ← 函数指针（本篇）
 *    int (*f[3])(int,int); ← 3 个函数指针组成的数组
 * 3. 三种调用写法等价
 *    fp(3, 4)        = 7
 *    (*fp)(3, 4)     = 7
 *    (**fp)(3, 4)    = 7
 *    空指针调用会跳到地址 0：接口里务必先判空
 * 4. 回调：同一个 apply，换不同函数就是不同行为
 *    apply(7, 5, add) = 12
 *    apply(7, 5, sub) = 2
 *    apply(7, 5, mul) = 35
 *    apply(7, 5, NULL) = 0（防御性返回 0，而不是崩溃）
 * 5. 回调实战：mysort 只做交换，顺序由比较器决定
 *    原始: [5, 2, 9, 1, 7]
 *    cmp_asc 升序: [1, 2, 5, 7, 9]
 *    cmp_desc 降序: [9, 7, 5, 2, 1]
 *    比较器返回 <0 / 0 / >0，表示“前 < 后 / 相等 / 前 > 后”
 *    别用 a - b：int 相减可能溢出，排序结果会错
 * 6. 分发表：名字 → 函数，替代长 switch
 *    dispatch("add", 8, 2) = 10
 *    dispatch("sub", 8, 2) = 6
 *    dispatch("mul", 8, 2) = 16
 *    dispatch("div", 8, 2) → 未命中，返回错误
 *    加一个操作只加一行表项；但每项签名必须一致
 * 7. 返回函数指针：把“选哪个函数”推迟到运行时
 *    pick_op('+') → 调用结果 9
 *    pick_op('-') → 调用结果 3
 *    pick_op('*') → 调用结果 18
 *    pick_op('%') = NULL（没匹配到，返回 NULL，调用方判空）
 *    不用 typedef 的原始声明：int (*pick_op_raw(char))(int, int)
 *    pick_op_raw('+') 调用结果 = 9
 * 8. 函数指针不能随便转 void *
 *    C 标准只保证对象指针与 void * 互转；函数指针是另一类
 *    gcc 不报错，但 -Wpedantic 会提示，换平台可能就出事
 *    要通用存函数，用具体类型或包一层结构体
 * 9. 小结：
 *     函数名在表达式里退化为指针，add == &add == *add
 *     int (*f)(int,int) 是函数指针；int *f(int,int) 是指针函数
 *     一律 typedef 成 BinOp，声明立刻可读
 *     作参数就是回调，签名必须完全一致，调用前判空
 *     函数指针数组 = 分发表，比长 switch 好扩展
 *     返回函数指针时，没匹配到就返回 NULL，别返回野地址
 * ==========================================================================
 */
