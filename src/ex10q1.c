#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ex9q1.h"
#include "ex10q1.h"

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

City *my_cities = NULL;
int num_my_cities = 0;
Position *played_positions = NULL;
int num_played = 0;
int grid_sw_x, grid_sw_y, grid_w, grid_h;
SearchPQ *pq = NULL;

int is_available(int x, int y) {
    if (x < grid_sw_x || x >= grid_sw_x + grid_w || y < grid_sw_y || y >= grid_sw_y + grid_h) {
        return 0;
    }

    for (int i = 0; i < num_my_cities; i++) {
        City c = my_cities[i];
        if (x >= c.x && x < c.x + c.w && y >= c.y && y < c.y + c.h) {
            return 0;
        }
    }

    for (int i = 0; i < num_played; i++) {
        if (played_positions[i].x == x && played_positions[i].y == y) {
            return 0;
        }
    }
    return 1;
}

void add_played_position(int x, int y) {
    num_played++;
    played_positions = (Position*) realloc(played_positions, num_played * sizeof(Position));
    if (played_positions == NULL) exit(1);
    played_positions[num_played - 1].x = x;
    played_positions[num_played - 1].y = y;
}

void add_my_city(int x, int y, int w, int h) {
    num_my_cities++;
    my_cities = (City*) realloc(my_cities, num_my_cities * sizeof(City));
    if (my_cities == NULL) exit(1);
    my_cities[num_my_cities - 1].x = x;
    my_cities[num_my_cities - 1].y = y;
    my_cities[num_my_cities - 1].w = w;
    my_cities[num_my_cities - 1].h = h;
}

int deq(MonopolizationQueue *monq, int *x, int *y, char *dir) {
    if (monq->head == NULL) {
        return 0;
    }
    MonopolizationQueueNode *temp = monq->head;
    *x = temp->x;
    *y = temp->y;
    *dir = temp->dir;
    monq->head = temp->next;
    if (monq->head == NULL) {
        monq->tail = NULL;
    }
    free(temp);
    return 1;
}

void enq_neighbours(MonopolizationQueue *monq, int x, int y) {
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    char dirs[] = {'N', 'E', 'S', 'W'};

    for (int i = 0; i < 4; i++) {
        MonopolizationQueueNode *newNode = (MonopolizationQueueNode*) malloc(sizeof(MonopolizationQueueNode));
        if (newNode == NULL) {
            exit(1);
        }
        newNode->x = x + dx[i];
        newNode->y = y + dy[i];
        newNode->dir = dirs[i];
        newNode->next = NULL;

        if (monq->tail == NULL) {
            monq->head = newNode;
            monq->tail = newNode;
        } else {
            monq->tail->next = newNode;
            monq->tail = newNode;
        }
    }
}

void clear_queue(MonopolizationQueue *monq) {
    int x, y;
    char dir;
    while (deq(monq, &x, &y, &dir));
}

int find_candidate(int x, int y) {
    if (pq == NULL) return -1;
    for (int i = 0; i < pq->size; i++) {
        if (pq->storage[i]->x == x && pq->storage[i]->y == y) {
            return i;
        }
    }
    return -1;
}

void add_candidate_point(int x, int y) {
    if (!is_available(x, y)) {
        return;
    }
    int pqi = find_candidate(x, y);
    if (pqi != -1) {
        pq_update(pq, pqi, pq->storage[pqi]->cc + 1);
    } else {
        Candidate *c = create_candidate(x, y, 1);
        pq_insert(pq, c);
    }
}

void add_diamond_candidates(int guess_x, int guess_y, int dist) {
    if (dist == 0) {
        add_candidate_point(guess_x, guess_y);
        return;
    }
    for (int i = 0; i < dist; i++) {
        int j = dist - i;
        add_candidate_point(guess_x + i, guess_y + j);
        add_candidate_point(guess_x - i, guess_y - j);
        add_candidate_point(guess_x + j, guess_y - i);
        add_candidate_point(guess_x - j, guess_y + i);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }
    int my_player_num = atoi(argv[1]);
    char buffer[100];

    fgets(buffer, sizeof(buffer), stdin);
    while (fgets(buffer, sizeof(buffer), stdin) && strncmp(buffer, "done", 4) != 0) {
        if (my_player_num == 1) {
            int x, y, w, h;
            if(sscanf(buffer, "(%d, %d), %d, %d", &x, &y, &w, &h) == 4) {
                add_my_city(x, y, w, h);
            }
        }
    }
    
    fgets(buffer, sizeof(buffer), stdin);
    while (fgets(buffer, sizeof(buffer), stdin) && strncmp(buffer, "done", 4) != 0) {
        if (my_player_num == 2) {
            int x, y, w, h;
            if(sscanf(buffer, "(%d, %d), %d, %d", &x, &y, &w, &h) == 4) {
                add_my_city(x, y, w, h);
            }
        }
    }
    
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "(%d, %d), %d, %d", &grid_sw_x, &grid_sw_y, &grid_w, &grid_h);

    int search_min_x = grid_sw_x;
    int search_max_x = grid_sw_x + grid_w - 1;
    int search_min_y = grid_sw_y;
    int search_max_y = grid_sw_y + grid_h - 1;

    bool is_in_search_mode = true;
    MonopolizationQueue mon_queue = {NULL, NULL};
    pq = create_searchPQ();
    
    int hit_min_x = grid_sw_x;
    int hit_max_x = grid_sw_x + grid_w - 1;
    int hit_min_y = grid_sw_y;
    int hit_max_y = grid_sw_y + grid_h - 1;

    char last_deq_dir = ' ';

    while (fgets(buffer, sizeof(buffer), stdin)) {
        int current_turn_player = 0;
        if (strncmp(buffer, "P1:", 3) == 0) {
            current_turn_player = 1;
        } else if (strncmp(buffer, "P2:", 3) == 0) {
            current_turn_player = 2;
        }

        if (current_turn_player != my_player_num) {
            fgets(buffer, sizeof(buffer), stdin);
            int x, y;
            if(sscanf(buffer, "(%d, %d)", &x, &y) == 2) {
                add_played_position(x, y);
                int pqi = find_candidate(x, y);
                if (pqi != -1) {
                    pq_remove(pq, pqi);
                }
            }
            
            fgets(buffer, sizeof(buffer), stdin);
            if (strncmp(buffer, "G", 1) == 0) {
                break;
            }
        } else {
            int guess_x, guess_y;

            if (is_in_search_mode) {
                if (pq->size == 0) {
                    get_guess_position(search_min_x, search_max_x, search_min_y, search_max_y, is_available, &guess_x, &guess_y);
                } else {
                    pq_extract_max(pq, &guess_x, &guess_y);
                }
            } else {
                bool found_move = false;
                while (deq(&mon_queue, &guess_x, &guess_y, &last_deq_dir)) {
                    if (is_available(guess_x, guess_y) && 
                        (guess_x >= hit_min_x && guess_x <= hit_max_x && guess_y >= hit_min_y && guess_y <= hit_max_y)) {
                        found_move = true;
                        break;
                    }
                }
                if (!found_move) {
                    is_in_search_mode = true;
                    clear_queue(&mon_queue);
                    if (pq->size == 0) {
                         get_guess_position(search_min_x, search_max_x, search_min_y, search_max_y, is_available, &guess_x, &guess_y);
                    } else {
                        pq_extract_max(pq, &guess_x, &guess_y);
                    }
                }
            }

            printf("(%d, %d)\n", guess_x, guess_y);
            fflush(stdout);
            add_played_position(guess_x, guess_y);
            int pqi = find_candidate(guess_x, guess_y);
            if (pqi != -1) {
                pq_remove(pq, pqi);
            }

            fgets(buffer, sizeof(buffer), stdin); 
            
            if (strncmp(buffer, "H", 1) == 0) {
                if (is_in_search_mode) {
                    is_in_search_mode = false;
                    hit_min_x = grid_sw_x;
                    hit_max_x = grid_sw_x + grid_w - 1;
                    hit_min_y = grid_sw_y;
                    hit_max_y = grid_sw_y + grid_h - 1;
                }
                enq_neighbours(&mon_queue, guess_x, guess_y);
            } else if (strncmp(buffer, "M", 1) == 0) {
                if (!is_in_search_mode) {
                    if (last_deq_dir == 'N') hit_max_y = guess_y - 1;
                    else if (last_deq_dir == 'S') hit_min_y = guess_y + 1;
                    else if (last_deq_dir == 'E') hit_max_x = guess_x - 1;
                    else if (last_deq_dir == 'W') hit_min_x = guess_x + 1;
                }
            } else if (strncmp(buffer, "R", 1) == 0) {
                if (is_in_search_mode) {
                    int dist;
                    if(sscanf(buffer, "R %d", &dist) == 1) {
                        if (dist > 0) {
                            add_diamond_candidates(guess_x, guess_y, dist);
                        }
                    }
                } else {
                    is_in_search_mode = true;
                    clear_queue(&mon_queue);
                }
            } else if (strncmp(buffer, "G", 1) == 0) {
                break;
            }
        }
    }

    free(my_cities);
    free(played_positions);
    clear_queue(&mon_queue);
    destroy_searchPQ(pq);
    
    return 0;
}