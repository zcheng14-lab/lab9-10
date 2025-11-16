#include <stdio.h>      // 引入"标准输入输出"库 (printf, fgets, sscanf)
#include <stdlib.h>     // 引入"标准库" (exit, atoi, malloc, realloc, free)
#include <string.h>     // 引入"字符串"库 (strncmp)
#include <stdbool.h>    // 引入"布尔"库 (true, false)
#include "ex9q1.h"      // 包含上个练习的头文件 (定义了"垄断队列" MonopolizationQueue)
#include "ex10q1.h"     // !!新!! 包含这个练习的头文件 (定义了"优先队列" SearchPQ 和 "候选点" Candidate)

// --- 结构体定义 (和 ex9q1 一样) ---
typedef struct {
    int x;
    int y;
    int w;
    int h;
} City;

typedef struct {
    int x;
    int y;
} Position;

// --- 全局变量 ---
City *my_cities = NULL;         // 存储"我的城市"的动态数组
int num_my_cities = 0;
Position *played_positions = NULL; // 存储"所有已玩位置"的动态数组
int num_played = 0;
int grid_sw_x, grid_sw_y, grid_w, grid_h; // 网格边界

// !!新!! 智能搜索用的"优先队列" (Priority Queue) 指针
// 它会帮我们存储所有"R"报告提供的线索 (候选点)，并自动排序
SearchPQ *pq = NULL;

// 检查 (x, y) 是否可用 (越界、在城市、已玩过)
// (和 ex9q1 一样)
int is_available(int x, int y) {
    if (x < grid_sw_x || x >= grid_sw_x + grid_w || y < grid_sw_y || y >= grid_sw_y + grid_h) {
        return 0; // 越界
    }
    for (int i = 0; i < num_my_cities; i++) {
        City c = my_cities[i];
        if (x >= c.x && x < c.x + c.w && y >= c.y && y < c.y + c.h) {
            return 0; // 在城市里
        }
    }
    for (int i = 0; i < num_played; i++) {
        if (played_positions[i].x == x && played_positions[i].y == y) {
            return 0; // 已玩过
        }
    }
    return 1; // 可用
}

// 添加一个"已玩"位置到动态数组
// (和 ex9q1 一样)
void add_played_position(int x, int y) {
    num_played++;
    played_positions = (Position*) realloc(played_positions, num_played * sizeof(Position));
    if (played_positions == NULL) exit(1);
    played_positions[num_played - 1].x = x;
    played_positions[num_played - 1].y = y;
}

// 添加一个"我的城市"到动态数组
// (和 ex9q1 一样)
void add_my_city(int x, int y, int w, int h) {
    num_my_cities++;
    my_cities = (City*) realloc(my_cities, num_my_cities * sizeof(City));
    if (my_cities == NULL) exit(1);
    my_cities[num_my_cities - 1].x = x;
    my_cities[num_my_cities - 1].y = y;
    my_cities[num_my_cities - 1].w = w;
    my_cities[num_my_cities - 1].h = h;
}


// 入队 (Enqueue): 将 (x, y) 的四个邻居加入队尾
void enq_neighbours(MonopolizationQueue *monq, int x, int y) {
    int dx[] = {0, 1, 0, -1}; // N, E, S, W
    int dy[] = {1, 0, -1, 0};
    char dirs[] = {'N', 'E', 'S', 'W'};
    for (int i = 0; i < 4; i++) {
        MonopolizationQueueNode *newNode = (MonopolizationQueueNode*) malloc(sizeof(MonopolizationQueueNode));
        if (newNode == NULL) exit(1);
        newNode->x = x + dx[i];
        newNode->y = y + dy[i];
        newNode->dir = dirs[i];
        newNode->next = NULL;
        if (monq->tail == NULL) { // 队空
            monq->head = newNode;
            monq->tail = newNode;
        } else { // 链接到队尾
            monq->tail->next = newNode;
            monq->tail = newNode;
        }
    }
}

// 清空队列 (释放所有节点内存)
void clear_queue(MonopolizationQueue *monq) {
    int x, y;
    char dir;
    while (deq(monq, &x, &y, &dir)); // 持续出队
}

// --- 主函数 ---
int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    int my_player_num = atoi(argv[1]); // "1" -> 1
    char buffer[100]; // 读行缓冲区

    // --- 游戏设置阶段 (和 ex9q1 一样) ---
    fgets(buffer, sizeof(buffer), stdin); // 读 "P1 cities:"
    while (fgets(buffer, sizeof(buffer), stdin) && strncmp(buffer, "done", 4) != 0) {
        if (my_player_num == 1) {
            int x, y, w, h;
            if(sscanf(buffer, "(%d, %d), %d, %d", &x, &y, &w, &h) == 4) {
                add_my_city(x, y, w, h);
            }
        }
    }
    fgets(buffer, sizeof(buffer), stdin); // 读 "P2 cities:"
    while (fgets(buffer, sizeof(buffer), stdin) && strncmp(buffer, "done", 4) != 0) {
        if (my_player_num == 2) {
            int x, y, w, h;
            if(sscanf(buffer, "(%d, %d), %d, %d", &x, &y, &w, &h) == 4) {
                add_my_city(x, y, w, h);
            }
        }
    }
    fgets(buffer, sizeof(buffer), stdin); // 读 "Grid:"
    sscanf(buffer, "(%d, %d), %d, %d", &grid_sw_x, &grid_sw_y, &grid_w, &grid_h);


    // --- 游戏主循环 ---
    while (fgets(buffer, sizeof(buffer), stdin)) {
        int current_turn_player = 0;
        if (strncmp(buffer, "P1:", 3) == 0) current_turn_player = 1;
        else if (strncmp(buffer, "P2:", 3) == 0) current_turn_player = 2;

        // A) 别人的回合
        if (current_turn_player != my_player_num) {
            fgets(buffer, sizeof(buffer), stdin); // 读对手猜测
            int x, y;
            if(sscanf(buffer, "(%d, %d)", &x, &y) == 2) {
                add_played_position(x, y); // 记录
                // !!新!! 检查对手是否"抢"了我们的候选点
                int pqi = find_candidate(x, y); 
                if (pqi != -1) {
                    pq_remove(pq, pqi); // 如果是，从我们的队列中移除
                }
            }
            fgets(buffer, sizeof(buffer), stdin); // 读对手结果
            if (strncmp(buffer, "G", 1) == 0) break; // 游戏结束
        
        // B) 我的回合
        } else {
            int guess_x, guess_y;
            if (is_in_search_mode) {
                // !!新!! "搜索模式" 逻辑升级！
                if (pq->size == 0) {
                    // 1. 如果优先队列是空的 (没线索)
                    //    退回 (fallback) 到 ex9q1 的老办法：get_guess_position 盲扫
                    get_guess_position(search_min_x, search_max_x, search_min_y, search_max_y, is_available, &guess_x, &guess_y);
                } else {
                    // 2. !!关键!! 如果优先队列 *有* 线索
                    //    调用 pq_extract_max() 取出"优先级最高"(最可疑)的那个点来猜！
                    pq_extract_max(pq, &guess_x, &guess_y);
                }
            } else {
                // "垄断模式" (和 ex9q1 一样): 从普通队列(mon_queue)取
                bool found_move = false;
                while (deq(&mon_queue, &guess_x, &guess_y, &last_deq_dir)) {
                    if (is_available(guess_x, guess_y) && 
                        (guess_x >= hit_min_x && guess_x <= hit_max_x && guess_y >= hit_min_y && guess_y <= hit_max_y)) {
                        found_move = true; // 找到有效猜测
                        break;
                    }
                }
                if (!found_move) {
                    // 垄断队列空了，退回"搜索模式"
                    is_in_search_mode = true;
                    clear_queue(&mon_queue);
                    // !!新!! 退回时，也优先用 PQ
                    if (pq->size == 0) { // 没线索，用老办法
                         get_guess_position(search_min_x, search_max_x, search_min_y, search_max_y, is_available, &guess_x, &guess_y);
                    } else { // 有线索，用新办法
                        pq_extract_max(pq, &guess_x, &guess_y);
                    }
                }
            }

            // 做出猜测
            printf("(%d, %d)\n", guess_x, guess_y);
            fflush(stdout); // !!重要!! 刷新缓冲区，立刻把猜测发给裁判
            add_played_position(guess_x, guess_y); // 记录自己的猜测
            // !!新!! 检查我们刚猜的点
            int pqi = find_candidate(guess_x, guess_y);
            if (pqi != -1) {
                pq_remove(pq, pqi); // !!新!! 如果它在优先队列里，就移除 (猜过了)
            }

            // C) 读取结果并更新 AI 状态
            fgets(buffer, sizeof(buffer), stdin); 
            
            if (strncmp(buffer, "H", 1) == 0) { // "H" (Hit - 击中)
                if (is_in_search_mode) {
                    is_in_search_mode = false; // 切换到"垄断模式"
                    // 重置 hit 边界 (和 ex9q1 一样)
                    hit_min_x = grid_sw_x;
                    hit_max_x = grid_sw_x + grid_w - 1;
                    hit_min_y = grid_sw_y;
                    hit_max_y = grid_sw_y + grid_h - 1;
                }
                enq_neighbours(&mon_queue, guess_x, guess_y); // 邻居加入"垄断队列"
            
            } else if (strncmp(buffer, "M", 1) == 0) { // "M" (Miss - 未击中)
                if (!is_in_search_mode) {
                    // "垄断"时 Miss，缩小 hit 边界 (和 ex9q1 一样)
                    if (last_deq_dir == 'N') hit_max_y = guess_y - 1;
                    else if (last_deq_dir == 'S') hit_min_y = guess_y + 1;
                    else if (last_deq_dir == 'E') hit_max_x = guess_x - 1;
                    else if (last_deq_dir == 'W') hit_min_x = guess_x + 1;
                }
            
            } else if (strncmp(buffer, "R", 1) == 0) { // "R" (Report - 报告)
                if (is_in_search_mode) {
                    // !!新!! "搜索模式" 下的 R 逻辑
                    int dist;
                    // sscanf 尝试解析 "R %d" (R 后面跟一个距离)
                    if(sscanf(buffer, "R %d", &dist) == 1) { 
                        if (dist > 0) {
                            // 收到线索！把"菱形"上的点加入"优先队列"
                            add_diamond_candidates(guess_x, guess_y, dist);
                        }
                    }
                    // 注意：我们不再用 ex9q1 的方法缩小 search_min/max 边界
                    // 因为"优先队列"是更高级的线索处理方式
                
                } else {
                    // "垄断模式" 下 R，表示城市已找完，退回"搜索模式"
                    is_in_search_mode = true;
                    clear_queue(&mon_queue);
                }
            
            } else if (strncmp(buffer, "G", 1) == 0) { // "G" (Game Over - 游戏结束)
                break; // 跳出主循环
            }
        }
    }

    // --- 清理 ---
    // 释放所有动态分配 (malloc/realloc) 的内存
    free(my_cities);
    free(played_positions);
    clear_queue(&mon_queue);    // 释放"垄断队列"
    destroy_searchPQ(pq);       // !!新!! 销毁"优先队列"，释放所有候选点内存
    
    return 0; // 程序正常退出
}
