/*
 * 02_多分支_switch_case.c —— 阶段1 / 第 04 课 / 第 02 个文件：switch - case 多分支
 *
 * 编译：gcc 02_多分支_switch_case.c -o switch_demo
 * 运行：./switch_demo
 *
 * 本文件能干净编译（-Wall -Wextra 无警告）：
 * 第 2、7 节故意"贯穿（fall through）"的地方都加了 fall through 注释，
 * 编译器认这种注释，不会报 -Wimplicit-fallthrough。
 * 如果你把注释删掉，就会看到那条警告——那正是提醒你"这里真的不要 break 吗"。
 *
 * ====================== 一、switch 解决什么问题 ======================
 * 三个以上"等值判断"时，if-else if 链会变得又长又丑：
 *
 *      if (n == 1)      ...
 *      else if (n == 2) ...
 *      else if (n == 3) ...
 *      else             ...
 *
 * switch 就是为这种场景生的：把"比较同一个表达式 == 若干个常量"写成一张表，
 * 一眼就能看清有哪几种情况。
 *
 * ====================== 二、语法 ======================
 *
 *      switch (整型表达式) {
 *      case 常量1:
 *          语句...;
 *          break;                // 跳出 switch
 *      case 常量2:
 *          语句...;
 *          break;
 *      default:                  // 兜底：谁都不匹配时执行
 *          语句...;
 *          break;
 *      }
 *
 * 执行流程（★ 和 if 完全不同的地方）：
 *      1) 先算 switch 括号里的表达式的值
 *      2) 拿它和各个 case 的常量比，**找到相等的，跳过去**（像跳表，不是逐个 if）
 *      3) 从那一行开始**一路往下执行**，遇到 break 才跳出 switch
 *      4) 一个都不匹配 → 跳 default；没有 default → 整个 switch 什么都不做
 *
 * 记住这句：**switch 只负责"跳"，不负责"停"。停止是 break 的事。**
 *
 * ====================== 三、break 的作用 ======================
 * break 是"跳出当前 switch"的开关。少了它就会出现"贯穿（fall through）"：
 *
 *      switch (2) {
 *      case 1:  printf("A");
 *      case 2:  printf("B");      // 从这开始
 *      case 3:  printf("C");      // 没有 break，继续打印 C
 *      default: printf("D");      // 继续打印 D
 *      }
 *      输出：BCD        ← 不是只打印 B！
 *
 * 所以：**每个 case 结尾默认该有 break**，除非你故意要贯穿。
 *
 * ====================== 四、default 的规则 ======================
 *  - 可以没有：那就"什么都不做"
 *  - 位置任意：放最后最常见，放最前面也合法（但会先比较后面的 case）
 *  - 一般不需要 break（它后面没 case 了），但加上不吃亏，也方便以后加 case
 *  - 兜底是种习惯：输入校验、状态机的"未知状态"都该走 default 报错
 *
 * ====================== 五、case 后面能写什么（重要限制） ======================
 * case 标签必须是**整型常量表达式**，编译期就要能算出值：
 *
 *      ✓ 整数常量        case 1:   case -5:   case 0x10:
 *      ✓ 字符常量        case 'a':  case '\n':
 *      ✓ 枚举常量        case RED:
 *      ✓ 宏 / 常量表达式  case MAX_LEN:   case 1 + 2:
 *      ✗ 变量            case n:              // 编译不过
 *      ✗ const 变量      case max:            // C 里 const int 不是常量表达式
 *      ✗ 浮点数          case 1.5:            // switch 只支持整型
 *      ✗ 字符串          case "abc":
 *      ✗ 范围            case 1..5:           // GNU 扩展，别用
 *      ✗ 重复的值        case 1: ... case 1:  // 编译报错
 *
 * switch 的表达式类型只能是整型（char / short / int / long / 枚举），
 * 不能是 float / double / 指针 / 字符串。
 * 想用字符串做分支，就 if-else + strcmp，或者查表。
 *
 * ====================== 六、case 里定义变量要加花括号 ======================
 * C 的语法规定：标签后面必须跟**语句**，而声明不是语句，所以：
 *
 *      case 1:
 *          int t = 10;          // ✗ 编译报错（C23 之前）
 *          printf("%d", t);
 *          break;
 *
 * 正确写法是给这个 case 套一层作用域：
 *
 *      case 1: {
 *          int t = 10;
 *          printf("%d", t);
 *          break;
 *      }
 *
 * 顺带好处：变量名不会和别的 case 冲突，出了花括号就销毁。
 *
 * ====================== 七、什么时候"故意不写 break" ======================
 * 贯穿不全是 bug，有两个正当用途：
 *
 *  (1) 多个 case 共享同一段代码
 *          case 'a': case 'e': case 'i': case 'o': case 'u':
 *              printf("元音\n");
 *              break;
 *
 *  (2) 累积型的逻辑（例如按月份算天数、状态机逐级推进）
 *          case 1: days += 31;      // fall through（有意不写 break）
 *          case 2: days += 28;      // fall through
 *          ...
 *
 * 规则：只要不是这两种，就必须写 break；真需要贯穿时用注释标注，
 *       让读代码的人知道"这不是忘了"，也能顺便消掉编译器的警告。
 *
 * ====================== 八、switch 和 if-else 怎么选 ======================
 *      switch 擅长：同一个变量 / 表达式的**等值**多分支（菜单、状态、字符命令）
 *      if-else 擅长：区间判断（score >= 60）、复合条件（a > 0 && b < 0）、
 *                    判断的类型各不相同（指针判空、浮点比较）
 *
 * 经验值：
 *      2 个分支 → if-else 就够
 *      3 个以上等值分支 → switch 更清楚
 *      区间 / 范围 / 复杂条件 → 只能 if-else
 *
 * ====================== 九、常见坑 ======================
 *  1. 忘写 break             → 贯穿，执行了本不该执行的 case
 *  2. case 写成表达式        → case a+b: 编译不过
 *  3. case 用了变量 / const  → 同样编译不过，改用 if 或 #define
 *  4. 用 switch 判浮点       → 编译不过，浮点只能 if
 *  5. case 里定义变量不加 {} → 编译不过（见第六节）
 *  6. 以为 switch 里的 break 能跳出循环 → 只能跳出 switch
 *  7. 忘了 default           → 非法输入静默通过，很难查
 *  8. 在 switch 里用 continue → 在 C 里合法，但作用于外层循环，极易误读
 *  9. case 顺序随便写        → 不影响结果，但按顺序排列更易读
 * 10. 贯穿时不加注释         → 后人以为你忘了 break
 *
 * ====================== 十、实践建议 ======================
 *  - 一个 case 一件事，代码块超过 10 行就抽成函数
 *  - break 与 case 对齐写，缩进统一（本书风格：case 与 switch 齐平）
 *  - default 一律写上，哪怕只写一句"未知输入"
 *  - 故意贯穿处写 fall through 注释标注（编译器也认这种写法，不会报警）
 *  - 需要返回值时每个 case 直接 return，可以省掉 break（见 grade 示例）
 *  - 菜单 / 命令解析这类"字符串 → 动作"的需求，先查表再 switch
 */

#include <stdio.h>

/* 宏常量可以作 case 标签 */
#define CMD_ADD   1
#define CMD_DEL   2
#define CMD_LIST  3

/* 枚举常量也可以作 case 标签 */
enum color { RED, GREEN, BLUE };

/* 用 return 代替 break：每个 case 直接返回，最不容易漏 */
static const char *day_name(int d)
{
    switch (d) {
    case 1: return "周一";
    case 2: return "周二";
    case 3: return "周三";
    case 4: return "周四";
    case 5: return "周五";
    case 6: return "周六";
    case 7: return "周日";
    default: return "不是星期的编号";
    }
}

/* 故意贯穿的正当用法：多个月份共享同一个结果 */
static int days_in_month(int m)
{
    switch (m) {
    case 2:
        return 28;                      /* 简化：不考虑闰年 */
    case 4: case 6: case 9: case 11:    /* 小月：4 个 case 共享 */
        return 30;
    default:
        return 31;
    }
}

/* 典型的 switch 用途：简易计算器 */
static void calc(int a, char op, int b)
{
    switch (op) {
    case '+':
        printf("   %d + %d = %d\n", a, b, a + b);
        break;
    case '-':
        printf("   %d - %d = %d\n", a, b, a - b);
        break;
    case '*':
        printf("   %d * %d = %d\n", a, b, a * b);
        break;
    case '/':
        if (b == 0)
            printf("   %d / %d → 除数不能为 0\n", a, b);
        else
            printf("   %d / %d = %d\n", a, b, a / b);
        break;
    default:                            /* ★ 兜底，防止非法运算符 */
        printf("   不支持的运算符 '%c'\n", op);
        break;
    }
}

int main(void)
{
    /* ---------- 1. 基本用法：等值多分支 ---------- */
    printf("1. switch 基本用法：按编号取名字\n");
    for (int d = 1; d <= 8; d++)
        printf("   %d → %s\n", d, day_name(d));
    printf("   8 走到 default，说明兜底分支生效\n");

    /* ---------- 2. 执行流程：只跳不停 ---------- */
    int hit = 2;
    printf("2. 只负责跳，不负责停：switch (2) 且都没有 break\n");
    switch (hit) {
    case 1:
        printf("   case 1\n");
    case 2:
        printf("   case 2  ← 从这里开始执行\n");
        /* fall through —— 故意不写 break，演示贯穿 */
    case 3:
        printf("   case 3  （被贯穿带下来了）\n");
        /* fall through */
    default:
        printf("   default （也被带下来了）\n");
        break;
    }
    printf("   结论：命中位置之后的 case 全都会执行，直到遇到 break\n");

    /* ---------- 3. 正确写法：每个 case 配 break ---------- */
    printf("3. 正确写法：case 后面配 break，命中后就跳出\n");
    for (int h = 1; h <= 3; h++) {
        printf("   switch (%d) → ", h);
        switch (h) {
        case 1: printf("走了 case 1\n"); break;
        case 2: printf("走了 case 2\n"); break;
        case 3: printf("走了 case 3\n"); break;
        default: printf("走了 default\n"); break;
        }
    }
    printf("   每次只打印一行，这就是 break 的作用\n");

    /* ---------- 4. default：兜底 + 位置任意 ---------- */
    int code = 99;
    printf("4. default 兜底：谁都不匹配时才执行\n");
    switch (code) {
    case CMD_ADD:  printf("   新增\n");  break;
    case CMD_DEL:  printf("   删除\n");  break;
    case CMD_LIST: printf("   列表\n");  break;
    default:
        printf("   code=%d 是非法命令，走 default 报错\n", code);
        break;
    }
    printf("   default 也可以写在最前面，但建议放最后，读起来顺\n");

    /* ---------- 5. case 支持字符和枚举 ---------- */
    printf("5. case 支持字符常量和枚举常量\n");
    char cmd = 'q';
    switch (cmd) {
    case 'y': case 'Y':                 /* 两个 case 共享同一段代码 */
        printf("   输入 %c → 确认\n", cmd);
        break;
    case 'n': case 'N':
        printf("   输入 %c → 取消\n", cmd);
        break;
    case 'q':
        printf("   输入 %c → 退出\n", cmd);
        break;
    default:
        printf("   输入 %c → 无法识别\n", cmd);
        break;
    }

    enum color c = BLUE;
    switch (c) {
    case RED:   printf("   枚举 RED → 红\n");   break;
    case GREEN: printf("   枚举 GREEN → 绿\n"); break;
    case BLUE:  printf("   枚举 BLUE → 蓝\n");  break;
    }
    printf("   枚举比魔法数字 0/1/2 好读，switch 里尤其明显\n");

    /* ---------- 6. 不用 break 的写法：每支直接 return ---------- */
    printf("6. 函数里用 return 代替 break，能彻底避免忘记 break\n");
    printf("   day_name(3) = %s，这里面一个 break 都没有\n", day_name(3));
    printf("   因为 return 已经直接离开函数了\n");

    /* ---------- 7. 故意贯穿：多 case 共享 / 累积计算 ---------- */
    printf("7. 故意贯穿的两个正当用途\n");
    printf("   (1) 共享代码：'a'、'e'、'i'、'o'、'u' 五个 case 一起处理\n");
    for (const char *p = "hello"; *p; p++) {
        switch (*p) {
        case 'a': case 'e': case 'i': case 'o': case 'u':
            printf("       %c 是元音\n", *p);
            break;
        default:
            printf("       %c 是辅音\n", *p);
            break;
        }
    }

    printf("   (2) 累积型：从年初算起的天数，逐月累加\n");
    int mth = 3;                        /* 1~3 月，想看累计天数 */
    int days = 0;
    switch (mth) {
    case 3: days += 31;                 /* fall through */
    case 2: days += 28;                 /* fall through */
    case 1: days += 31;                 /* fall through */
    default:
        printf("       按倒序累加，前 %d 个月共 %d 天\n", mth, days);
        break;
    }
    printf("   ★ 这种倒序累加是贯穿的唯一“高级”用法，写注释说明白\n");

    /* ---------- 8. case 里定义变量要加花括号 ---------- */
    int sel = 1;
    printf("8. case 里定义变量必须套一层花括号（C 的语法限制）\n");
    switch (sel) {
    case 1: {
        int tmp = 10;                   /* 有 {} 才能这样写 */
        printf("   case 1 内部定义 tmp=%d，出了花括号就销毁\n", tmp);
        break;
    }
    case 2:
        printf("   case 2\n");
        break;
    default:
        break;
    }
    printf("   不加 {} 写成 case 1: int tmp = 10; 会编译报错（C23 之前）\n");

    /* ---------- 9. switch 里的 break 不跳出循环 ---------- */
    printf("9. switch 里的 break 只能跳出 switch，不影响外层循环\n");
    for (int i = 1; i <= 3; i++) {
        switch (i) {
        case 2:
            printf("   i=%d 命中 case 2，break 后循环继续\n", i);
            break;
        default:
            printf("   i=%d 走 default\n", i);
            break;
        }
    }
    printf("   要跳出循环得用 goto 标志位或在循环里再判断一次\n");

    /* ---------- 10. 实际用法：简易计算器 ---------- */
    printf("10. 实际用法：用 switch 实现四则运算\n");
    calc(6, '+', 3);
    calc(6, '-', 3);
    calc(6, '*', 3);
    calc(6, '/', 3);
    calc(6, '/', 0);
    calc(6, '%', 3);
    printf("   非法运算符走 default 报错，不会静默出错\n");

    /* ---------- 11. 月度天数：case 共享 + default ---------- */
    printf("11. 多 case 共享：4/6/9/11 月都是 30 天\n");
    for (int mm = 1; mm <= 12; mm++)
        printf("   %2d 月：%d 天\n", mm, days_in_month(mm));

    /* ---------- 12. 小结 ---------- */
    printf("12. 小结：\n");
    printf("    switch (整型表达式) { case 常量: 语句; break; default: ... }\n");
    printf("    只负责跳，不负责停：不写 break 会贯穿到下面的 case\n");
    printf("    case 必须是整型常量表达式，不能用变量/浮点/字符串\n");
    printf("    等值多分支用 switch，区间和复杂条件用 if-else\n");
    printf("    default 一律写上，兜住未知输入\n");
    printf("    case 里定义变量要加 { }；函数里可用 return 省掉 break\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     gcc 02_多分支_switch_case.c -o switch_demo
 *     ./switch_demo
 *
 * 输出如下：
 *
 * 1. switch 基本用法：按编号取名字
 *    1 → 周一
 *    2 → 周二
 *    3 → 周三
 *    4 → 周四
 *    5 → 周五
 *    6 → 周六
 *    7 → 周日
 *    8 → 不是星期的编号
 *    8 走到 default，说明兜底分支生效
 * 2. 只负责跳，不负责停：switch (2) 且都没有 break
 *    case 2  ← 从这里开始执行
 *    case 3  （被贯穿带下来了）
 *    default （也被带下来了）
 *    结论：命中位置之后的 case 全都会执行，直到遇到 break
 * 3. 正确写法：case 后面配 break，命中后就跳出
 *    switch (1) → 走了 case 1
 *    switch (2) → 走了 case 2
 *    switch (3) → 走了 case 3
 *    每次只打印一行，这就是 break 的作用
 * 4. default 兜底：谁都不匹配时才执行
 *    code=99 是非法命令，走 default 报错
 *    default 也可以写在最前面，但建议放最后，读起来顺
 * 5. case 支持字符常量和枚举常量
 *    输入 q → 退出
 *    枚举 BLUE → 蓝
 *    枚举比魔法数字 0/1/2 好读，switch 里尤其明显
 * 6. 函数里用 return 代替 break，能彻底避免忘记 break
 *    day_name(3) = 周三，这里面一个 break 都没有
 *    因为 return 已经直接离开函数了
 * 7. 故意贯穿的两个正当用途
 *    (1) 共享代码：'a'、'e'、'i'、'o'、'u' 五个 case 一起处理
 *        h 是辅音
 *        e 是元音
 *        l 是辅音
 *        l 是辅音
 *        o 是元音
 *    (2) 累积型：从年初算起的天数，逐月累加
 *        按倒序累加，前 3 个月共 90 天
 *    ★ 这种倒序累加是贯穿的唯一“高级”用法，写注释说明白
 * 8. case 里定义变量必须套一层花括号（C 的语法限制）
 *    case 1 内部定义 tmp=10，出了花括号就销毁
 *    不加 {} 写成 case 1: int tmp = 10; 会编译报错（C23 之前）
 * 9. switch 里的 break 只能跳出 switch，不影响外层循环
 *    i=1 走 default
 *    i=2 命中 case 2，break 后循环继续
 *    i=3 走 default
 *    要跳出循环得用 goto 标志位或在循环里再判断一次
 * 10. 实际用法：用 switch 实现四则运算
 *    6 + 3 = 9
 *    6 - 3 = 3
 *    6 * 3 = 18
 *    6 / 3 = 2
 *    6 / 0 → 除数不能为 0
 *    不支持的运算符 '%'
 *    非法运算符走 default 报错，不会静默出错
 * 11. 多 case 共享：4/6/9/11 月都是 30 天
 *     1 月：31 天
 *     2 月：28 天
 *     3 月：31 天
 *     4 月：30 天
 *     5 月：31 天
 *     6 月：30 天
 *     7 月：31 天
 *     8 月：31 天
 *     9 月：30 天
 *    10 月：31 天
 *    11 月：30 天
 *    12 月：31 天
 * 12. 小结：
 *     switch (整型表达式) { case 常量: 语句; break; default: ... }
 *     只负责跳，不负责停：不写 break 会贯穿到下面的 case
 *     case 必须是整型常量表达式，不能用变量/浮点/字符串
 *     等值多分支用 switch，区间和复杂条件用 if-else
 *     default 一律写上，兜住未知输入
 *     case 里定义变量要加 { }；函数里可用 return 省掉 break
 *
 * 几个容易忽略的对照点：
 *   第 1 条：输入 8 时所有 case 都不匹配，才走到 default —— 这就是兜底。
 *   第 2 条：switch (2) 只打印了 case 2 之后的内容，case 1 根本没执行，
 *           证明 switch 是"跳过去"，而不是从头逐个比较。
 *   第 7 条 (2)：mth=3 时 case 3 → case 2 → case 1 依次累加 31+28+31 = 90。
 *            如果按 1→2→3 正序写，就只会加上 31，贯穿方向错了结果就错。
 *   第 9 条：break 在 switch 里只跳 switch，循环照常跑满 3 次。
 *
 * 为什么 2 月是 28 天：示例为了简单没处理闰年，
 * 想加闰年判断，在 case 2 里用 if 判断 year 是否为闰年即可
 * （switch 里当然可以嵌 if，只是分支别太深）。
 * ==========================================================================
 */
