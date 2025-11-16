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
    int size, capacity;
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

// 释放 "候选点" 内存
void destroy_candidate(Candidate *cand)
{
    free(cand);
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

// 创建一个新的、空的 "优先队列"
SearchPQ *create_searchPQ()
{
    SearchPQ *pq = (SearchPQ *) malloc(sizeof(SearchPQ));
    if (pq == NULL) {
        exit(1);
    }
    pq->capacity = 10; // 初始容量
    pq->size = 0;
    pq->storage = (Candidate **) malloc(pq->capacity * sizeof(Candidate *));
    if (pq->storage == NULL) {
        exit(1);
    }
    return pq;
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

// "上浮" (Sift Up): 将元素 i 调整到堆的正确位置 (向上)
void pq_heapify_up(SearchPQ *pq, int i) {
    while (i > 0 && is_prioritized(pq->storage[i], pq->storage[parent(i)])) {
        pq_swap(pq, i, parent(i));
        i = parent(i);
    }
}

// "下沉" (Sift Down): 将元素 i 调整到堆的正确位置 (向下)
void pq_heapify_down(SearchPQ *pq, int i) {
    int max_index = i;
    int l = left(i);
    int r = right(i);

    if (l < pq->size && is_prioritized(pq->storage[l], pq->storage[max_index])) {
        max_index = l;
    }
    if (r < pq->size && is_prioritized(pq->storage[r], pq->storage[max_index])) {
        max_index = r;
    }
    if (i != max_index) {
        pq_swap(pq, i, max_index);
        pq_heapify_down(pq, max_index); // 递归下沉
    }
}

// "修复" (Heapify): 自动选择上浮或下沉来修复 i
void pq_heapify(SearchPQ *pq, int i)
{
    if (i < 0 || i >= pq->size) {
        return;
    }
    int p = parent(i);
    if (i > 0 && is_prioritized(pq->storage[i], pq->storage[p])) {
        pq_heapify_up(pq, i); // 应该上浮
    } else {
        pq_heapify_down(pq, i); // 否则下沉
    }
}

// 插入 (Insert): 添加一个新元素到堆中
void pq_insert(SearchPQ *pq, Candidate *cand)
{
    // 1. 检查容量，如果满了就 2 倍扩容
    if (pq->size == pq->capacity) {
        pq->capacity *= 2;
        pq->storage = (Candidate **) realloc(pq->storage, pq->capacity * sizeof(Candidate *));
        if (pq->storage == NULL) {
            exit(1);
        }
    }
    // 2. 放在数组末尾
    int i = pq->size;
    pq->size++;
    pq->storage[i] = cand;
    cand->pqi = i;
    // 3. "上浮" 到正确位置
    pq_heapify_up(pq, i);
}

// 更新 (Update): 修改元素 i 的优先级
void pq_update(SearchPQ *pq, int i, int new_cc)
{
    int old_cc = pq->storage[i]->cc;
    pq->storage[i]->cc = new_cc;
    // 自动判断上浮还是下沉
    if (new_cc > old_cc) {
        pq_heapify_up(pq, i);
    } else {
        pq_heapify_down(pq, i);
    }
}

// 移除 (Remove): 从堆中删除索引为 i 的元素
void pq_remove(SearchPQ *pq, int i)
{
    if (pq->size == 0 || i < 0 || i >= pq->size) {
        return; // 索引无效
    }
    Candidate *to_remove = pq->storage[i]; // 记住它，稍后 free
    int last_index = pq->size - 1;
    
    // 1. 用"最后一个"元素覆盖"要删除"的元素
    pq_swap(pq, i, last_index);
    // 2. 缩小 size (逻辑删除)
    pq->size--;
    // 3. 释放被删除元素的内存
    destroy_candidate(to_remove);

    // 4. "修复"被换到 i 的那个元素 (原 last_index 元素)
    if (i < pq->size) {
        pq_heapify(pq, i);
    }
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
