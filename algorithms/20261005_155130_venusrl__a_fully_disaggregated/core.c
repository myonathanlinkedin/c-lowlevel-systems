#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* Experience tuple */
typedef struct {
    int state;
    int action;
    double reward;
    int next_state;
    bool done;
    double priority;
} Experience;

/* Binary max-heap priority queue for experiences */
typedef struct {
    Experience **data;
    size_t capacity;
    size_t size;
} PriorityQueue;

static void pq_swap(Experience **a, Experience **b) {
    Experience *tmp = *a;
    *a = *b;
    *b = tmp;
}

static void pq_heapify_up(PriorityQueue *pq, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) >> 1;
        if (pq->data[idx]->priority <= pq->data[parent]->priority) break;
        pq_swap(&pq->data[idx], &pq->data[parent]);
        idx = parent;
    }
}

static void pq_heapify_down(PriorityQueue *pq, size_t idx) {
    while (1) {
        size_t left = (idx << 1) + 1;
        size_t right = left + 1;
        size_t largest = idx;
        if (left < pq->size && pq->data[left]->priority > pq->data[largest]->priority)
            largest = left;
        if (right < pq->size && pq->data[right]->priority > pq->data[largest]->priority)
            largest = right;
        if (largest == idx) break;
        pq_swap(&pq->data[idx], &pq->data[largest]);
        idx = largest;
    }
}

PriorityQueue* pq_create(size_t capacity) {
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    pq->data = malloc(sizeof(Experience*) * capacity);
    pq->capacity = capacity;
    pq->size = 0;
    return pq;
}

void pq_destroy(PriorityQueue *pq) {
    for (size_t i = 0; i < pq->size; ++i) free(pq->data[i]);
    free(pq->data);
    free(pq);
}

bool pq_push(PriorityQueue *pq, Experience *exp) {
    if (pq->size >= pq->capacity) return false;
    pq->data[pq->size++] = exp;
    pq_heapify_up(pq, pq->size - 1);
    return true;
}

Experience* pq_pop(PriorityQueue *pq) {
    if (pq->size == 0) return NULL;
    Experience *top = pq->data[0];
    pq->data[0] = pq->data[--pq->size];
    pq_heapify_down(pq, 0);
    return top;
}

size_t pq_size(PriorityQueue *pq) { return pq->size; }

/* Simple tabular Q-learning agent */
typedef struct {
    double *q_table;      /* flattened [state * action + action] */
    size_t state_size;
    size_t action_size;
    double alpha;         /* learning rate */
    double gamma;         /* discount factor */
    double epsilon;       /* exploration rate */
} Agent;

Agent* agent_create(size_t state_size, size_t action_size,
                    double alpha, double gamma, double epsilon) {
    Agent *a = malloc(sizeof(Agent));
    a->state_size = state_size;
    a->action_size = action_size;
    a->alpha = alpha;
    a->gamma = gamma;
    a->epsilon = epsilon;
    a->q_table = calloc(state_size * action_size, sizeof(double));
    return a;
}

void agent_destroy(Agent *a) {
    free(a->q_table);
    free(a);
}

int agent_choose_action(Agent *a, int state) {
    if ((double)rand() / RAND_MAX < a->epsilon) {
        return rand() % a->action_size;
    }
    double best = -1e308;
    int best_a = 0;
    for (int act = 0; act < (int)a->action_size; ++act) {
        double val = a->q_table[state * a->action_size + act];
        if (val > best) { best = val; best_a = act; }
    }
    return best_a;
}

void agent_update_q(Agent *a, Experience *exp) {
    size_t idx = exp->state * a->action_size + exp->action;
    double q = a->q_table[idx];
    double max_next = -1e308;
    for (int act = 0; act < (int)a->action_size; ++act) {
        double val = a->q_table[exp->next_state * a->action_size + act];
        if (val > max_next) max_next = val;
    }
    double target = exp->reward + (exp->done ? 0.0 : a->gamma * max_next);
    a->q_table[idx] += a->alpha * (target - q);
}

/* Simple deterministic environment */
typedef struct {
    size_t state_size;
    size_t action_size;
} Environment;

Environment* env_create(size_t state_size, size_t action_size) {
    Environment *e = malloc(sizeof(Environment));
    e->state_size = state_size;
    e->action_size = action_size;
    return e;
}

void env_destroy(Environment *e) { free(e); }

Experience* env_step(Environment *e, int state, int action) {
    Experience *exp = malloc(sizeof(Experience));
    exp->state = state;
    exp->action = action;
    exp->next_state = (state + action) % e->state_size;
    exp->reward = (state + action) % 10;
    exp->done = (exp->next_state == 0);
    exp->priority = fabs(exp->reward); /* simple priority */
    return exp;
}
