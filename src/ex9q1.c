#include <stdio.h>      // 引入"标准输入输出"库，让我们能用 printf(打印) 和 fgets(读行)
#include <stdlib.h>     // 引入"标准库"，让我们能用 exit(退出), atoi(转整数), 和内存函数(malloc, realloc, free)
#include <string.h>     // 引入"字符串"库，让我们能用 strncmp(比较字符串)
#include <stdbool.h>    // 引入"布尔"库，让我们能用 true 和 false (比 1 和 0 更易读)
#include "ex9q1.h"      // 引入题目给的头文件，里面定义了我们必须用的 get_guess_position 和队列结构体

// --- 结构体定义 ---
// typedef struct: 给一个"结构体"类型起一个"昵称"
typedef struct {
    int x;
    int y;
    int w;
    int h;
} City; // "City" 现在是这种结构体的昵称

typedef struct {
    int x;
    int y;
} Position; // "Position" 现在是这种结构体的昵称

// --- 全局变量 ---
// 定义在所有函数外面，整个程序都能访问
City *my_cities = NULL;         // 一个指向 City 类型的指针。我们将用它来创建一个"动态数组"
                                // 'NULL' 表示这个数组目前是空的，不指向任何内存
int num_my_cities = 0;          // 记录数组中有多少个城市
Position *played_positions = NULL; // 同上，用于存储所有"玩过"的坐标
int num_played = 0;                // 记录玩过的坐标数量
int grid_sw_x, grid_sw_y, grid_w, grid_h; // 存储网格的边界信息

/**
 * 核心函数：检查 (x, y) 坐标是否"可用"
 * "可用" = 在网格内 + 不在已知城市上 + 没被猜过
 * @return 1 (true) 代表可用, 0 (false) 代表不可用
 */
int is_available(int x, int y) {
    // 1. 检查是否在网格边界之外 (注意: >= 右/上边界就算越界)
    if (x < grid_sw_x || x >= grid_sw_x + grid_w || y < grid_sw_y || y >= grid_sw_y + grid_h) {
        return 0; // 越界，不可用
    }
    // 2. 遍历所有"我的城市"，检查坐标是否在某个城市内部
    for (int i = 0; i < num_my_cities; i++) {
        City c = my_cities[i]; // 取出第 i 个城市
        // 检查 (x, y) 是否在城市的矩形区域内
        if (x >= c.x && x < c.x + c.w && y >= c.y && y < c.y + c.h) {
            return 0; // 在城市里，不可用
        }
    }
    // 3. 遍历所有"已玩"坐标，检查是否玩过
    for (int i = 0; i < num_played; i++) {
        if (played_positions[i].x == x && played_positions[i].y == y) {
            return 0; // 已经玩过，不可用
        }
    }
    return 1; // 所有检查都通过了，这个点可用！
}

// 添加一个"已玩"位置到 played_positions 动态数组
void add_played_position(int x, int y) {
    num_played++; // 计数+1
    // 'realloc' (重新分配内存) 是动态数组的核心！
    // 它会尝试扩大 'played_positions' 指向的内存，使其能多放一个 Position
    played_positions = (Position*) realloc(played_positions, num_played * sizeof(Position));
    if (played_positions == NULL) exit(1); // 如果内存不够，realloc 会失败返回 NULL，程序退出
    played_positions[num_played - 1].x = x; // 在数组的新末尾 (索引是 n-1) 存入新坐标
    played_positions[num_played - 1].y = y;
}

// 添加一个"我的城市"到 my_cities 动态数组 (原理同上)
void add_my_city(int x, int y, int w, int h) {
    num_my_cities++;
    my_cities = (City*) realloc(my_cities, num_my_cities * sizeof(City));
    if (my_cities == NULL) exit(1); // 内存分配失败，退出
    my_cities[num_my_cities - 1].x = x;
    my_cities[num_my_cities - 1].y = y;
    my_cities[num_my_cities - 1].w = w;
    my_cities[num_my_cities - 1].h = h;
}

// --- 队列 (Queue) 操作 (用于 "垄断模式") ---
// 队列是"先进先出" (FIFO)，像排队。这里用"链表"实现。

// 出队 (Dequeue): 从 'monq' 队列头部取出一个元素
// (参数里的 *x, *y, *dir 是"指针"，用来把取出的值"传回"给调用者)
int deq(MonopolizationQueue *monq, int *x, int *y, char *dir) {
    if (monq->head == NULL) {
        return 0; // 队列是空的，出队失败
    }
    MonopolizationQueueNode *temp = monq->head; // 临时记住队头节点
    *x = temp->x; // 通过指针"传回"数据
    *y = temp->y;
    *dir = temp->dir;
    monq->head = temp->next; // 队头指针 "前进" 到下一个节点
    if (monq->head == NULL) {
        monq->tail = NULL; // 如果队头没了，说明队列空了，队尾也要设为 NULL
    }
    free(temp); // 'free' 释放掉刚才取出的那个节点的内存，防止"内存泄漏"
    return 1; // 出队成功
}

// 入队 (Enqueue): 将 (x, y) 的四个邻居 (N, E, S, W) 加入 'monq' 队尾
void enq_neighbours(MonopolizationQueue *monq, int x, int y) {
    int dx[] = {0, 1, 0, -1}; // 对应 'N', 'E', 'S', 'W' 的 x 变化
    int dy[] = {1, 0, -1, 0}; // 对应 'N', 'E', 'S', 'W' 的 y 变化
    char dirs[] = {'N', 'E', 'S', 'W'};

    for (int i = 0; i < 4; i++) {
        // 'malloc' (内存分配) 申请一块新内存来存放新节点
        MonopolizationQueueNode *newNode = (MonopolizationQueueNode*) malloc(sizeof(MonopolizationQueueNode));
        if (newNode == NULL) {
            exit(1); // 内存申请失败，退出
        }
        newNode->x = x + dx[i]; // 设置新节点的数据
        newNode->y = y + dy[i];
        newNode->dir = dirs[i];
        newNode->next = NULL;   // 新节点总是在队尾，所以它的 next 是 NULL

        if (monq->tail == NULL) { // 如果队列本来是空的
            monq->head = newNode; // head 和 tail 都指向这个新节点
            monq->tail = newNode;
        } else { // 如果队列不是空的
            monq->tail->next = newNode; // 让当前的队尾指向新节点
            monq->tail = newNode;       // 更新队尾标记为这个新节点
        }
    }
}

// 清空队列 (通过不断出队，来释放所有节点的内存)
void clear_queue(MonopolizationQueue *monq) {
    int x, y;
    char dir;
    // 循环: 只要 deq() 成功 (返回 1)，就继续
    while (deq(monq, &x, &y, &dir));
}

// --- 主函数: 程序的入口 ---
// (argc: 命令行参数个数, argv: 存着命令行参数字符串的数组)
int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1; // 运行程序时必须提供玩家编号 (如: ./a.out 1)
    }
    int my_player_num = atoi(argv[1]); // 'atoi': 把字符串 "1" 转换成整数 1
    char buffer[100]; // 准备一个100字节的"缓冲区"，用来

    // --- 游戏主循环 ---
    // 只要能从 'stdin' 读到一行 (比如 "P1:" 或 "P2:")，就继续循环
    while (fgets(buffer, sizeof(buffer), stdin)) {
        int current_turn_player = 0; // 0 = 未知
        if (strncmp(buffer, "P1:", 3) == 0) current_turn_player = 1;
        else if (strncmp(buffer, "P2:", 3) == 0) current_turn_player = 2;

        // A) 如果是别人的回合
        if (current_turn_player != my_player_num) {
            fgets(buffer, sizeof(buffer), stdin); // 读对手的猜测坐标，如 "(10, 20)"
            int x, y;
            if(sscanf(buffer, "(%d, %d)", &x, &y) == 2) {
                add_played_position(x, y); // 必须记录，否则 is_available 会出错
            }
            fgets(buffer, sizeof(buffer), stdin); // 读对手的猜测结果 (H, M, R, G)
            if (strncmp(buffer, "G", 1) == 0) break; // 游戏结束，跳出 while 循环
        
        // B) 如果是我的回合
        } else {
            int guess_x, guess_y; // 准备存放"我"这回合的猜测
            if (is_in_search_mode) {
                // "搜索模式": 调用题目给的 get_guess_position 函数
                // 我们把 is_available (函数指针) 传给它，它就能用我们的函数来检查位置
                get_guess_position(search_min_x, search_max_x, search_min_y, search_max_y, is_available, &guess_x, &guess_y);
            } else {
                // "垄断模式": 从队列里拿一个坐标来猜
                bool found_move = false;
                // 循环: 只要能从队列取出 (deq)
                while (deq(&mon_queue, &guess_x, &guess_y, &last_deq_dir)) {
                    // 检查这个坐标是否"可用" 且 在"hit边界"内
                    if (is_available(guess_x, guess_y) && 
                        (guess_x >= hit_min_x && guess_x <= hit_max_x && guess_y >= hit_min_y && guess_y <= hit_max_y)) {
                        found_move = true; // 找到了一个可以猜的点
                        break; // 退出 deq 循环
                    }
                }
                if (!found_move) {
                    // 队列空了，或者里面的点都无效，说明这个城市找完了
                    is_in_search_mode = true; // 退回到 "搜索模式"
                    clear_queue(&mon_queue); // 清空队列
                    // 重置"搜索边界"为整个网格
                    search_min_x = grid_sw_x;
                    search_max_x = grid_sw_x + grid_w - 1;
                    search_min_y = grid_sw_y;
                    search_max_y = grid_sw_y + grid_h - 1;
                    // 用"搜索模式"再猜一次
                    get_guess_position(search_min_x, search_max_x, search_min_y, search_max_y, is_available, &guess_x, &guess_y);
                }
            }

            // 做出猜测: 打印坐标到 'stdout' (标准输出)
            printf("(%d, %d)\n", guess_x, guess_y);
            // 'fflush(stdout)' 刷新缓冲区！非常重要！
            // 确保我们的猜测立刻被裁判程序读到，而不是卡在半路
            fflush(stdout);
            add_played_position(guess_x, guess_y); // 记录"我"的猜测

            // C) 读取"我"的猜测结果，并更新 AI 状态
            fgets(buffer, sizeof(buffer), stdin); 
            
            if (strncmp(buffer, "H", 1) == 0) { // "H" (Hit - 击中)
                if (is_in_search_mode) {
                    is_in_search_mode = false; // 太好了！从"搜索"切换到"垄断"
                    // 重置 "hit 边界" 为整个网格 (因为是新城市)
                    hit_min_x = grid_sw_x;
                    hit_max_x = grid_sw_x + grid_w - 1;
                    hit_min_y = grid_sw_y;
                    hit_max_y = grid_sw_y + grid_h - 1;
                }
                // 把"击中"点的四个邻居加入队列，准备下次探索
                enq_neighbours(&mon_queue, guess_x, guess_y);
            
            } else if (strncmp(buffer, "M", 1) == 0) { // "M" (Miss - 未击中)
                if (!is_in_search_mode) {
                    // 在"垄断"时 Miss，可以帮我们缩小"hit 边界"
                    // 比如我们是往 'N' (北) 猜的，但 Miss 了，说明城市北边界在 guess_y - 1 以南
                    if (last_deq_dir == 'N') hit_max_y = guess_y - 1;
                    else if (last_deq_dir == 'S') hit_min_y = guess_y + 1;
                    else if (last_deq_dir == 'E') hit_max_x = guess_x - 1;
                    else if (last_deq_dir == 'W') hit_min_x = guess_x + 1;
                }
            
            } else if (strncmp(buffer, "R", 1) == 0) { // "R" (Report - 报告)
                if (is_in_search_mode) {
                    // 在"搜索"时，R 提供了线索，缩小"搜索边界"
                    int n, e, s, w;
                    sscanf(buffer, "R (%d, %d, %d, %d)", &n, &e, &s, &w);
                    if (n > 0) search_min_y = guess_y + 1; // 提示在北，下次只搜北边
                    else if (e > 0) search_min_x = guess_x + 1; // 提示在东
                    else if (s > 0) search_max_y = guess_y - 1; // 提示在南
                    else if (w > 0) search_max_x = guess_x - 1; // 提示在西
                } else {
                    // 在"垄断"时，R 表示这个城市我们已经"完全找到了"
                    is_in_search_mode = true; // 退回到 "搜索模式"
                    clear_queue(&mon_queue); // 清空队列，准备找下一个城市
                    // 重置"搜索边界"
                    search_min_x = grid_sw_x;
                    search_max_x = grid_sw_x + grid_w - 1;
                    search_min_y = grid_sw_y;
                    search_max_y = grid_sw_y + grid_h - 1;
                }
            
            } else if (strncmp(buffer, "G", 1) == 0) { // "G" (Game Over - 游戏结束)
                break; // 跳出 while 循环
            }
        }
    }

    // --- 清理工作 ---
    // 程序结束前，必须 'free' 掉所有 'malloc'/'realloc' 申请的内存
    free(my_cities);          // 释放城市数组
    free(played_positions);   // 释放已玩位置数组
    clear_queue(&mon_queue);  // 释放队列中所有剩余的节点
    
    return 0; // 程序正常退出
}
