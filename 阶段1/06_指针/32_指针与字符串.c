/*
 * 32_指针与字符串.c —— 阶段1 / 第 06 课 / 第 32 个文件：指针与字符串
 *
 * 编译：gcc -std=c99 -Wall -Wextra 32_指针与字符串.c -o bin/32_指针与字符串
 * 运行：./bin/32_指针与字符串
 *
 * 本文件**没有**去改字符串字面量（那是未定义行为，通常直接段错误），
 * 只在注释里说明后果。需要修改的内容都放在可写的 char 数组里。
 *
 * ====================== 一、C 没有"字符串类型" ======================
 * C 的字符串只有一个约定：
 *
 *      一串 char，以 '\0'（数值 0）结尾，用首字符的地址代表整串。
 *
 * 所以 `char *s` 能表示字符串，靠的不是类型系统，而是**约定**：
 * 函数从 s 开始一个字节一个字节往后读，读到 0 就停。
 *
 *      char s[] = "hi";
 *
 *      内存：  'h'   'i'   '\0'
 *      地址：   s    s+1   s+2
 *
 * 这带来两个后果：
 *  1) 长度不在类型里 → 求长度必须遍历（这就是 strlen 是 O(n) 的原因）
 *  2) 忘了写 '\0' → 函数会一直往后读到撞上 0，读到越界地址就是未定义行为
 *
 * ====================== 二、char s[] 与 char *s（本节核心） ======================
 *      char  s1[] = "hello";     // 数组：把 6 个字节**复制**到自己的空间里
 *      char *s2   = "hello";     // 指针：只存字面量的地址，指向只读区
 *
 *      对比项        char s1[] = "hello"        char *s2 = "hello"
 *      ------------  -------------------------  ---------------------------
 *      sizeof        6（含 '\0'）                8（指针大小）
 *      strlen        5                          5
 *      内存在哪      栈上（函数内的局部数组）     只读数据段（字面量）
 *      能改内容吗    ✓ 可以                     ✗ 未定义行为，通常崩
 *      能改指向吗    ✗ 数组名是常量             ✓ 可以 s2 = "world"
 *      赋值 b = a    ✗ 数组不能整体赋值         ✓ 只是复制地址，共享同一串
 *
 * 一句话：**[] 是"一块可写副本"，* 是"指向只读原文的指针"。**
 *
 * ★ 现代写法一律加 const：
 *      const char *s = "hello";      // 让编译器替我守住"不许改"
 *   不加 const 也能编译（C 的历史包袱），但一改就崩，而且编译器不拦你。
 *
 * ====================== 三、字符串字面量为什么不能改 ======================
 * "hello" 这种字面量在编译期就被放进**只读数据段（.rodata）**，
 * 而且相同内容的字面量**可能被合并成一份**：
 *
 *      char *a = "abc";
 *      char *b = "abc";        // a == b 很可能为真（同一份字面量）
 *
 * 所以
 *      a[0] = 'X';             // ✗ 未定义行为：写只读页 → 段错误
 * 而且改了一处可能"影响另一处"，因为两者本来就指向同一块内存。
 *
 * 想改就复制到数组里：
 *      char buf[16];
 *      strcpy(buf, "hello");
 *      buf[0] = 'H';           // ✓ 改的是自己的副本
 *
 * ====================== 四、sizeof 与 strlen 的区别 ======================
 *      char buf[16] = "abc";
 *
 *      sizeof(buf)  = 16    容器多大（编译期常量，数组独有）
 *      strlen(buf)  = 3     实际有几个字符（运行期遍历，遇 0 停）
 *
 *      sizeof("abc") = 4    （字符 + '\0'）
 *      strlen("abc") = 3
 *
 * ★ 传进函数后 sizeof 就变成指针大小了，只剩 strlen 可用。
 * ★ 给字符串分配空间时，一定要记得给 '\0' 留 1 个字节：
 *      char s[5] = "hello";   // ✗ 放不下 '\0'，数组没有结尾
 *      char s[6] = "hello";   // ✓ 5 个字符 + 1 个结尾
 *
 * ====================== 五、遍历：三种写法 ======================
 *      for (int i = 0; s[i] != '\0'; i++)  ...      // 下标
 *      for (int i = 0; p[i] != '\0'; i++)  ...      // 指针 + 下标
 *      for (const char *q = s; *q != '\0'; q++) ... // 指针自增
 *
 * 第三种可以再简写成 `for (const char *q = s; *q; q++)`：
 * 因为 '\0' 的数值就是 0，直接当条件用天然表示"还没到结尾"。
 *
 * 注意：**用指针自增遍历之后 q 已经指向结尾**，
 * 原指针 s 没变（我们改的是 q 的副本），这点和数组名不同。
 *
 * ====================== 六、多个字符串的两种存法 ======================
 *      const char *names1[3] = {"Tom", "Jerry", "Spike"};   // 指针数组
 *      char names2[3][8]     = {"Tom", "Jerry", "Spike"};   // 二维数组
 *
 *      对比          指针数组 char *a[]          二维数组 char a[][8]
 *      ------------  --------------------------  -------------------------
 *      sizeof(整体)  3 × 8 = 24（3 个指针）      3 × 8 = 24（真的字符空间）
 *      内容可写      ✗ 指向字面量                 ✓ 每行都是可写副本
 *      行宽是否相等  ✗ 每行可以任意长             ✓ 每行都必须 ≤ 7 字符
 *      内存是否连续  指针连续，指向的串不连续     整块连续
 *
 * 选择原则：
 *  - 只读、长度不齐 → 指针数组（省内存，长度自由）
 *  - 需要原地修改 → 二维数组（每行都得够宽，可能有浪费）
 *
 * ====================== 七、会用指针返回值的标准库函数 ======================
 *      char *strchr (const char *s, int c);        // 找字符，返回指向它的指针
 *      char *strstr (const char *s, const char *sub); // 找子串
 *      char *strcpy (char *dst, const char *src);  // 返回 dst
 *      char *strcat (char *dst, const char *src);  // 返回 dst
 *
 * 它们找不到时返回 **NULL**，所以**必须判空**再用。
 * 返回值是"字符串内部的地址"，所以 `hit - text` 就是命中位置的下标。
 *
 * ★ 这些函数都不检查目标缓冲区大小：strcpy 目标不够大就是缓冲区溢出。
 *   工程上优先用带 n 的版本（strncpy / strncat / snprintf），并自己补 '\0'。
 *
 * ====================== 八、常见坑 ======================
 *  1. 忘记 '\0'                          char s[3] = {'a','b','c'}; 不是字符串
 *  2. 用 == 比较字符串内容               比的是地址，要 strcmp(s1,s2) == 0
 *  3. 修改字符串字面量                   char *s = "abc"; s[0]='X'; 未定义行为
 *  4. 数组太小装不下结尾                 char s[5] = "hello"; 少一个字节
 *  5. 函数里 sizeof 字符串不等于长度     退化成 char *，只能用 strlen
 *  6. strcpy 目标不够大                  经典缓冲区溢出，优先用 snprintf
 *  7. 对 NULL 调 strlen/strcpy           直接崩，先判空
 *  8. 以为 char 是"字符"就一定是 1 个汉字 UTF-8 下一个汉字 3 字节，
 *                                        strlen 数的是**字节**不是字符
 *  9. 指针自增遍历后原指针丢了           用 q 遍历，别改 s
 * 10. 用 char * 接 const 字符串再改       编译器警告要当真（discards const）
 *
 * ====================== 九、实践建议 ======================
 *  - 只读字符串一律 `const char *`
 *  - 需要修改就先复制到数组，别指望字面量可写
 *  - 目标缓冲区大小当作参数传进函数（`void f(char *buf, size_t cap)`）
 *  - 比较用 strcmp / strncmp，赋值用 strcpy / snprintf，长度用 strlen
 *  - 分配空间时永远记得 +1 给 '\0'
 *  - 任何返回指针的字符串函数，用完先判 NULL
 */

#include <stdio.h>
#include <string.h>

/* 自己实现 strlen：看清"靠结尾的 0 停下来"这件事 */
static size_t my_strlen(const char *s)
{
    const char *p = s;
    while (*p != '\0')
        p++;
    return (size_t)(p - s);
}

/* 原地反转一个可写的字符串（双指针，O(1) 空间） */
static void reverse_str(char *s)
{
    char *left = s;
    char *right = s + my_strlen(s);
    while (left < right) {
        --right;
        char t = *left;
        *left = *right;
        *right = t;
        ++left;
    }
}

int main(void)
{
    /* ---------- 1. char[] 与 char* 的本质区别 ---------- */
    printf("1. char s[] 与 char *s 看着像，其实完全不同\n");
    char s1[] = "hello";                 /* 数组：6 字节可写副本 */
    const char *s2 = "hello";            /* 指针：指向只读字面量 */
    printf("   char s1[] = \"hello\";  sizeof=%zu  strlen=%zu  → 栈上的副本，可改\n",
           sizeof(s1), strlen(s1));
    printf("   char *s2   = \"hello\";  sizeof=%zu  strlen=%zu  → 只读字面量的地址\n",
           sizeof(s2), strlen(s2));
    s1[0] = 'H';
    printf("   s1[0] = 'H' 之后 s1 = \"%s\"（数组是副本，随便改）\n", s1);
    /* s2[0] = 'H';   ✗ 未定义行为：字面量在只读段，通常直接段错误 */
    printf("   s2[0] = 'H' 会崩，因为 s2 指向只读段（本程序不跑这一行）\n");
    printf("   好习惯：只读就写 const char *，让编译器替你守着\n");

    /* ---------- 2. 三种遍历写法 ---------- */
    printf("2. 三种遍历，本质都是走到结尾 0 就停\n");
    printf("   ① 下标 s1[i] ：");
    for (size_t i = 0; s1[i] != '\0'; i++)
        putchar(s1[i]);
    printf("\n   ② 指针 p[i]  ：");
    const char *p = s1;
    for (size_t i = 0; p[i] != '\0'; i++)
        putchar(p[i]);
    printf("\n   ③ *q++ 自增  ：");
    for (const char *q = s1; *q; q++)
        putchar(*q);
    printf("\n   *q 直接当条件：'\\0' 的数值就是 0，天然表示「到头了」\n");

    /* ---------- 3. sizeof vs strlen ---------- */
    printf("3. sizeof 是容器大小，strlen 是实际长度\n");
    char buf[16] = "abc";
    printf("   char buf[16] = \"abc\";  sizeof=%zu  strlen=%zu\n",
           sizeof(buf), strlen(buf));
    printf("   剩下 %zu 个字节全是 0（因为写了初始化列表，其余补 0）\n",
           sizeof(buf) - strlen(buf));
    printf("   传进函数后 sizeof 只剩 %zu（指针大小），所以长度只能靠 strlen\n",
           sizeof(const char *));
    printf("   分配空间记得给结尾留 1 个字节：\"hello\" 至少要 char s[6]\n");

    /* ---------- 4. 自己写 strlen，看结尾 0 ---------- */
    printf("4. 自己实现 strlen\n");
    printf("   my_strlen(\"%s\") = %zu\n", s1, my_strlen(s1));
    const char *cn = "指针与字符串";
    printf("   my_strlen(\"%s\") = %zu（数的是字节数，UTF-8 一个汉字 3 字节）\n",
           cn, my_strlen(cn));

    /* ---------- 5. 多个字符串：指针数组 vs 二维数组 ---------- */
    printf("5. 存多个字符串的两种形态\n");
    const char *names1[3] = {"Tom", "Jerry", "Spike"};   /* 指针数组，长度随意 */
    char names2[3][8] = {"Tom", "Jerry", "Spike"};       /* 二维数组，每行 ≤ 7 字符 */
    printf("   const char *names1[3] → sizeof=%zu（3 个指针，指向 3 段字面量）\n",
           sizeof(names1));
    printf("   char names2[3][8]     → sizeof=%zu（3 × 8 字节，整块连续可写）\n",
           sizeof(names2));
    for (int i = 0; i < 3; i++)
        printf("      names1[%d]=\"%s\"  names2[%d]=\"%s\"\n",
               i, names1[i], i, names2[i]);
    names2[0][0] = 't';
    printf("   names2[0][0]='t' → \"%s\"（二维数组是可写副本）；names1 做不到\n",
           names2[0]);
    printf("   &names2[1] - &names2[0] = %ld 个元素（每个元素是 char[8]）\n",
           (long)(&names2[1] - &names2[0]));

    /* ---------- 6. 返回指针的标准库函数 ---------- */
    printf("6. strchr / strstr 返回的是「串内部的指针」\n");
    const char *text = "hello world";
    const char *hit = strchr(text, 'o');          /* 返回指向第一个 'o' 的指针 */
    if (hit != NULL)                              /* ★ 一定要判空 */
        printf("   strchr(\"%s\", 'o') → 偏移 %ld，剩下的串是 \"%s\"\n",
               text, (long)(hit - text), hit);
    const char *sub = strstr(text, "world");
    if (sub != NULL)
        printf("   strstr(\"%s\", \"world\") → 偏移 %ld\n", text, (long)(sub - text));
    const char *miss = strchr(text, 'z');
    printf("   strchr(\"%s\", 'z') 返回 %s（找不到就是 NULL，解引用必崩）\n",
           text, (miss == NULL) ? "NULL" : "非空");
    printf("   因为返回的是指针，hit - text 就是下标 —— 这是「返回指针」的典型好处\n");

    /* ---------- 7. 用 == 比不了内容 ---------- */
    printf("7. 字符串比较：== 比地址，strcmp 比内容\n");
    char c1[4] = "abc";
    char c2[4];
    strcpy(c2, "abc");                            /* 复制内容到另一个数组 */
    const char *pa = c1;
    const char *pb = c2;
    printf("   c1、c2 内容相同：strcmp(c1, c2) == 0 为%s\n",
           (strcmp(c1, c2) == 0) ? "真" : "假");
    printf("   但 pa == pb 为%s，因为它们是两个不同数组的首地址\n",
           (pa == pb) ? "真" : "假");
    printf("   更坑的是：两个内容相同的字面量可能被合并，== 有时「碰巧」为真\n");
    printf("   所以内容比较只认 strcmp / strncmp\n");

    /* ---------- 8. 修改字符串必须用可写副本 ---------- */
    printf("8. 想改就先复制到数组\n");
    char work[16];
    strcpy(work, "hello");
    printf("   strcpy 之后 work = \"%s\"（%zu 字节的栈副本）\n", work, sizeof(work));
    reverse_str(work);
    printf("   原地反转后 work = \"%s\"\n", work);
    printf("   整个过程只动了 work 自己，字面量一个字没碰\n");

    /* ---------- 9. 小结 ---------- */
    printf("9. 小结：\n");
    printf("    C 的字符串 = 一串 char + 结尾的 '\\0'，用首地址代表整串\n");
    printf("    char[] 是可写副本（sizeof 是容器大小），char* 指向只读字面量\n");
    printf("    strlen 数到 0 为止，sizeof 只有数组才有真实大小\n");
    printf("    指针数组存多个不等长只读串，二维数组存多个可写等宽串\n");
    printf("    strchr/strstr 返回指针，可能为 NULL，用前先判空\n");
    printf("    比较内容用 strcmp，永远不要用 ==\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 32_指针与字符串.c -o bin/32_指针与字符串
 *     ./bin/32_指针与字符串
 *
 * 输出如下：
 *
 * 1. char s[] 与 char *s 看着像，其实完全不同
 *    char s1[] = "hello";  sizeof=6  strlen=5  → 栈上的副本，可改
 *    char *s2   = "hello";  sizeof=8  strlen=5  → 只读字面量的地址
 *    s1[0] = 'H' 之后 s1 = "Hello"（数组是副本，随便改）
 *    s2[0] = 'H' 会崩，因为 s2 指向只读段（本程序不跑这一行）
 *    好习惯：只读就写 const char *，让编译器替你守着
 * 2. 三种遍历，本质都是走到结尾 0 就停
 *    ① 下标 s1[i] ：Hello
 *    ② 指针 p[i]  ：Hello
 *    ③ *q++ 自增  ：Hello
 *    *q 直接当条件：'\0' 的数值就是 0，天然表示「到头了」
 * 3. sizeof 是容器大小，strlen 是实际长度
 *    char buf[16] = "abc";  sizeof=16  strlen=3
 *    剩下 13 个字节全是 0（因为写了初始化列表，其余补 0）
 *    传进函数后 sizeof 只剩 8（指针大小），所以长度只能靠 strlen
 *    分配空间记得给结尾留 1 个字节："hello" 至少要 char s[6]
 * 4. 自己实现 strlen
 *    my_strlen("Hello") = 5
 *    my_strlen("指针与字符串") = 18（数的是字节数，UTF-8 一个汉字 3 字节）
 * 5. 存多个字符串的两种形态
 *    const char *names1[3] → sizeof=24（3 个指针，指向 3 段字面量）
 *    char names2[3][8]     → sizeof=24（3 × 8 字节，整块连续可写）
 *       names1[0]="Tom"  names2[0]="Tom"
 *       names1[1]="Jerry"  names2[1]="Jerry"
 *       names1[2]="Spike"  names2[2]="Spike"
 *    names2[0][0]='t' → "tom"（二维数组是可写副本）；names1 做不到
 *    &names2[1] - &names2[0] = 1 个元素（每个元素是 char[8]）
 * 6. strchr / strstr 返回的是「串内部的指针」
 *    strchr("hello world", 'o') → 偏移 4，剩下的串是 "o world"
 *    strstr("hello world", "world") → 偏移 6
 *    strchr("hello world", 'z') 返回 NULL（找不到就是 NULL，解引用必崩）
 *    因为返回的是指针，hit - text 就是下标 —— 这是「返回指针」的典型好处
 * 7. 字符串比较：== 比地址，strcmp 比内容
 *    c1、c2 内容相同：strcmp(c1, c2) == 0 为真
 *    但 pa == pb 为假，因为它们是两个不同数组的首地址
 *    更坑的是：两个内容相同的字面量可能被合并，== 有时「碰巧」为真
 *    所以内容比较只认 strcmp / strncmp
 * 8. 想改就先复制到数组
 *    strcpy 之后 work = "hello"（16 字节的栈副本）
 *    原地反转后 work = "olleh"
 *    整个过程只动了 work 自己，字面量一个字没碰
 * 9. 小结：
 *     C 的字符串 = 一串 char + 结尾的 '\0'，用首地址代表整串
 *     char[] 是可写副本（sizeof 是容器大小），char* 指向只读字面量
 *     strlen 数到 0 为止，sizeof 只有数组才有真实大小
 *     指针数组存多个不等长只读串，二维数组存多个可写等宽串
 *     strchr/strstr 返回指针，可能为 NULL，用前先判空
 *     比较内容用 strcmp，永远不要用 ==
 *
 * 几个容易忽略的对照点：
 *   第 1 条：同样写着 "hello"，sizeof 一个是 6 一个是 8。
 *           数组的 sizeof 是"容器有多大"（含 '\0'），指针的 sizeof 是"地址有多长"。
 *   第 2 条：三种写法读到的是同一串字符；差别只在"谁在动"——
 *           下标写法动的是 i，指针写法动的是 q（s1 本身始终没变）。
 *   第 3 条：char buf[16] = "abc" 会把 16 字节全填好（后面 13 个是 0），
 *           所以 strlen 能安全地在这里停下，这是"数组初始化补 0"的功劳。
 *   第 4 条：18 是字节数，不是字符数 —— "指针与字符串" 6 个汉字 × 3 = 18。
 *           strlen 永远数字节，想按"人眼看到的字符"数得用宽字符/mbrtowc。
 *   第 5 条：两种形态 sizeof 都是 24，纯属巧合（3 个指针 vs 3×8 字节），
 *           但一个是"24 字节的指针表"，一个是"24 字节的真实字符空间"，
 *           所以只有后者能 names2[0][0] = 't'。
 *   第 6 条：命中时返回的是"串内部的地址"，所以 hit - text 直接是下标；
 *           找不到时返回 NULL，不判空就解引用是最常见的崩溃原因之一。
 *   第 7 条：strcmp 内容同 → 0 为真；而 pa/pb 指向两个不同数组，
 *           地址比较必然为假。字面量会被合并这点更要命：
 *           == 偶尔"对"，等于把 bug 藏起来了。
 *   第 8 条：work 是 char[16] 的栈副本，strcpy 复制内容，reverse 原地改，
 *           全程没碰 "hello" 那份只读字面量。
 * ==========================================================================
 */
