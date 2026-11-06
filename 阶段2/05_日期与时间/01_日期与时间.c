/*
 * 01_日期与时间.c —— 阶段2 / 第 05 课 / 第 01 个文件：时间日期函数
 *
 * 编译：gcc -std=c99 -Wall -Wextra 01_日期与时间.c -o bin/01_日期与时间
 * 运行：./bin/01_日期与时间
 *
 * C 标准库把"时间"拆成三样东西，先分清楚再写代码：
 *
 *      概念              类型        含义
 *      ────────────────────────────────────────────────────────────
 *      日历时间          time_t      从 1970-01-01 00:00:00 UTC 起算的"秒数"
 *      分解时间          struct tm   年月日时分秒拆成 9 个字段，给人看
 *      处理器时间        clock_t     进程消耗的 CPU 时钟数，用来计时
 *
 * 本篇按"取 → 转 → 算"的顺序展开：
 *      一、time()：拿到时间戳
 *      二、localtime() / gmtime()：时间戳 → 结构体（附静态缓冲区陷阱）
 *      三、struct tm 九个字段的取值起点（最容易错的地方）
 *      四、strftime()：结构体 → 字符串
 *      五、mktime()：结构体 → 时间戳，以及它的"自动规范化"用法
 *      六、difftime()：两个时间戳求差
 *      七、自己写日期函数：闰年、当月天数、年内第几天
 *      八、日期加减：借 mktime() 做进位
 *      九、算星期几
 *     十、clock()：量一段代码的耗时
 *     十一、坑与 2038 问题
 *
 * ★ 一句话：标准库只提供"时间戳 ↔ 结构体 ↔ 字符串"三者的互转，
 *   真正的日期业务（闰年、月末、区间天数）要自己写，本篇第七、八节就是示范。
 *
 * ====================== 一、time()：拿到时间戳 ======================
 *      time_t now = time(NULL);
 *
 *  - 参数可以传指针，函数会把结果同时写进去；传 NULL 表示"我只要返回值"
 *  - 失败返回 (time_t)-1，**必须判断**：这是唯一能表示失败的返回值
 *  - time_t 标准只保证是"算术类型"，宽度和符号由实现决定，
 *    所以不要写 printf("%d", now)，也不要假设它是 int
 *
 * ★ time_t 的单位是"秒"，不是毫秒；要更高精度得用 timespec_get()（C11）或平台接口。
 *
 * ====================== 二、localtime() / gmtime() ======================
 *      struct tm *localtime(const time_t *t);   // 转成本地时间（含时区、夏令时）
 *      struct tm *gmtime  (const time_t *t);    // 转成 UTC（零时区）
 *
 * ★ 最大的坑：**这两个函数返回的是库内部的静态对象**。
 *   也就是说，连续调用两次，第二次会把第一次的结果覆盖掉：
 *
 *      struct tm *a = localtime(&t1);
 *      struct tm *b = localtime(&t2);
 *      printf("%d %d\n", a->tm_mday, b->tm_mday);   // 两个值一模一样！
 *
 *   两种解法：
 *      1) 拿到就用、或立刻做一次值拷贝：struct tm snap = *localtime(&t);  ← 本篇采用
 *      2) 用可重入版本 localtime_r() / gmtime_r()（POSIX，不是 C 标准，
 *         -std=c99 严格模式下可能没有声明，跨平台代码慎用）
 *
 * 两者都可能返回 NULL（时区数据异常等），用前判空。
 *
 * ====================== 三、struct tm 的九个字段 ======================
 *      int tm_sec;    // 秒      0–60（60 是闰秒，标准允许）
 *      int tm_min;    // 分      0–59
 *      int tm_hour;   // 时      0–23
 *      int tm_mday;   // 日      1–31      ← 从 1 开始
 *      int tm_mon;    // 月      0–11      ← 从 0 开始！1 月是 0
 *      int tm_year;   // 年      实际年份 - 1900   ← 2026 年存的是 126
 *      int tm_wday;   // 星期    0–6，0 是周日
 *      int tm_yday;   // 年内第几天 0–365，0 是 1 月 1 日
 *      int tm_isdst;  // 夏令时   >0 是，0 否，<0 未知
 *
 * ★ 三个反直觉的起点记住了，90% 的日期 bug 就不会有：
 *      tm_year 减了 1900，tm_mon 减了 1，tm_yday 和 tm_wday 从 0 开始，
 *      只有 tm_mday 是"人类习惯"的从 1 开始。
 *
 * ====================== 四、strftime()：结构体 → 字符串 ======================
 *      size_t strftime(char *buf, size_t max, const char *fmt, const struct tm *t);
 *
 * 返回写入的字节数（不含结尾 '\0'）；**返回 0 表示缓冲区不够或格式为空**，
 * 所以"格式化失败"和"格式化出空串"要用返回值区分，别只看 buf。
 *
 * 常用转换符：
 *      %Y 四位年    %m 月(01–12)   %d 日(01–31)
 *      %H 时(00–23) %M 分          %S 秒
 *      %F 等价 %Y-%m-%d     %T 等价 %H:%M:%S        （这两个是 C99 新增）
 *      %A 星期全名  %B 月份全名    %j 年内第几天(001–366)
 *      %% 百分号本身
 *
 * ★ %A / %B / %c 受 **locale** 影响：不调 setlocale() 时 C locale 只有英文，
 *   而且 setlocale() 结果依赖系统装了哪些语言包 —— 别指望它输出中文。
 *
 * ====================== 五、mktime()：结构体 → 时间戳 ======================
 *      time_t mktime(struct tm *t);
 *
 * 三件事一次做完：
 *      1) 按本地时区把 tm 转成 time_t
 *      2) **顺手把 tm_wday 和 tm_yday 算好写回**
 *      3) **自动规范化越界字段**（这是最好用的隐藏技能）
 *
 *      struct tm t = {0};
 *      t.tm_year = 2024 - 1900;  t.tm_mon = 2 - 1;  t.tm_mday = 30;
 *      mktime(&t);                 // 2 月 30 日不存在 → 自动变成 3 月 1 日
 *
 *      t.tm_mday = 0;              // 某月 0 日 = 上个月最后一天
 *      mktime(&t);                 // 2024-03-00 → 2024-02-29（闰年！）
 *
 *      t.tm_mon = 12;              // 月份 12（0 起点，即"第 13 个月"）→ 次年 1 月
 *
 * ★ 因为会规范化，**mktime() 会修改你传进去的结构体**：
 *   不想被改就传副本；想拿规范化结果就传原件（本篇第八节正是利用这一点）。
 * ★ 调用前把 tm_isdst 设为 -1，让库自己判断夏令时；填 0 而不准会导致差一小时。
 *
 * ====================== 六、difftime()：时间戳求差 ======================
 *      double difftime(time_t end, time_t begin);      // 返回秒数，注意参数顺序
 *
 * ★ 直接写 t2 - t1 在多数平台也能跑，但 time_t 不保证是无符号整数，
 *   标准要求用 difftime()，跨平台才安全。
 *
 * ====================== 七~九、标准库不管的部分，自己写 ======================
 * 闰年规则（格里高利历）：
 *      能被 4 整除，但不能被 100 整除；或者能被 400 整除。
 *      → 2000 是闰年，1900 不是，2100 不是。写成一行：
 *      (y % 4 == 0 && y % 100 != 0) || y % 400 == 0
 *
 * 当月天数：查表 + 二月单独判断。
 * 年内第几天：累加前面各月天数 + 当日。
 * 日期加减：tm_mday += n 后交给 mktime() 进位，月末、年末、闰年全部自动处理。
 * 星期几：填好年月日后让 mktime() 回填 tm_wday，比自己推蔡勒公式稳。
 *
 * ★ 边界一定要挡：month 不在 1–12、day 不在 1–当月天数时，
 *   自己写的函数要么返回 0，要么明确说明"依赖调用方保证合法"。
 *
 * ====================== 十、clock()：量代码耗时 ======================
 *      clock_t clock(void);        // 返回进程消耗的 CPU 时间
 *      秒数 = (double)(end - start) / CLOCKS_PER_SEC
 *
 * ★ clock() 量的是 **CPU 时间**，不是墙钟时间：程序 sleep 3 秒，clock() 几乎不动。
 *   要量真实流逝时间，用 time() 或 timespec_get()（C11）。
 *   CLOCKS_PER_SEC 是每秒的时钟数（POSIX 平台通常是 1000000）。
 *
 * ====================== 十一、坑与 2038 问题 ======================
 *  1. tm_year 忘了加 1900，tm_mon 忘了加 1          → 输出 126 年 0 月
 *  2. 连续调用 localtime()，前一个指针被覆盖        → 两个日期变得一样
 *  3. 不判断 time()/localtime() 的失败返回值        → 空指针解引用
 *  4. mktime() 返回 (time_t)-1 不检查               → 得到 1970 年附近的怪日期
 *  5. 传 struct tm 时不初始化，垃圾值把日期算飞     → 用 {0} 或 memset
 *  6. 忘了设 tm_isdst = -1                          → 夏令时地区差一小时
 *  7. tm_mday += n 之后不调 mktime() 就输出         → 出现 "2 月 32 日"
 *  8. 把 time_t 当 int 打印                         → 64 位下被截断
 *  9. 用 t2 - t1 而不是 difftime()                  → 非便携
 * 10. 用日期秒数差 ÷86400 算天数，遇夏令时少/多一天 → 用 mktime 差值 + 四舍五入
 * 11. **2038 问题**：32 位 time_t 最大只到 2038-01-19 03:14:07 UTC，
 *     之后溢出成负数。今天写新代码就假定 time_t 是 64 位，别做 32 位兼容假设。
 */

#include <stdio.h>
#include <string.h>     /* memset */
#include <time.h>       /* time / localtime / mktime / strftime / clock */

/* ====================== 自己实现的日期函数 ====================== */

/*
 * 判断闰年。
 * 入参：year 公历年（四位，如 2024）
 * 返回：1 是闰年，0 不是
 */
static int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
 * 返回某年某月的天数。
 * 入参：month 取 1–12（人类习惯，**不是** tm_mon 的 0 起点）
 * 返回：28/29/30/31；月份非法时返回 0，由调用方判断
 */
static int days_in_month(int year, int month)
{
    static const int table[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12)
        return 0;                       /* 边界保护：非法月份 */
    if (month == 2 && is_leap_year(year))
        return 29;
    return table[month - 1];
}

/*
 * 返回某日期是当年的第几天（1 月 1 日 = 1）。
 * 入参：month 1–12，day 1–31；非法输入返回 0
 */
static int day_of_year(int year, int month, int day)
{
    int sum = 0;

    if (days_in_month(year, month) == 0 || day < 1 || day > days_in_month(year, month))
        return 0;

    for (int m = 1; m < month; m++)
        sum += days_in_month(year, m);

    return sum + day;
}

/* 星期的中文名，tm_wday：0 = 周日 */
static const char *weekday_name(int wday)
{
    static const char *names[7] = {
        "周日", "周一", "周二", "周三", "周四", "周五", "周六"
    };

    if (wday < 0 || wday > 6)
        return "??";
    return names[wday];
}

/*
 * 用年月日"干净地"构造一个 struct tm。
 * 关键点：先清零（避免垃圾值），tm_isdst 设 -1 让 mktime 自己判断。
 */
static struct tm make_tm(int year, int month, int day)
{
    struct tm t;

    memset(&t, 0, sizeof t);
    t.tm_year = year - 1900;        /* 内部存的是"距 1900 年" */
    t.tm_mon  = month - 1;          /* 内部从 0 开始 */
    t.tm_mday = day;
    t.tm_isdst = -1;                /* -1：未知，交给库判断 */

    return t;
}

/* 打印 "标签  年-月-日 时:分:秒  星期" */
static void print_tm(const char *label, const struct tm *t)
{
    char buf[64];

    if (strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", t) == 0) {
        printf("   %s  <strftime 失败：缓冲区不足>\n", label);
        return;
    }
    printf("   %s  %s  %s\n", label, buf, weekday_name(t->tm_wday));
}

/*
 * 日期加 n 天（n 可为负）。
 * 技巧：直接把 n 加进 tm_mday，再让 mktime() 规范化 ——
 * 月末、年末、闰年、"负数回退到上个月"全部由库处理，不用自己写进位。
 */
static void add_days(struct tm *t, int n)
{
    t->tm_mday += n;
    t->tm_isdst = -1;
    (void)mktime(t);                /* 返回值用不上，只要它把 *t 规范化 */
}

/*
 * 求两个日期相差多少天（b - a）。
 * 用 mktime 得到两个时间戳再 difftime，最后四舍五入到"天"，
 * 这样即使跨夏令时（某天只有 23 小时）也不会算错。
 */
static long days_between(struct tm a, struct tm b)
{
    time_t ta = mktime(&a);
    time_t tb = mktime(&b);
    double days;

    if (ta == (time_t)-1 || tb == (time_t)-1)
        return 0;                    /* 转换失败，返回 0 由调用方判断 */

    days = difftime(tb, ta) / 86400.0;
    return (long)(days < 0 ? days - 0.5 : days + 0.5);
}

int main(void)
{
    /* ---------- 1. 三种"时间"的尺寸 ---------- */
    printf("1. 三种时间类型\n");
    printf("   sizeof(time_t)  = %zu  （秒数，宽度由平台定）\n", sizeof(time_t));
    printf("   sizeof(struct tm) = %zu\n", sizeof(struct tm));
    printf("   sizeof(clock_t) = %zu，CLOCKS_PER_SEC = %ld\n",
           sizeof(clock_t), (long)CLOCKS_PER_SEC);

    /* ---------- 2. time() + localtime() ---------- */
    printf("2. time() 取时间戳，localtime() 转本地时间\n");

    time_t now = time(NULL);
    if (now == (time_t)-1) {                 /* 必须判：-1 表示失败 */
        printf("   time() 调用失败\n");
        return 1;
    }
    printf("   time(NULL) = %ld（从 1970-01-01 UTC 起的秒数）\n", (long)now);

    struct tm *p = localtime(&now);
    if (p == NULL) {
        printf("   localtime() 调用失败\n");
        return 1;
    }
    print_tm("localtime(now):", p);

    struct tm *g = gmtime(&now);
    if (g != NULL)
        print_tm("gmtime(now)   :", g);

    /* 静态缓冲区陷阱：两次 localtime 拿到的是同一块内存 */
    time_t yesterday = now - 86400;          /* 86400 秒 = 1 天（忽略夏令时） */

    struct tm *pa = localtime(&now);
    struct tm snap_today = *pa;              /* ★ 立刻值拷贝 */
    struct tm *pb = localtime(&yesterday);
    struct tm snap_last = *pb;

    printf("   两次返回的指针相同吗？%s\n", pa == pb ? "相同（同一块静态内存）" : "不同");
    print_tm("拷贝后的今天:", &snap_today);
    print_tm("拷贝后的昨天:", &snap_last);
    printf("   若直接用 pa->tm_mday / pb->tm_mday，两个值会一模一样\n");

    /* ---------- 3. struct tm 字段的起点 ---------- */
    printf("3. struct tm 九个字段（注意起点）\n");
    printf("   tm_year = %d  → 实际年份 %d（差 1900）\n",
           snap_today.tm_year, snap_today.tm_year + 1900);
    printf("   tm_mon  = %d  → 实际月份 %d（差 1）\n",
           snap_today.tm_mon, snap_today.tm_mon + 1);
    printf("   tm_mday = %d（唯一从 1 开始的日期字段）\n", snap_today.tm_mday);
    printf("   tm_wday = %d（0 = 周日），tm_yday = %d（0 = 1 月 1 日）\n",
           snap_today.tm_wday, snap_today.tm_yday);
    printf("   tm_hour:min:sec = %02d:%02d:%02d\n",
           snap_today.tm_hour, snap_today.tm_min, snap_today.tm_sec);

    /* ---------- 4. strftime() 格式化 ---------- */
    printf("4. strftime() 结构体 → 字符串\n");
    {
        char buf[64];

        strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &snap_today);
        printf("   %%Y-%%m-%%d %%H:%%M:%%S → %s\n", buf);
        strftime(buf, sizeof buf, "%F %T", &snap_today);       /* C99 简写 */
        printf("   %%F %%T（C99 简写）      → %s\n", buf);
        strftime(buf, sizeof buf, "%j/%Y %A", &snap_today);
        printf("   %%j/%%Y %%A              → %s\n", buf);

        /* 返回值 0 表示"写不下或格式为空"，要判 */
        if (strftime(buf, 4, "%Y-%m-%d", &snap_today) == 0)
            printf("   缓冲区只有 4 字节 → strftime 返回 0，内容不可用\n");
    }

    /* ---------- 5. mktime()：反向转换 + 自动规范化 ---------- */
    printf("5. mktime() 结构体 → 时间戳，并自动规范化越界字段\n");
    {
        struct tm t = make_tm(2024, 3, 1);
        t.tm_mday = 0;                       /* 3 月 0 日 = 2 月最后一天 */
        (void)mktime(&t);
        print_tm("2024-03-00 →", &t);

        struct tm t2 = make_tm(2023, 12, 32); /* 12 月 32 日 */
        (void)mktime(&t2);
        print_tm("2023-12-32 →", &t2);

        struct tm t3 = make_tm(2024, 2, 30);  /* 2 月 30 日不存在 */
        (void)mktime(&t3);
        print_tm("2024-02-30 →", &t3);

        time_t stamp = mktime(&t3);
        printf("   顺便回填：tm_wday = %d, tm_yday = %d\n", t3.tm_wday, t3.tm_yday);
        printf("   对应时间戳 = %ld（失败才返回 (time_t)-1）\n", (long)stamp);
    }

    /* ---------- 6. difftime() ---------- */
    printf("6. difftime() 求两个时间戳的差\n");
    {
        struct tm a = make_tm(2024, 1, 1);
        struct tm b = make_tm(2025, 1, 1);
        time_t ta = mktime(&a);
        time_t tb = mktime(&b);

        if (ta != (time_t)-1 && tb != (time_t)-1)
            printf("   2024-01-01 → 2025-01-01 相差 %.0f 秒（366 天的闰年）\n",
                   difftime(tb, ta));
    }

    /* ---------- 7. 自己写的日期函数 ---------- */
    printf("7. 自己写：闰年 / 当月天数 / 年内第几天\n");
    {
        int years[] = { 1900, 2000, 2023, 2024, 2100 };

        for (size_t i = 0; i < sizeof years / sizeof years[0]; i++)
            printf("   %d 年：%s（2 月 %d 天）\n",
                   years[i],
                   is_leap_year(years[i]) ? "闰年" : "平年",
                   days_in_month(years[i], 2));

        printf("   2024 年各月天数：");
        for (int m = 1; m <= 12; m++)
            printf("%d ", days_in_month(2024, m));
        printf("\n");

        printf("   非法月份 days_in_month(2024, 13) = %d（返回 0 表示出错）\n",
               days_in_month(2024, 13));
        printf("   2024-03-01 是年内第 %d 天，2024-12-31 是第 %d 天\n",
               day_of_year(2024, 3, 1), day_of_year(2024, 12, 31));
    }

    /* ---------- 8. 日期加减 ---------- */
    printf("8. 日期加减：tm_mday += n 后交给 mktime() 进位\n");
    {
        struct tm t = make_tm(2024, 2, 28);

        for (int i = 0; i <= 3; i++) {
            struct tm cur = t;
            add_days(&cur, i);
            printf("   2024-02-28 加 %d 天 → %04d-%02d-%02d\n",
                   i, cur.tm_year + 1900, cur.tm_mon + 1, cur.tm_mday);
        }

        struct tm back = make_tm(2024, 1, 1);
        add_days(&back, -1);                 /* 减一天：跨年到上一年 */
        print_tm("2024-01-01 减 1 天 →", &back);

        printf("   2024-01-01 到 2024-03-01 相差 %ld 天\n",
               days_between(make_tm(2024, 1, 1), make_tm(2024, 3, 1)));
    }

    /* ---------- 9. 星期几 ---------- */
    printf("9. 星期几：让 mktime() 回填 tm_wday\n");
    {
        struct tm known = make_tm(2000, 1, 1);   /* 已知 2000-01-01 是周六 */
        (void)mktime(&known);
        printf("   2000-01-01 → %s（可用来验证算法）\n", weekday_name(known.tm_wday));

        struct tm today = make_tm(snap_today.tm_year + 1900,   /* 转回公历年 */
                                  snap_today.tm_mon + 1,       /* 转回 1–12 */
                                  snap_today.tm_mday);
        (void)mktime(&today);
        printf("   今天 → %s（tm_wday = %d）\n",
               weekday_name(today.tm_wday), today.tm_wday);
    }

    /* ---------- 10. clock() 计时 ---------- */
    printf("10. clock() 量 CPU 耗时\n");
    {
        clock_t start = clock();
        volatile double sink = 0.0;

        for (long i = 0; i < 3000000L; i++)
            sink += (double)i;

        clock_t stop = clock();
        printf("    300 万次累加耗时 %.3f ms（结果 %.0f，防止被优化掉）\n",
               (double)(stop - start) * 1000.0 / (double)CLOCKS_PER_SEC,
               (double)sink);
        printf("    clock() 量的是 CPU 时间，sleep 期间几乎不动；\n");
        printf("    量墙钟时间用 time() 或 timespec_get()（C11）\n");
    }

    /* ---------- 11. 小结 ---------- */
    printf("11. 小结：\n");
    printf("     time() 拿时间戳，失败返回 (time_t)-1，必须判\n");
    printf("     localtime()/gmtime() 返回静态对象，连调两次会互相覆盖 → 立刻拷贝\n");
    printf("     tm_year 差 1900，tm_mon 差 1，tm_wday/tm_yday 从 0 开始\n");
    printf("     strftime() 负责格式化，返回 0 表示写不下\n");
    printf("     mktime() 会规范化越界字段，是日期加减最省事的工具\n");
    printf("     difftime() 求差，跨夏令时算天数要四舍五入\n");
    printf("     闰年、当月天数得自己写：(y%%4==0 && y%%100!=0) || y%%400==0\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 带"当前时间"的几行每次运行都会变，其余部分固定。
 *
 * 编译运行：
 *     mkdir -p bin
 *     gcc -std=c99 -Wall -Wextra 01_日期与时间.c -o bin/01_日期与时间
 *     ./bin/01_日期与时间
 *
 * 输出如下（时间相关行以实际运行为准）：
 *
 * 1. 三种时间类型
 *    sizeof(time_t)  = 8  （秒数，宽度由平台定）
 *    sizeof(struct tm) = 56
 *    sizeof(clock_t) = 8，CLOCKS_PER_SEC = 1000000
 * 2. time() 取时间戳，localtime() 转本地时间
 *    time(NULL) = 1793xxxxxx（从 1970-01-01 UTC 起的秒数）
 *    localtime(now):  2026-11-06 10:00:00  周五
 *    gmtime(now)   :  2026-11-06 02:00:00  周五
 *    两次返回的指针相同吗？相同（同一块静态内存）
 *    拷贝后的今天:  2026-11-06 10:00:00  周五
 *    拷贝后的昨天:  2026-11-05 10:00:00  周四
 *    若直接用 pa->tm_mday / pb->tm_mday，两个值会一模一样
 * 3. struct tm 九个字段（注意起点）
 *    tm_year = 126  → 实际年份 2026（差 1900）
 *    tm_mon  = 10  → 实际月份 11（差 1）
 *    tm_mday = 6（唯一从 1 开始的日期字段）
 *    tm_wday = 5（0 = 周日），tm_yday = 309（0 = 1 月 1 日）
 *    tm_hour:min:sec = 10:00:00
 * 4. strftime() 结构体 → 字符串
 *    %Y-%m-%d %H:%M:%S → 2026-11-06 10:00:00
 *    %F %T（C99 简写）      → 2026-11-06 10:00:00
 *    %j/%Y %A              → 310/2026 Friday
 *    缓冲区只有 4 字节 → strftime 返回 0，内容不可用
 * 5. mktime() 结构体 → 时间戳，并自动规范化越界字段
 *    2024-03-00 →  2024-02-29 00:00:00  周四
 *    2023-12-32 →  2024-01-01 00:00:00  周一
 *    2024-02-30 →  2024-03-01 00:00:00  周五
 *    顺便回填：tm_wday = 5, tm_yday = 60
 *    对应时间戳 = 1709222400（具体值随本地时区变化；失败才返回 (time_t)-1）
 * 6. difftime() 求两个时间戳的差
 *    2024-01-01 → 2025-01-01 相差 31622400 秒（366 天的闰年）
 * 7. 自己写：闰年 / 当月天数 / 年内第几天
 *    1900 年：平年（2 月 28 天）
 *    2000 年：闰年（2 月 29 天）
 *    2023 年：平年（2 月 28 天）
 *    2024 年：闰年（2 月 29 天）
 *    2100 年：平年（2 月 28 天）
 *    2024 年各月天数：31 29 31 30 31 30 31 31 30 31 30 31
 *    非法月份 days_in_month(2024, 13) = 0（返回 0 表示出错）
 *    2024-03-01 是年内第 61 天，2024-12-31 是第 366 天
 * 8. 日期加减：tm_mday += n 后交给 mktime() 进位
 *    2024-02-28 加 0 天 → 2024-02-28
 *    2024-02-28 加 1 天 → 2024-02-29
 *    2024-02-28 加 2 天 → 2024-03-01
 *    2024-02-28 加 3 天 → 2024-03-02
 *    2024-01-01 减 1 天 →  2023-12-31 00:00:00  周日
 *    2024-01-01 到 2024-03-01 相差 60 天
 * 9. 星期几：让 mktime() 回填 tm_wday
 *    2000-01-01 → 周六（可用来验证算法）
 *    今天 → 周五（tm_wday = 5）
 * 10. clock() 量 CPU 耗时
 *    300 万次累加耗时 20.xxx ms（结果 4499998500000，防止被优化掉）
 *    clock() 量的是 CPU 时间，sleep 期间几乎不动；
 *    量墙钟时间用 time() 或 timespec_get()（C11）
 * 11. 小结：
 *     time() 拿时间戳，失败返回 (time_t)-1，必须判
 *     localtime()/gmtime() 返回静态对象，连调两次会互相覆盖 → 立刻拷贝
 *     tm_year 差 1900，tm_mon 差 1，tm_wday/tm_yday 从 0 开始
 *     strftime() 负责格式化，返回 0 表示写不下
 *     mktime() 会规范化越界字段，是日期加减最省事的工具
 *     difftime() 求差，跨夏令时算天数要四舍五入
 *     闰年、当月天数得自己写：(y%4==0 && y%100!=0) || y%400==0
 * ==========================================================================
 */
