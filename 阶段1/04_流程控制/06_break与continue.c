/*
 * 06_break与continue.c —— 阶段1 / 第 04 课 / 第 06 个文件：break、continue 与跳出多层循环
 *
 * 编译：gcc 06_break与continue.c -o bin/06_break与continue
 * 运行：./bin/06_break与continue
 *
 * 本文件能干净编译（-Wall -Wextra 零警告）。
 * 第 10 节讲"while 里 continue 会死循环"，错误写法只放在注释里，不会真的跑。
 *
 * ====================== 一、两个关键字只管"最内层" ======================
 *
 *      break     立刻结束**当前这一层**循环（或 switch），跳到它后面继续执行
 *      continue  跳过本轮剩下的语句，直接进入**下一轮**
 *
 * ★ 最重要的一句话：**它们都只作用于最内层的那个循环或 switch。**
 *   写在两层循环的内层里，break 只跳出内层，外层照常跑 —— 这是本章最大的坑。
 *
 * 跳到哪儿去（这个细节决定会不会死循环）：
 *
 *      循环        continue 跳到这里
 *      ---------   ------------------------
 *      for         更新语句（i++），所以安全
 *      while       条件判断处，更新在体内 → 可能被跳过 → 死循环
 *      do-while    条件判断处，同上
 *      switch      continue 不影响 switch，作用于外层循环
 *
 * ====================== 二、break 的三种正当用法 ======================
 *
 *  (1) 查找命中即停（最常见）
 *          for (int i = 0; i < n; i++)
 *              if (a[i] == target) { pos = i; break; }
 *
 *  (2) 提前结束：已经能确定结果，不用再算
 *          for (...) {
 *              if (sum > limit) { too_big = 1; break; }
 *              sum += ...;
 *          }
 *
 *  (3) switch 里结束分支（02 篇）
 *          case 1: ...; break;      ← 这里的 break 属 switch，不属于循环
 *
 * 反例：把 break 当"正常出口"，让循环条件形同虚设。
 *       for (i = 0; i < n; i++) 里靠 break 退出，i 的值就不好读了。
 *
 * ====================== 三、continue 的正当用法 ======================
 *
 *  (1) 过滤脏数据（守卫风格，主逻辑不缩进）
 *          for (i = 0; i < n; i++) {
 *              if (a[i] < 0) continue;
 *              total += a[i];              // 这里只剩合法数据
 *          }
 *
 *  (2) 跳过本轮的某个特殊情况
 *          if (i == skip) continue;
 *
 * 什么时候别用：把 continue 写在一堆 if 中间、或者一个循环里出现三四次，
 * 建议改成 if/else 的完整分支，读起来更线性。
 *
 * ====================== 四、★★ 两层循环怎么一起退出（本章重点） ======================
 * 需求：内层找到目标后，外层也别再跑了。共有四种方案。
 *
 *  方案 A：标志位 flag（最通用，任何场景都能用）
 *
 *      int found = 0;
 *      for (int i = 0; i < n && !found; i++) {      // ① 外层条件带上 flag
 *          for (int j = 0; j < m; j++) {
 *              if (hit(i, j)) { found = 1; break; } // ② 内层 break
 *          }                                        //    （也可以外层再判一次 break）
 *      }
 *
 *      ★ 关键：光在内层 break 是不够的，必须让外层知道"该停了"。
 *        常见写法是在内层循环后面补一句 if (found) break;
 *
 *  方案 B：goto（C 里最直接的多层跳出）
 *
 *      for (...) for (...) if (hit) goto done;
 *      done:
 *      // 继续后面的代码
 *
 *      goto 在 C 里不算洪水猛兽，**只往前跳、只用于跳出/清理**是被广泛接受的写法
 *      （Linux 内核里就大量使用）。禁忌只有一个：不要用 goto 跳进循环体内部。
 *      ★ goto 的完整规则（标签作用域、VLA 限制、清理范式、C/C++ 差异）
 *        见 07_goto.c，这里只当作"多层跳出的一种手段"。
 *
 *  方案 C：抽成函数 + return（可读性最好，推荐优先考虑）
 *
 *      int find_pair(void) {
 *          for (...) for (...) if (hit) return 1;   // return 一步结束整个函数
 *          return 0;
 *      }
 *
 *  方案 D：把退出条件写进外层循环条件（不用 break）
 *
 *      for (int i = 0; i < n && !found; i++)
 *          for (int j = 0; j < m && !found; j++)
 *              if (hit(i, j)) found = 1;
 *
 *      缺点：内层不会"立刻"停（还要把本轮剩下的 j 跑完），
 *            但对 n×m 不大的场景，代码最平、最不容易出错。
 *
 *  四者对比：
 *
 *      方案      可读性   改动量   适合场景
 *      --------- -------- -------- ----------------------------
 *      flag      中       小       任何多层循环，最保险
 *      goto      中       最小     深层嵌套 + 出错清理
 *      return    高       中       能自然抽成函数时（优先）
 *      条件驱动  高       小       判断不贵、层数不深
 *
 * ====================== 五、continue 在两层循环里 ======================
 * continue 只跳过**内层**这一轮，外层该跑几轮还是几轮：
 *
 *      for (i = 1; i <= 3; i++) {
 *          for (j = 1; j <= 3; j++) {
 *              if (i == j) continue;        // 只跳过内层的对角元素
 *              ...
 *          }
 *          ...                              // 外层照常继续
 *      }
 *
 * ====================== 六、break 在 switch 里的坑 ======================
 * 循环里套 switch 时，switch 的 break 一执行，**只跳出 switch**，
 * 循环还会继续下一轮。想在 switch 里结束整个循环，只能：
 *
 *      return（在函数里）  /  flag + 循环条件  /  goto
 *
 * 这一点和02 篇的"贯穿"是同一个知识点的两面。
 *
 * ====================== 七、常见坑 ======================
 *  1. 以为 break 能跳出所有循环         只退出最内层
 *  2. 内层 break 了外层没检查           外层继续跑，结果"停不下来"
 *  3. continue 在 while 里跳过更新       死循环（for 没这个问题）
 *  4. do-while 的 continue 跳到条件      更新写在它后面同样会卡死
 *  5. break/continue 写在循环外          编译错误
 *  6. goto 跳进循环体内部                能编过也不要写，语义混乱
 *  7. switch 里的 break 当成跳出循环     只跳 switch
 *  8. 循环里多次 continue 分散           结构难读，改成 if/else
 *  9. 忘记重置 flag                     外层跑第二轮时永远进不去内层
 * 10. goto 乱跳形成面条代码             只允许向前跳、只用于跳出/清理
 *
 * ====================== 八、goto 的正确姿势：统一清理 ======================
 * 多层资源申请失败时，用 goto 跳到统一的清理段，能避免重复的释放代码：
 *
 *      int rc = 0;
 *      if (!open_a()) { rc = -1; goto cleanup; }
 *      if (!open_b()) { rc = -2; goto cleanup; }
 *      ...
 *      cleanup:
 *          close_all();          // 不管从哪一步来，都从这里统一收尾
 *          return rc;
 *
 * 规则：
 *      标号只放在函数末尾的清理段；只往后跳；绝不用它实现循环。
 *
 * ====================== 九、怎么选（决策表） ======================
 *
 *      需求                                 推荐做法
 *      -----------------------------------  -------------------
 *      单层循环里找到就停                    break
 *      单层循环里跳过某些数据                continue
 *      两层循环一起退出，逻辑简单            flag + 外层条件
 *      三层以上嵌套 + 需要立刻退出           goto（标签放循环外）
 *      能自然抽成独立函数                    return
 *      循环里套 switch 要退出循环            flag / goto / return
 *
 * ====================== 十、实践建议 ======================
 *  - 先问"只退一层够吗"，不够就必须显式传递退出信号
 *  - flag 命名写清楚：found / done / quit，别用 f1、flag2
 *  - 能抽函数就抽函数，return 比任何跳转都好读
 *  - goto 只用于"跳出 + 清理"，且只往前跳
 *  - continue 优先用于"挡掉不合法数据"，而不是跳过一堆正常逻辑
 *  - 每个 break 旁边写一句它跳的是哪一层，复杂嵌套时能救命
 */

#include <stdio.h>

/* 方案 C：抽成函数，用 return 一步结束整个函数 */
static int find_pair(int *pi, int *pj)
{
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            if (i == 2 && j == 2) {
                *pi = i;
                *pj = j;
                return 1;               /* 直接离开函数，两层都结束 */
            }
        }
    }
    return 0;
}

int main(void)
{
    /* ---------- 1. break：找到就停 ---------- */
    printf("1. break：单层循环里找到目标就停\n");
    int a[6] = {5, 12, -3, 8, -1, 20};
    int first_neg = -1;
    for (int i = 0; i < 6; i++) {
        printf("   检查 a[%d] = %d\n", i, a[i]);
        if (a[i] < 0) {
            first_neg = i;
            break;
        }
    }
    printf("   第一个负数在下标 %d，后面的 8、-1、20 都没检查\n", first_neg);

    /* ---------- 2. continue：跳过脏数据 ---------- */
    printf("2. continue：跳过负数，只累加合法值\n");
    int total = 0;
    for (int i = 0; i < 6; i++) {
        if (a[i] < 0) {
            printf("   跳过 a[%d] = %d\n", i, a[i]);
            continue;
        }
        total += a[i];
    }
    printf("   合法值之和 = %d（5 + 12 + 8 + 20）\n", total);

    /* ---------- 3. 错误示范：只在内层 break ---------- */
    printf("3. 错误示范：只在内层 break，外层照样跑完\n");
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            if (i == 2 && j == 2) {
                printf("   内层命中 (i=%d, j=%d)，执行 break\n", i, j);
                break;
            }
            printf("   处理 (%d, %d)\n", i, j);
        }
    }
    printf("   i=3 这一行还是跑了 —— break 只管内层\n");

    /* ---------- 4. 方案 A：标志位 ---------- */
    printf("4. 方案 A：标志位 found，内外配合\n");
    int found = 0, fi = 0, fj = 0;
    for (int i = 1; i <= 3 && !found; i++) {          /* 外层条件带上 !found */
        for (int j = 1; j <= 3; j++) {
            if (i == 2 && j == 2) {
                found = 1;
                fi = i;
                fj = j;
                break;                               /* 跳出内层 */
            }
        }
    }
    printf("   命中 (i=%d, j=%d)，外层条件 !found 变假，整体退出\n", fi, fj);
    printf("   关键：内层 break 之后，外层条件必须能感知到\n");

    /* ---------- 5. 方案 B：goto 一层跳出 ---------- */
    printf("5. 方案 B：goto 一步跳出两层\n");
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            if (i == 2 && j == 2) {
                printf("   命中 (i=%d, j=%d)，goto done\n", i, j);
                goto done;
            }
        }
    }
done:
    printf("   已经跳到两层循环外面，i=3 不会执行\n");
    printf("   goto 只往前跳、只用于跳出，是可接受的写法\n");

    /* ---------- 6. 方案 C：抽函数 + return ---------- */
    printf("6. 方案 C：抽成函数，用 return 结束\n");
    int ri = 0, rj = 0;
    if (find_pair(&ri, &rj))
        printf("   find_pair 返回 1，命中 (%d, %d)，函数里两层同时结束\n", ri, rj);
    printf("   可读性最好，优先考虑这种\n");

    /* ---------- 7. 方案 D：把退出条件写进循环条件 ---------- */
    printf("7. 方案 D：不写 break，让条件驱动\n");
    int got = 0, gi = 0, gj = 0;
    for (int i = 1; i <= 3 && !got; i++)
        for (int j = 1; j <= 3 && !got; j++)
            if (i == 2 && j == 2) {
                got = 1;
                gi = i;
                gj = j;
            }
    printf("   命中 (%d, %d)，条件里带 !got 就够了，不用 break\n", gi, gj);
    printf("   代价：内层会把当前这一轮 j 跑完才退出，逻辑简单时这样最平\n");

    /* ---------- 8. continue 只影响内层 ---------- */
    printf("8. continue 在两层里：只跳过内层这一轮（对角元素跳过）\n");
    for (int i = 1; i <= 3; i++) {
        printf("   ");
        for (int j = 1; j <= 3; j++) {
            if (i == j) {
                printf("- ");
                continue;                        /* 只跳过内层当前元素 */
            }
            printf("%d ", i * j);
        }
        printf("\n");
    }
    printf("   外层 3 行照常打印，continue 管不到外层\n");

    /* ---------- 9. switch 里的 break 只跳 switch ---------- */
    printf("9. 循环里套 switch：switch 的 break 不跳循环\n");
    for (int i = 1; i <= 4; i++) {
        switch (i) {
        case 2:
            printf("   i=%d → 命中 case 2，break 只跳出 switch，循环继续\n", i);
            break;
        default:
            printf("   i=%d → 普通一轮\n", i);
            break;
        }
    }
    printf("   想让循环停下，得用 flag / goto / return\n");

    /* ---------- 10. while / do-while 里的 continue ---------- */
    printf("10. while 里的 continue：更新必须写在它前面\n");
    printf("    安全写法（i++ 在前）：");
    int i = 0;
    while (i < 6) {
        i++;                                 /* ★ 先更新 */
        if (i % 2 == 0)
            continue;
        printf("%d ", i);
    }
    printf("\n    错误写法（死循环，不演示）：while (i < 6) { if (...) continue; i++; }\n");

    /* ---------- 11. goto 的统一清理模式 ---------- */
    printf("11. goto 的正确用法：出错后统一清理\n");
    for (int step = 1; step <= 3; step++) {
        if (step == 2) {
            printf("    步骤 %d 失败，goto cleanup\n", step);
            goto cleanup;
        }
        printf("    步骤 %d 成功\n", step);
    }
cleanup:
    printf("    统一清理：释放已申请的资源，返回错误码\n");
    printf("    标号放在函数末尾、只往前跳，这是 goto 唯一被广泛接受的用法\n");

    /* ---------- 12. 小结 ---------- */
    printf("12. 小结：\n");
    printf("    break 跳出当前一层循环/switch，continue 只跳过本轮\n");
    printf("    两者都只管最内层，想退出两层必须显式传递信号\n");
    printf("    四方案：flag / goto / 抽函数 return / 条件驱动\n");
    printf("    能抽函数就抽函数，return 最清楚；深层嵌套 + 清理用 goto\n");
    printf("    continue：for 会跳到 i++（安全），while/do-while 有死循环风险\n");
    printf("    switch 里的 break 只跳 switch，别指望它结束循环\n");

    return 0;
}

/* ====================== 本程序的运行结果（对照着看） ======================
 * 编译运行：
 *     mkdir -p bin
 *     gcc 06_break与continue.c -o bin/06_break与continue && ./bin/06_break与continue
 *
 * 输出如下：
 *
 * 1. break：单层循环里找到目标就停
 *    检查 a[0] = 5
 *    检查 a[1] = 12
 *    检查 a[2] = -3
 *    第一个负数在下标 2，后面的 8、-1、20 都没检查
 * 2. continue：跳过负数，只累加合法值
 *    跳过 a[2] = -3
 *    跳过 a[4] = -1
 *    合法值之和 = 45（5 + 12 + 8 + 20）
 * 3. 错误示范：只在内层 break，外层照样跑完
 *    处理 (1, 1)
 *    处理 (1, 2)
 *    处理 (1, 3)
 *    处理 (2, 1)
 *    内层命中 (i=2, j=2)，执行 break
 *    处理 (3, 1)
 *    处理 (3, 2)
 *    处理 (3, 3)
 *    i=3 这一行还是跑了 —— break 只管内层
 * 4. 方案 A：标志位 found，内外配合
 *    命中 (i=2, j=2)，外层条件 !found 变假，整体退出
 *    关键：内层 break 之后，外层条件必须能感知到
 * 5. 方案 B：goto 一步跳出两层
 *    命中 (i=2, j=2)，goto done
 *    已经跳到两层循环外面，i=3 不会执行
 *    goto 只往前跳、只用于跳出，是可接受的写法
 * 6. 方案 C：抽成函数，用 return 结束
 *    find_pair 返回 1，命中 (2, 2)，函数里两层同时结束
 *    可读性最好，优先考虑这种
 * 7. 方案 D：不写 break，让条件驱动
 *    命中 (2, 2)，条件里带 !got 就够了，不用 break
 *    代价：内层会把当前这一轮 j 跑完才退出，逻辑简单时这样最平
 * 8. continue 在两层里：只跳过内层这一轮（对角元素跳过）
 *    - 2 3
 *    2 - 6
 *    3 6 -
 *    外层 3 行照常打印，continue 管不到外层
 * 9. 循环里套 switch：switch 的 break 不跳循环
 *    i=1 → 普通一轮
 *    i=2 → 命中 case 2，break 只跳出 switch，循环继续
 *    i=3 → 普通一轮
 *    i=4 → 普通一轮
 *    想让循环停下，得用 flag / goto / return
 * 10. while 里的 continue：更新必须写在它前面
 *     安全写法（i++ 在前）：1 3 5
 *     错误写法（死循环，不演示）：while (i < 6) { if (...) continue; i++; }
 * 11. goto 的正确用法：出错后统一清理
 *     步骤 1 成功
 *     步骤 2 失败，goto cleanup
 *     统一清理：释放已申请的资源，返回错误码
 *     标号放在函数末尾、只往前跳，这是 goto 唯一被广泛接受的用法
 * 12. 小结：
 *     break 跳出当前一层循环/switch，continue 只跳过本轮
 *     两者都只管最内层，想退出两层必须显式传递信号
 *     四方案：flag / goto / 抽函数 return / 条件驱动
 *     能抽函数就抽函数，return 最清楚；深层嵌套 + 清理用 goto
 *     continue：for 会跳到 i++（安全），while/do-while 有死循环风险
 *     switch 里的 break 只跳 switch，别指望它结束循环
 *
 * 几个容易忽略的对照点：
 *   第 1 条：检查到 a[2] 就 break，后面三个元素一次都没碰 —— 这是 break 的价值。
 *   第 2 条：continue 只挡掉两个负数，4 个合法值一个不少地加进去了。
 *   第 3 条：对比第 4~7 条看。这里 (2,3)、(3,*) 依然被"处理"了，
 *           说明只在内层 break 完全达不到"停下来的目的"。
 *   第 4 条：flag 的作用只有一个 —— 让外层条件能"看见"内层发生了什么。
 *   第 5 条：goto done 跳过的是两层循环，i=3 那一整轮直接不存在了。
 *   第 6 条：return 之所以优雅，是把"跳出"变成函数的正常出口。
 *   第 7 条：条件驱动的代价是内层本轮剩余下标仍会被求值（不做处理），
 *           数据量大时会比 break 慢一点点，但代码最平最好读。
 *   第 8 条：对角线上打 "-"，说明 continue 只跳过了内层的 3 个元素，
 *           外层 i 从 1 走到 3，一次都没被跳过。
 *   第 9 条：case 2 的 break 执行完，i=3、i=4 仍然照跑 —— 和贯穿是同一个知识点的两面。
 *   第 11 条：goto cleanup 往前跳到末尾的收尾段，这是内核代码里最常见的用法。
 * ==========================================================================
 */
