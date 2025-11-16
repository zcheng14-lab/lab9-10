#ifndef EX10Q1_H
#define EX10Q1_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

typedef struct Candidate Candidate;
struct Candidate
{
    int x, y, cc, pqi;
};

typedef struct SearchPQ SearchPQ;
struct SearchPQ
{
    Candidate **storage;
    int size, capacity;
};

int parent(int i) { return (i - 1) / 2; }
int left(int i) { return 2 * i + 1; }
int right(int i) { return 2 * i + 2; }

Candidate *create_candidate(int x, int y, int cc)
{
    Candidate *cand = (Candidate *) malloc(sizeof(Candidate));
    if (cand == NULL) {
        exit(1);
    }
    cand->x = x;
    cand->y = y;
    cand->cc = cc;
    cand->pqi = -1;
    return cand;
}

void destroy_candidate(Candidate *cand)
{
    free(cand);
}

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

SearchPQ *create_searchPQ()
{
    SearchPQ *pq = (SearchPQ *) malloc(sizeof(SearchPQ));
    if (pq == NULL) {
        exit(1);
    }
    pq->capacity = 10;
    pq->size = 0;
    pq->storage = (Candidate **) malloc(pq->capacity * sizeof(Candidate *));
    if (pq->storage == NULL) {
        exit(1);
    }
    return pq;
}

void destroy_searchPQ(SearchPQ *pq)
{
    if (pq == NULL) {
        return;
    }
    for (int i = 0; i < pq->size; i++) {
        destroy_candidate(pq->storage[i]);
    }
    free(pq->storage);
    free(pq);
}

void pq_swap(SearchPQ *pq, int i, int j) {
    Candidate *temp = pq->storage[i];
    pq->storage[i] = pq->storage[j];
    pq->storage[j] = temp;
    pq->storage[i]->pqi = i;
    pq->storage[j]->pqi = j;
}

void pq_heapify_up(SearchPQ *pq, int i) {
    while (i > 0 && is_prioritized(pq->storage[i], pq->storage[parent(i)])) {
        pq_swap(pq, i, parent(i));
        i = parent(i);
    }
}

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
        pq_heapify_down(pq, max_index);
    }
}

void pq_heapify(SearchPQ *pq, int i)
{
    if (i < 0 || i >= pq->size) {
        return;
    }
    int p = parent(i);
    if (i > 0 && is_prioritized(pq->storage[i], pq->storage[p])) {
        pq_heapify_up(pq, i);
    } else {
        pq_heapify_down(pq, i);
    }
}

void pq_insert(SearchPQ *pq, Candidate *cand)
{
    if (pq->size == pq->capacity) {
        pq->capacity *= 2;
        pq->storage = (Candidate **) realloc(pq->storage, pq->capacity * sizeof(Candidate *));
        if (pq->storage == NULL) {
            exit(1);
        }
    }
    int i = pq->size;
    pq->size++;
    pq->storage[i] = cand;
    cand->pqi = i;
    pq_heapify_up(pq, i);
}

void pq_update(SearchPQ *pq, int i, int new_cc)
{
    int old_cc = pq->storage[i]->cc;
    pq->storage[i]->cc = new_cc;
    if (new_cc > old_cc) {
        pq_heapify_up(pq, i);
    } else {
        pq_heapify_down(pq, i);
    }
}

void pq_remove(SearchPQ *pq, int i)
{
    if (pq->size == 0 || i < 0 || i >= pq->size) {
        return;
    }
    Candidate *to_remove = pq->storage[i];
    int last_index = pq->size - 1;
    
    pq_swap(pq, i, last_index);
    pq->size--;
    destroy_candidate(to_remove);

    if (i < pq->size) {
        pq_heapify(pq, i);
    }
}

void pq_extract_max(SearchPQ *pq, int *x, int *y)
{
    if (pq->size == 0) {
        *x = -1; 
        *y = -1;
        return;
    }
    Candidate *max_cand = pq->storage[0];
    *x = max_cand->x;
    *y = max_cand->y;
    pq_remove(pq, 0);
}

#endif