#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            tests_passed++; \
        } else { \
            tests_failed++; \
            printf("FAIL: %s\n", msg); \
        } \
    } while (0)

static void test_stack_basic(void) {
    Stack s;
    TEST_ASSERT(stack_init(&s, 10) == MEM_OK, "stack_init");
    TEST_ASSERT(stack_is_empty(&s), "stack empty after init");
    TEST_ASSERT(stack_size(&s) == 0, "stack size 0");
    TEST_ASSERT(stack_push(&s, 1) == MEM_OK, "push 1");
    TEST_ASSERT(stack_push(&s, 2) == MEM_OK, "push 2");
    TEST_ASSERT(stack_push(&s, 3) == MEM_OK, "push 3");
    TEST_ASSERT(stack_size(&s) == 3, "stack size 3");
    uint8_t val;
    TEST_ASSERT(stack_peek(&s, &val) == MEM_OK, "peek");
    TEST_ASSERT(val == 3, "peek value 3");
    TEST_ASSERT(stack_pop(&s, &val) == MEM_OK, "pop");
    TEST_ASSERT(val == 3, "pop value 3");
    TEST_ASSERT(stack_pop(&s, &val) == MEM_OK, "pop");
    TEST_ASSERT(val == 2, "pop value 2");
    TEST_ASSERT(stack_pop(&s, &val) == MEM_OK, "pop");
    TEST_ASSERT(val == 1, "pop value 1");
    TEST_ASSERT(stack_is_empty(&s), "stack empty after pops");
    TEST_ASSERT(stack_pop(&s, &val) == MEM_ERR_BOUNDS, "pop empty");
    stack_destroy(&s);
}

static void test_stack_overflow(void) {
    Stack s;
    TEST_ASSERT(stack_init(&s, 2) == MEM_OK, "stack_init cap 2");
    TEST_ASSERT(stack_push(&s, 1) == MEM_OK, "push 1");
    TEST_ASSERT(stack_push(&s, 2) == MEM_OK, "push 2");
    TEST_ASSERT(stack_push(&s, 3) == MEM_ERR_BOUNDS, "overflow");
    stack_destroy(&s);
}

static void test_stack_null(void) {
    uint8_t val;
    TEST_ASSERT(stack_push(NULL, 1) == MEM_ERR_NULL, "push null");
    TEST_ASSERT(stack_pop(NULL, &val) == MEM_ERR_NULL, "pop null");
    TEST_ASSERT(stack_peek(NULL, &val) == MEM_ERR_NULL, "peek null");
    TEST_ASSERT(stack_is_empty(NULL), "empty null");
    TEST_ASSERT(stack_size(NULL) == 0, "size null");
}

static void test_heap_basic(void) {
    Heap h;
    TEST_ASSERT(heap_init(&h, 10) == MEM_OK, "heap_init");
    TEST_ASSERT(heap_is_empty(&h), "heap empty after init");
    TEST_ASSERT(heap_size(&h) == 0, "heap size 0");
    TEST_ASSERT(heap_push(&h, 10) == MEM_OK, "push 10");
    TEST_ASSERT(heap_push(&h, 20) == MEM_OK, "push 20");
    TEST_ASSERT(heap_push(&h, 30) == MEM_OK, "push 30");
    TEST_ASSERT(heap_size(&h) == 3, "heap size 3");
    uint8_t val;
    TEST_ASSERT(heap_peek(&h, &val) == MEM_OK, "peek");
    TEST_ASSERT(val == 30, "peek value 30");
    TEST_ASSERT(heap_pop(&h, &val) == MEM_OK, "pop");
    TEST_ASSERT(val == 30, "pop value 30");
    TEST_ASSERT(heap_pop(&h, &val) == MEM_OK, "pop");
    TEST_ASSERT(val == 20, "pop value 20");
    TEST_ASSERT(heap_pop(&h, &val) == MEM_OK, "pop");
    TEST_ASSERT(val == 10, "pop value 10");
    TEST_ASSERT(heap_is_empty(&h), "heap empty after pops");
    TEST_ASSERT(heap_pop(&h, &val) == MEM_ERR_BOUNDS, "pop empty");
    heap_destroy(&h);
}

static void test_heap_overflow(void) {
    Heap h;
    TEST_ASSERT(heap_init(&h, 2) == MEM_OK, "heap_init cap 2");
    TEST_ASSERT(heap_push(&h, 1) == MEM_OK, "push 1");
    TEST_ASSERT(heap_push(&h, 2) == MEM_OK, "push 2");
    TEST_ASSERT(heap_push(&h, 3) == MEM_ERR_BOUNDS, "overflow");
    heap_destroy(&h);
}

static void test_heap_null(void) {
    uint8_t val;
    TEST_ASSERT(heap_push(NULL, 1) == MEM_ERR_NULL, "push null");
    TEST_ASSERT(heap_pop(NULL, &val) == MEM_ERR_NULL, "pop null");
    TEST_ASSERT(heap_peek(NULL, &val) == MEM_ERR_NULL, "peek null");
    TEST_ASSERT(heap_is_empty(NULL), "empty null");
    TEST_ASSERT(heap_size(NULL) == 0, "size null");
}

static void test_lifo_order(void) {
    Stack s;
    stack_init(&s, 5);
    stack_push(&s, 1);
    stack_push(&s, 2);
    stack_push(&s, 3);
    uint8_t v1, v2, v3;
    stack_pop(&s, &v1);
    stack_pop(&s, &v2);
    stack_pop(&s, &v3);
    TEST_ASSERT(v1 == 3 && v2 == 2 && v3 == 1, "LIFO order");
    stack_destroy(&s);
}

static void test_heap_stack_equivalence(void) {
    Stack s;
    Heap h;
    stack_init(&s, 5);
    heap_init(&h, 5);
    for (int i = 0; i < 5; i++) {
        stack_push(&s, (uint8_t)i);
        heap_push(&h, (uint8_t)i);
    }
    uint8_t sv, hv;
    for (int i = 0; i < 5; i++) {
        stack_pop(&s, &sv);
        heap_pop(&h, &hv);
        TEST_ASSERT(sv == hv, "stack/heap pop equivalence");
    }
    stack_destroy(&s);
    heap_destroy(&h);
}

int main(void) {
    printf("=== Heap vs Stack Memory Tests ===\n");
    test_stack_basic();
    test_stack_overflow();
    test_stack_null();
    test_heap_basic();
    test_heap_overflow();
    test_heap_null();
    test_lifo_order();
    test_heap_stack_equivalence();
    printf("Passed: %d, Failed: %d\n", tests_passed, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
