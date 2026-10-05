#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "types.h"

/* Helper: duplicate string */
static char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *dup = malloc(len + 1);
    if (dup) strcpy(dup, s);
    return dup;
}

/* Create a new table */
Table *create_table(const char *name, const Column *columns, size_t column_count) {
    if (!name || !columns || column_count == 0) return NULL;
    Table *t = malloc(sizeof(Table));
    if (!t) return NULL;
    t->name = strdup_safe(name);
    t->column_count = column_count;
    t->columns = malloc(sizeof(Column) * column_count);
    if (!t->columns) { free(t->name); free(t); return NULL; }
    for (size_t i = 0; i < column_count; ++i) {
        t->columns[i].name = strdup_safe(columns[i].name);
        t->columns[i].type = columns[i].type;
    }
    t->row_capacity = 4;
    t->rows = malloc(sizeof(Row) * t->row_capacity);
    if (!t->rows) { free(t->name); free(t->columns); free(t); return NULL; }
    t->row_count = 0;
    return t;
}

/* Free a table and all its contents */
void free_table(Table *table) {
    if (!table) return;
    for (size_t i = 0; i < table->column_count; ++i) {
        free(table->columns[i].name);
    }
    free(table->columns);
    for (size_t i = 0; i < table->row_count; ++i) {
        Row *r = &table->rows[i];
        for (size_t j = 0; j < table->column_count; ++j) {
            if (r->values[j].type == VT_STRING) free(r->values[j].u.s);
        }
        free(r->values);
    }
    free(table->rows);
    free(table->name);
    free(table);
}

/* Find column index by name */
size_t find_column_index(const Table *table, const char *column_name) {
    for (size_t i = 0; i < table->column_count; ++i) {
        if (strcmp(table->columns[i].name, column_name) == 0) return i;
    }
    return (size_t)-1;
}

/* Insert a row into the table */
bool insert_row(Table *table, const Value *values, size_t value_count) {
    if (!table || !values || value_count != table->column_count) return false;
    /* Ensure capacity */
    if (table->row_count == table->row_capacity) {
        size_t new_cap = table->row_capacity * 2;
        Row *new_rows = realloc(table->rows, sizeof(Row) * new_cap);
        if (!new_rows) return false;
        table->rows = new_rows;
        table->row_capacity = new_cap;
    }
    Row *r = &table->rows[table->row_count++];
    r->values = malloc(sizeof(Value) * table->column_count);
    if (!r->values) return false;
    for (size_t i = 0; i < table->column_count; ++i) {
        Value v = values[i];
        if (v.type != table->columns[i].type) {
            /* Type mismatch */
            free(r->values);
            --table->row_count;
            return false;
        }
        if (v.type == VT_STRING) {
            r->values[i].type = VT_STRING;
            r->values[i].u.s = strdup_safe(v.u.s);
        } else {
            r->values[i] = v;
        }
    }
    return true;
}

/* Compare two values for equality */
static bool value_equal(const Value *a, const Value *b) {
    if (a->type != b->type) return false;
    switch (a->type) {
        case VT_INT: return a->u.i == b->u.i;
        case VT_DOUBLE: return a->u.d == b->u.d;
        case VT_STRING: return strcmp(a->u.s, b->u.s) == 0;
    }
    return false;
}

/* Select rows where column equals value */
size_t select_rows(const Table *table, const char *column_name, const Value *value, Row ***out_rows) {
    if (!table || !column_name || !value || !out_rows) return 0;
    size_t col_idx = find_column_index(table, column_name);
    if (col_idx == (size_t)-1) return 0;
    if (value->type != table->columns[col_idx].type) return 0;
    /* First pass: count matches */
    size_t count = 0;
    for (size_t i = 0; i < table->row_count; ++i) {
        if (value_equal(&table->rows[i].values[col_idx], value)) ++count;
    }
    if (count == 0) {
        *out_rows = NULL;
        return 0;
    }
    Row **matches = malloc(sizeof(Row*) * count);
    if (!matches) return 0;
    size_t idx = 0;
    for (size_t i = 0; i < table->row_count; ++i) {
        if (value_equal(&table->rows[i].values[col_idx], value)) {
            matches[idx++] = &table->rows[i];
        }
    }
    *out_rows = matches;
    return count;
}
