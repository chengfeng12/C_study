/*
 * 03_char数组与字符串.c —— 阶段1 / 第 05 课 / 第 03 个文件：char 数组与字符串
 *
 * 编译：gcc 03_char数组与字符串.c -o bin/03_char数组与字符串
 * 运行：./bin/03_char数组与字符串
 *
 * 本文件能干净编译（-Wall -Wextra 零警告）。
 * 第 6、8 节的"错误写法"只放在注释里（改字面量会崩、不带 '\0' 打印会越界读）。
 *
 * ====================== 一、C 没有"字符串类型" ======================
 * 别的语言里 String 是一种类型；C 里**没有**。字符串只是：
 *
 *      以 '\0' 结尾的 char 数组
 *
 * 两个必须分清的常量：
 *
 *      'a'     字符常量，类型 char，值是 97，占 1 字节
 *      "a"     字符串字面量，是 char[2]：'a' + '\0'，占 2 字节
 *
 * 所以 sizeof('a') == 4（在 C 里字符常量是 int！），sizeof("a") == 2。
 * 初学时最容易晕的就是这一对。
 *
 * ====================== 二、三种定义方式，区别很大 ======================
 *
 *  (1) char s[] = "hello";        → 数组 6 字节，**内容可改**
 *  (2) char s[10] = "hello";      → 数组 10 字节，后 4 字节补 0，内容可改
 *  (3) char *p  = "hello";        → 指针 8 字节，指向**只读**的字面量，内容不可改
 *
 *      s[0] = 'H';    ✓  (1)(2) 可以改
 *      p[0] = 'H';    ✗  未定义行为，通常直接段错误（字面量在只读段）
 *
 * 记忆法：
 *      数组 = 自己的稿纸，能随便改
 *      指针 = 指着别人的书，只能读
 *
 * ====================== 三、'\0' 是什么，为什么必须有 ======================
 *      char s[] = "hello";
 *
 *      下标   0    1    2    3    4    5
 *      字符   h    e    l    l    o    \0
 *      数值  104  101  108  108  111   0
 *                                       └── 结束标记（NUL）
 *
 * 所有字符串函数（strlen、strcpy、printf("%s")）都是**从首地址一路往后找 '\0'**，
 * 找到才停。所以：
 *
 *      - 没有 '\0' 的 char 数组**不是字符串**，只是一串字符
 *      - 少了 '\0' 时，函数会继续读到数组外面 —— 越界读，未定义行为
 *
 * 数组用字面量初始化时，编译器会**自动**在末尾补 '\0'：
 *
 *      char a[] = "abc";          → 4 字节，a[3] = '\0'
 *      char b[3] = "abc";         → ✗ 3 字节放不下 '\0'（C 允许，但 b 不是字符串）
 *      char c[3] = {'a','b','c'}; → 同样是"字符数组"，不是字符串
 *
 * ====================== 四、'\0'、'0'、0 是三个东西 ======================
 *
 *      '\0'   转义字符，数值 0      → 字符串结束标记
 *      '0'    字符零，数值 48       → 用于显示"0"这个字形
 *      0      整数零
 *
 * 常见错误：用 `if (s[i] == '0')` 判断字符串结尾 —— 那是在找字符 '0'，
 *          正确写法是 `if (s[i] == '\0')` 或直接 `if (!s[i])`。
 *
 * ====================== 五、sizeof 和 strlen 又来了 ======================
 *      char s[] = "hello";
 *
 *      sizeof(s)     = 6   数组总共多少字节（含 '\0'），编译期就知道
 *      strlen(s)     = 5   到 '\0' 之前有多少个字符，运行期从头数
 *      sizeof(&s[0]) = 8   指针大小
 *
 * 换成指针就更明显：
 *      char *p = "hello";
 *      sizeof(p)     = 8   指针本身
 *      strlen(p)     = 5   字符串内容长度
 *
 * 记住：**sizeof 问的是"占多少内存"，strlen 问的是"字符串多长"。**
 *
 * ====================== 六、常用字符串函数（<string.h>） ======================
 *
 *      strlen(s)                长度，不含 '\0'
 *      strcpy(dst, src)         复制（含 '\0'），**不检查容量**
 *      strncpy(dst, src, n)     最多复制 n 个字符，**不保证补 '\0'**
 *      strcat(dst, src)         追加（含 '\0'），**不检查容量**
 *      strcmp(a, b)             比较内容：0 相等、<0 a 小、>0 a 大
 *      strncmp(a, b, n)         只比前 n 个字符
 *      memset(p, c, n)          把 n 个字节都设成 c
 *      memcpy / memmove         按字节搬（不关心 '\0'）
 *
 * 两个高频坑：
 *      1) 判等千万别用 ==：`s1 == s2` 比的是地址；要用 `strcmp(s1, s2) == 0`
 *      2) strcpy/strcat 不知道目标有多大，越长越容易写爆缓冲区，
 *         工程代码里用 strncpy + 手动补 '\0'，或者 snprintf
 *
 * ====================== 七、遍历字符串 ======================
 *      for (int i = 0; s[i] != '\0'; i++)       // 最自然，靠结束标记
 *          ...
 *
 *      int n = (int)strlen(s);                  // 先算长度，少调多次
 *      for (int i = 0; i < n; i++)
 *          ...
 *
 * 别在 for 的循环条件里反复调 strlen（每次都要从头扫一遍）：
 *      for (i = 0; i < strlen(s); i++)          // ✗ O(n²)
 * 如果字符串中途会被改长度就更不能用这种写法。
 *
 * ====================== 八、读入字符串：scanf 要限宽 ======================
 *      char buf[10];
 *      scanf("%s", buf);            // ✗ 输入超过 9 个字符就溢出
 *      scanf("%9s", buf);           // ✓ 最多读 9 个，留一位给 '\0'
 *
 *      fgets(buf, sizeof buf, stdin);   // ✓ 更安全，整行读，但会带 '\n'
 *
 * scanf %s 遇到空白（空格、Tab、换行）就停，所以它只能读"一个词"；
 * 整行要用 fgets（并处理末尾的 '\n'）。
 *
 * ====================== 九、常见坑 ======================
 *  1. 数组开小了放不下 '\0'       char b[3] = "abc"; 不是字符串
 *  2. 用 == 比较字符串           比的是地址，要用 strcmp
 *  3. 忘记 '\0' 就打印/strlen    一直读到越界，结果不可预测
 *  4. scanf("%s") 不限宽          输入长了就溢出（缓冲区攻击的经典入口）
 *  5. 改字符串字面量              char *p = "abc"; p[0] = 'x'; → 崩溃
 *  6. 把 sizeof 当 strlen 用      数组 6、指针 8，都不是"字符数"
 *  7. 中文按"字"算长度            UTF-8 下一个汉字 3 字节，strlen("中文")=6
 *  8. strncpy 之后忘了补 '\0'     手动 dst[n] = '\0'，或改用 snprintf
 *  9. strcat 前没检查容量         目标数组放不下就溢出
 * 10. 用 char 接 getchar 的返回值  EOF 是 -1，char 可能装不下，要用 int
 *
 * ====================== 十、实践建议 ======================
 *  - 存字符串优先用 `char s[] = "...";`，需要改就一定是数组不是指针
 *  - 数组长度宁大勿小，留够 '\0' 和后续追加的空间
 *  - 判等 strcmp、求长 strlen、复制 strcpy/strncpy、拼接 strcat 前查容量
 *  - 输入一律限宽（%9s）或 fgets，永不裸写 scanf("%s")
 *  - 遍历用 `s[i] != '\0'`，循环里不要反复调 strlen
 *  - 处理中文/UTF-8 时要清楚：strlen 数的是字节，不是字
 */

#include <stdio.h>
#include <string.h>          /* strlen / strcpy / strncpy / strcat / strcmp / memset */

int main(void)
{
    /* ---------- 1. 字符常量 vs 字符串字面量 ---------- */
    printf("1. 'a' 是字符，\"a\" 是 2 字节的字符串\n");
    char ch = 'a';
    const char *lit = "a";
    printf("   'a' 的数值 = %d，sizeof('a') = %zu（C 里字符常量类型是 int）\n",
           ch, sizeof('a'));
    printf("   \"a\" 的长度 strlen = %zu，sizeof(\"a\") = %zu（多了 '\\0'）\n",
           strlen(lit), sizeof("a"));
    printf("   所以字符串永远比看到的字符多一个字节\n");

    /* ---------- 2. 三种定义方式 ---------- */
    printf("2. 三种定义方式的差别\n");
    char s1[] = "hello";                 /* 6 字节，可改 */
    char s2[10] = "hello";               /* 10 字节，后 4 字节补 0，可改 */
    char *s3 = "hello";                  /* 指针 8 字节，指向只读字面量 */

    printf("   char s1[]  = \"hello\";  sizeof=%zu  strlen=%zu\n",
           sizeof(s1), strlen(s1));
    printf("   char s2[10]= \"hello\";  sizeof=%zu  strlen=%zu\n",
           sizeof(s2), strlen(s2));
    printf("   char *s3   = \"hello\";  sizeof=%zu（指针！）  strlen=%zu\n",
           sizeof(s3), strlen(s3));
    printf("   s1、s2 是数组可修改；s3 指向只读字面量，改了就崩\n");

    /* ---------- 3. 内存布局：逐字节看 ---------- */
    printf("3. \"abc\" 在内存里的样子\n");
    char abc[] = "abc";
    for (int i = 0; i <= 3; i++) {
        if (abc[i] == '\0')
            printf("   abc[%d] = '\\0' (%d) ← 结束标记\n", i, abc[i]);
        else
            printf("   abc[%d] = '%c' (%d)\n", i, abc[i], abc[i]);
    }
    printf("   总共 %zu 字节：3 个字符 + 1 个 '\\0'\n", sizeof(abc));

    /* ---------- 4. '\0'、'0'、0 ---------- */
    printf("4. '\\0'、'0'、0 是三样不同的东西\n");
    char nul = '\0';
    char zero_ch = '0';
    int  zero_int = 0;
    printf("   '\\0' 的数值 = %d\n", nul);
    printf("   '0'  的数值 = %d（用于显示字符 0）\n", zero_ch);
    printf("   0    的数值 = %d（整数零）\n", zero_int);
    printf("   判断结尾要写 s[i] == '\\0'，写成 '0' 会找错位置\n");

    /* ---------- 5. 遍历统计 ---------- */
    printf("5. 遍历字符串：统计字母、数字、其他\n");
    const char *text = "C99 make 2025 easy!";
    int letters = 0, digits = 0, others = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        unsigned char c = (unsigned char)text[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            letters++;
        else if (c >= '0' && c <= '9')
            digits++;
        else
            others++;
    }
    printf("   \"%s\"：字母 %d，数字 %d，其他 %d，总长 %zu\n",
           text, letters, digits, others, strlen(text));

    /* ---------- 6. 修改字符串 ---------- */
    printf("6. 数组能改，指针指向的字面量不能改\n");
    char mut[] = "hello";
    mut[0] = 'H';
    mut[1] = 'E';
    printf("   char mut[] = \"hello\"; mut[0]='H'; → \"%s\"\n", mut);
    printf("   错误写法（会崩，别试）：char *p = \"hello\"; p[0] = 'H';\n");
    printf("   因为字面量放在只读段，写它属于未定义行为（通常段错误）\n");

    /* ---------- 7. 常用字符串函数 ---------- */
    printf("7. strlen / strcpy / strcat / strcmp\n");
    char dst[20];
    strcpy(dst, "abc");
    printf("   strcpy 后 dst = \"%s\"（长度 %zu）\n", dst, strlen(dst));
    strcat(dst, "def");
    printf("   strcat 追加 \"def\" 后 dst = \"%s\"（长度 %zu）\n", dst, strlen(dst));

    int r1 = strcmp("apple", "apple");
    int r2 = strcmp("apple", "banana");
    int r3 = strcmp("banana", "apple");
    printf("   strcmp(\"apple\",\"apple\")  = %s\n", r1 == 0 ? "0（相等）" : "非 0");
    printf("   strcmp(\"apple\",\"banana\") = %s\n", r2 < 0 ? "负数（前者小）" : "非负");
    printf("   strcmp(\"banana\",\"apple\") = %s\n", r3 > 0 ? "正数（前者大）" : "非正");
    printf("   注意：不要用 == 比较字符串内容，那只比地址\n");

    /* ---------- 8. strncpy 不一定补 '\0' ---------- */
    printf("8. 坑：strncpy 不保证补 '\\0'\n");
    char nd[8];
    memset(nd, 'X', sizeof(nd));         /* 先填满可见字符，方便观察 */
    strncpy(nd, "hello", 3);             /* 只复制 3 个字符，不补 '\0' */
    printf("   strncpy(nd, \"hello\", 3) 后逐字节：");
    for (int i = 0; i < 8; i++) printf("%c", nd[i]);
    printf("\n   前 3 个是 \"hel\"，后面仍是原来的 'X'，**没有结束标记**\n");
    nd[3] = '\0';                        /* ★ 手动补上 */
    printf("   补上 nd[3] = '\\0' 后才能安全当字符串用：\"%s\"\n", nd);

    /* ---------- 9. scanf/sscanf 要限宽 ---------- */
    printf("9. 读字符串要限宽：%%5s 最多读 5 个字符\n");
    char w1[6] = {0};
    char w2[6] = {0};
    sscanf("abcdefghij 123", "%5s %5s", w1, w2);   /* 模拟用户输入 */
    printf("   输入 \"abcdefghij 123\"，%s + %s\n", w1, w2);
    printf("   %%s 遇到空白就停；写成 %%5s 才不会把 w1[6] 写爆\n");
    printf("   真实输入整行建议 fgets(buf, sizeof buf, stdin)，注意末尾 '\\n'\n");

    /* ---------- 10. 中文与 UTF-8：strlen 数的是字节 ---------- */
    printf("10. 中文按字节算：UTF-8 下一个汉字占 3 字节\n");
    const char *cn = "中文";
    printf("    strlen(\"中文\") = %zu（不是 2）\n", strlen(cn));
    printf("    sizeof(\"中文\") = %zu（2 个汉字 × 3 字节 + '\\0'）\n", sizeof("中文"));
    printf("    sizeof(cn)    = %zu（指针大小，别看错）\n", sizeof(cn));
    printf("    所以 char buf[10] 最多放 3 个汉字，长度要按字节算\n");

    /* ---------- 11. 小结 ---------- */
    printf("11. 小结：\n");
    printf("    C 的字符串 = 以 '\\0' 结尾的 char 数组，没有 String 类型\n");
    printf("    'a' 是字符(1 字节)，\"a\" 是字符串(2 字节：字符 + '\\0')\n");
    printf("    数组能改、指针指向的字面量不能改\n");
    printf("    sizeof 问内存大小，strlen 问字符个数，两者差 1\n");
    printf("    判等用 strcmp、复制用 strcpy/strncpy（注意补 '\\0'）、输入要限宽\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc 03_char数组与字符串.c -o bin/03_char数组与字符串 && ./bin/03_char数组与字符串
 *
 * 输出如下：
 *
 * 1. 'a' 是字符，"a" 是 2 字节的字符串
 *    'a' 的数值 = 97，sizeof('a') = 4（C 里字符常量类型是 int）
 *    "a" 的长度 strlen = 1，sizeof("a") = 2（多了 '\0'）
 *    所以字符串永远比看到的字符多一个字节
 * 2. 三种定义方式的差别
 *    char s1[]  = "hello";  sizeof=6  strlen=5
 *    char s2[10]= "hello";  sizeof=10  strlen=5
 *    char *s3   = "hello";  sizeof=8（指针！）  strlen=5
 *    s1、s2 是数组可修改；s3 指向只读字面量，改了就崩
 * 3. "abc" 在内存里的样子
 *    abc[0] = 'a' (97)
 *    abc[1] = 'b' (98)
 *    abc[2] = 'c' (99)
 *    abc[3] = '\0' (0) ← 结束标记
 *    总共 4 字节：3 个字符 + 1 个 '\0'
 * 4. '\0'、'0'、0 是三样不同的东西
 *    '\0' 的数值 = 0
 *    '0'  的数值 = 48（用于显示字符 0）
 *    0    的数值 = 0（整数零）
 *    判断结尾要写 s[i] == '\0'，写成 '0' 会找错位置
 * 5. 遍历字符串：统计字母、数字、其他
 *    "C99 make 2025 easy!"：字母 9，数字 6，其他 4，总长 19
 * 6. 数组能改，指针指向的字面量不能改
 *    char mut[] = "hello"; mut[0]='H'; → "HEllo"
 *    错误写法（会崩，别试）：char *p = "hello"; p[0] = 'H';
 *    因为字面量放在只读段，写它属于未定义行为（通常段错误）
 * 7. strlen / strcpy / strcat / strcmp
 *    strcpy 后 dst = "abc"（长度 3）
 *    strcat 追加 "def" 后 dst = "abcdef"（长度 6）
 *    strcmp("apple","apple")  = 0（相等）
 *    strcmp("apple","banana") = 负数（前者小）
 *    strcmp("banana","apple") = 正数（前者大）
 *    注意：不要用 == 比较字符串内容，那只比地址
 * 8. 坑：strncpy 不保证补 '\0'
 *    strncpy(nd, "hello", 3) 后逐字节：helXXXXX
 *    前 3 个是 "hel"，后面仍是原来的 'X'，**没有结束标记**
 *    补上 nd[3] = '\0' 后才能安全当字符串用："hel"
 * 9. 读字符串要限宽：%5s 最多读 5 个字符
 *    输入 "abcdefghij 123"，abcde + fghij
 *    %s 遇到空白就停；写成 %5s 才不会把 w1[6] 写爆
 *    真实输入整行建议 fgets(buf, sizeof buf, stdin)，注意末尾 '\n'
 * 10. 中文按字节算：UTF-8 下一个汉字占 3 字节
 *     strlen("中文") = 6（不是 2）
 *     sizeof("中文") = 7（2 个汉字 × 3 字节 + '\0'）
 *     sizeof(cn)    = 8（指针大小，别看错）
 *     所以 char buf[10] 最多放 3 个汉字，长度要按字节算
 * 11. 小结：
 *     C 的字符串 = 以 '\0' 结尾的 char 数组，没有 String 类型
 *     'a' 是字符(1 字节)，"a" 是字符串(2 字节：字符 + '\0')
 *     数组能改、指针指向的字面量不能改
 *     sizeof 问内存大小，strlen 问字符个数，两者差 1
 *     判等用 strcmp、复制用 strcpy/strncpy（注意补 '\0'）、输入要限宽
 *
 * 几个容易忽略的对照点：
 *   第 1 条：sizeof('a') 是 4 不是 1 —— C 里字符常量被当作 int，
 *           而 char 变量才占 1 字节，这一点和 C++ 不同（C++ 是 1）。
 *   第 2 条：三个 sizeof 分别是 6 / 10 / 8，唯一没变的是 strlen —— 5。
 *           看到 sizeof 是 8 就要意识到"这是指针，不是数组"。
 *   第 3 条：'a' 是 97、'\0' 是 0，字符串靠 0 来标记结尾，
 *           所以字符串里不能出现 0 这个字节（二进制数据要用 memcpy 而不是 strcpy）。
 *   第 5 条：统计"字母 9、数字 6、其他 4"加起来正好等于 19 个可见字节。
 *   第 8 条：helXXXXX 里看不到 '\0'，说明 strncpy 只照搬了 3 个字符；
 *           直接 printf("%s", nd) 会继续往后读，属于未定义行为。
 *   第 9 条：%5s 让 "abcdefghij" 被切成 "abcde" 和 "fghij" 两段，
 *           正好演示"限宽"如何避免写爆缓冲区。
 *   第 10 条：strlen 数的是字节，不是"字"；对 UTF-8 中文来说
 *            一个汉字 = 3 字节 + 1 个结束符，所以 char buf[10] 只够 3 个汉字。
 * ==========================================================================
 */
