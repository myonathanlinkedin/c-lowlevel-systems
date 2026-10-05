#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

/* Forward declarations from core.c */
typedef struct { int state; int action; double reward; int next_state; bool done; double priority; } Experience;
typedef struct { Experience **data; size_t capacity; size_t size; } PriorityQueue;
PriorityQueue* pq_create(size_t capacity);
bool pq_push(PriorityQueue *pq, Experience *exp);
Experience* pq_pop(PriorityQueue *pq);
size_t pq_size(PriorityQueue *pq);
void pq_destroy(PriorityQueue *pq);

typedef struct { double *q_table; size_t state_size; size_t action_size; double alpha; double gamma; double epsilon; } Agent;
Agent* agent_create(size_t state_size, size_t action_size, double alpha, double gamma, double epsilon);
void agent_destroy(Agent *a);
int agent_choose_action(Agent *a, int state);
void agent_update_q(Agent *a, Experience *exp);

typedef struct { size_t state_size; size_t action_size; } Environment;
Environment* env_create(size_t state_size, size_t action_size);
void env_destroy(Environment *e);
Experience* env_step(Environment *e, int state, int action);

/* Unit tests */
static void test_priority_queue(void) {
    PriorityQueue *pq = pq_create(10);
    for (int i = 0; i < 5; ++i) {
        Experience *e = malloc(sizeof(Experience));
        e->priority = i;
        pq_push(pq, e);
    }
    assert(pq_size(pq) == 5);
    Experience *top = pq_pop(pq);
    assert(top->priority == 4);
    free(top);
    pq_destroy(pq);
}

static void test_agent_update(void) {
    Agent *a = agent_create(5, 3, 0.1, 0.9, 0.0);
    Experience exp = { .state=1, .action=2, .reward=5.0, .next_state=3, .done=false };
    agent_update_q(a, &exp);
    double val = a->q_table[1 * 3 + 2];
    assert(val > 0.0);
    agent_destroy(a);
}

/* Benchmark training loop */
static void benchmark(void) {
    const size_t steps = 1000;
    Environment *env = env_create(10, 4);
    Agent *agent = agent_create(10, 4, 0.1, 0.9, 0.1);
    clock_t start = clock();
    int state = 0;
    for (size_t i = 0; i < steps; ++i) {
        int action = agent_choose_action(agent, state);
        Experience *exp = env_step(env, state, action);
        agent_update_q(agent, exp);
        state = exp->next_state;
        free(exp);
    }
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Training %zu steps took %.3f seconds.\n", steps, elapsed);
    agent_destroy(agent);
    env_destroy(env);
}

int main(void) {
    srand((unsigned)time(NULL));
    printf("Running unit tests...\n");
    test_priority_queue();
    test_agent_update();
    printf("All unit tests passed.\n");
    benchmark();
    return 0;
}
