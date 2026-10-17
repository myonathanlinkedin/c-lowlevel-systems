#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_basic_intern(void) {
    const char *a1 = cache_command("ls");
    const char *a2 = cache_command("ls");
    const char *b  = cache_command("pwd");
    assert(a1 != NULL && a2 != NULL && b != NULL);
    assert(a1 == a2);               /* same pointer for duplicate */
    assert(strcmp(a1, "ls") == 0);
    assert(strcmp(b, "pwd") == 0);
    assert(a1 != b);
}

static void test_empty_string(void) {
    const char *e1 = cache_command("");
    const char *e2 = cache_command("");
    assert(e1 != NULL && e2 != NULL);
    assert(e1 == e2);
    assert(strcmp(e1, "") == 0);
}

static void test_long_string(void) {
    char long_cmd[256];
    for (int i = 0; i < 255; ++i) long_cmd[i] = 'a' + (i % 26);
    long_cmd[255] = '\0';
    const char *l1 = cache_command(long_cmd);
    const char *l2 = cache_command(long_cmd);
    assert(l1 != NULL && l2 != NULL);
    assert(l1 == l2);
    assert(strcmp(l1, long_cmd) == 0);
}

static void test_null_input(void) {
    const char *n = cache_command(NULL);
    assert(n == NULL);
}

static void test_free_and_reuse(void) {
    const char *x1 = cache_command("git");
    assert(x1 != NULL);
    cache_free();
    const char *x2 = cache_command("git");
    assert(x2 != NULL);
    /* After free, pointer may differ, but string must be equal */
    assert(strcmp(x2, "git") == 0);
    assert(x1 != x2);
    cache_free(); /* clean up again */
}

int main(void) {
    test_basic_intern();
    test_empty_string();
    test_long_string();
    test_null_input();
    test_free_and_reuse();
    printf("All tests passed.\n");
    return 0;
}
