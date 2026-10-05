#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Value type enumeration */
typedef enum {
    VT_INT,
    VT_DOUBLE,
    VT_STRING
} ValueType;

/* Value representation */
typedef struct {
    ValueType type;
    union {
        int i;
        double d;
        char *s;
    } u;
} Value;

/* Column definition */
typedef struct {
    char *name;
    ValueType type;
} Column;

/* Row representation */
typedef struct {
    Value *values; /* array of values, length = column_count */
} Row;

/* Table representation */
typedef struct {
    char *name;
    Column *columns;
    size_t column_count;
    Row *rows;
    size_t row_count;
    size_t row_capacity;
} Table;

/* API prototypes */
Table *create_table(const char *name, const Column *columns, size_t column_count);
void free_table(Table *table);
bool insert_row(Table *table, const Value *values, size_t value_count);
size_t select_rows(const Table *table, const char *column_name, const Value *value, Row ***out_rows);
size_t find_column_index(const Table *table, const char *column_name);

#endif /* TYPES_H */
