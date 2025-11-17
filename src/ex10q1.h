#ifndef EX10Q1_H
#define EX10Q1_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

// 结构体: "候选点", 存储坐标和优先级(cc)
typedef struct Candidate Candidate;
struct Candidate
{
    int x, y, cc, pqi; // pqi 是它在数组中的索引
};

// 结构体: "优先队列" 管理器
typedef struct SearchPQ SearchPQ;
struct SearchPQ
{
    Candidate **storage; // 动态数组, 存的是指向 Candidate 的指针
};

// --- 堆 (Heap) 索引计算 ---
int parent(int i) { return (i - 1) / 2; }
int left(int i) { return 2 * i + 1; }
int right(int i) { return 2 * i + 2; }

// 创建一个新的 "候选点" 结构体
Candidate *create_candidate(int x, int y, int cc)
{
    Candidate *cand = (Candidate *) malloc(sizeof(Candidate));
    if (cand == NULL) {
        exit(1);
    }
    cand->x = x;
    cand->y = y;
    cand->cc = cc;
    cand->pqi = -1; // -1 表示不在队列中
    return cand;
}



// 核心: 比较两个候选点的优先级
// 1. cc 越大越优先
// 2. cc 相同, y 越小越优先
// 3. cc 和 y 相同, x 越小越优先
int is_prioritized(Candidate *cand1, Candidate *cand2)
{
    if (cand1->cc > cand2->cc) {
        return 1;
    }
    if (cand1->cc < cand2->cc) {
        return 0;
    }
    if (cand1->y < cand2->y) {
        return 1;
    }
    if (cand1->y > cand2->y) {
        return 0;
    }
    if (cand1->x < cand2->x) {
        return 1;
    }
    return 0;
}



// 销毁 "优先队列" (释放所有相关内存)
void destroy_searchPQ(SearchPQ *pq)
{
    if (pq == NULL) {
        return;
    }
    // 1. 释放所有 Candidate 结构体
    for (int i = 0; i < pq->size; i++) {
        destroy_candidate(pq->storage[i]);
    }
    // 2. 释放 storage 数组
    free(pq->storage);
    // 3. 释放 pq 管理器
    free(pq);
}

// 交换数组中 i 和 j 位置的元素 (指针)
void pq_swap(SearchPQ *pq, int i, int j) {
    Candidate *temp = pq->storage[i];
    pq->storage[i] = pq->storage[j];
    pq->storage[j] = temp;
    // !! 关键: 交换后必须更新 pqi 索引 !!
    pq->storage[i]->pqi = i;
    pq->storage[j]->pqi = j;
}



// 提取最大 (Extract Max): 移除并返回"优先级最高"的元素
void pq_extract_max(SearchPQ *pq, int *x, int *y)
{
    if (pq->size == 0) {
        *x = -1; // 队空，返回无效值
        *y = -1;
        return;
    }
    // 1. 优先级最高的永远在 索引 0
    Candidate *max_cand = pq->storage[0];
    *x = max_cand->x; // "传回"坐标
    *y = max_cand->y;
    // 2. 移除 索引 0 的元素 (pq_remove 会自动修复堆)
    pq_remove(pq, 0);
}

#endif // 结束 #ifndef EX10Q1_H
