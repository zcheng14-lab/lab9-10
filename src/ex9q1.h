#ifndef EX9Q1_H
#define EX9Q1_H

#include <stdio.h>

typedef struct MonopolizationQueueNode MonopolizationQueueNode;
struct MonopolizationQueueNode
{
    int x, y;
    char dir;
    MonopolizationQueueNode *next;
};

typedef struct MonopolizationQueue MonopolizationQueue;
struct MonopolizationQueue
{
    MonopolizationQueueNode *head;
    MonopolizationQueueNode *tail;
};

/**
 * @brief   Dequeues an item from the monopolization queue.
 * @param   monq    (MonopolizationQueue *) Target queue.
 * @param   x       (int *)                 Buffer where the value of the `x` member of the dequeued item is written to.
 * @param   y       (int *)                 Buffer where the value of the `y` member of the dequeued item is written to.
 * @param   dir     (char *)                Buffer where the value of the `dir` member of the dequeued item is written to.
 * @return          (int)                   1 if a dequeue was successful and 0 otherwise.
 */
int deq(MonopolizationQueue *monq, int *x, int *y, char *dir);

/**
 * @brief   Enqueues the four neighbours of a target position to the monopolization queue in order N, E, S, W.
 * @param   monq    (MonopolizationQueue *) Target queue.
 * @param   x       (int)                   X coordinate of the target position.
 * @param   y       (int)                   Y coordinate of the target position.
 */
void enq_neighbours(MonopolizationQueue *monq, int x, int y);

/**
 * Computes the guess position inside a given search space.
 * This function should only be called on a search space that has at least one available position
 * @param   search_min_x    (int)                   Minimum X coordinate value of the current search space.
 * @param   search_max_x    (int)                   Minimum X coordinate value of the current search space.
 * @param   search_min_y    (int)                   Maximum Y coordinate value of the current search space.
 * @param   search_max_y    (int)                   Maximum Y coordinate value of the current search space.
 * @param   is_available    (int (*)(int, int)))    Function that returns 1 if the given position has an enemy city and no salesperson, or no city and no salesperson and 0 otherwise.
 * @param   guess_x         (int*)                  Pointer to buffer where the X value of the guess position is stored.
 * @param   guess_y         (int*)                  Pointer to buffer where the Y value of the guess position is stored.
 */
void get_guess_position(int search_min_x, int search_max_x, int search_min_y, int search_max_y, int (*is_available)(int, int), int *guess_x, int *guess_y)
{
    // Check if middle is valid
    int mid_x = (search_min_x + search_max_x) / 2;
    int mid_y = (search_min_y + search_max_y) / 2;
    int is_inside_search_space = (search_min_x <= mid_x && mid_x <= search_max_x) && (search_min_y <= mid_y && mid_y <= search_max_y);
    if (is_inside_search_space && is_available(mid_x, mid_y))
    {
        *guess_x = mid_x;
        *guess_y = mid_y;
        return;
    }
    
    // Compute the max possible Manhattan distance
    int w = search_max_x - search_min_x + 1;
    int h = search_max_y - search_min_y + 1;
    int max_dist = (w > h) ? 2*(w - 1) : 2*(h - 1);

    // Loop over every distance until an available position is found
    for (int dist = 1; dist <= max_dist; dist++)
    {
        for (int dir = 1; dir <= 4; dir++)
        {
            int xf = (dir == 1 || dir == 3) ? -1 : 1;
            int yf = (dir == 1 || dir == 2) ? -1 : 1;
            int offset = (dir == 2 || dir == 3);
            for (int x = !offset; x <= dist - offset; x++)
            {
                int y = dist - x;
                int cx = mid_x + x*xf;
                int cy = mid_y + y*yf;
                int is_inside_search_space = (search_min_x <= cx && cx <= search_max_x) && (search_min_y <= cy && cy <= search_max_y);
                if (is_inside_search_space && is_available(cx, cy))
                {
                    *guess_x = cx;
                    *guess_y = cy;
                    return;
                }
            }
        }
    }

    fprintf(stderr, "Error: `get_guess` did not find an available position.\n");
}

#endif
