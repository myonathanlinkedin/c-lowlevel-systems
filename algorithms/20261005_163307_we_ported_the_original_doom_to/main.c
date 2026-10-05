#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    /* Define columns for a "maps" table */
    Column map_cols[] = {
        {"id", VT_INT},
        {"name", VT_STRING}
    };
    Table *maps = create_table("maps", map_cols, 2);
    assert(maps != NULL);

    /* Insert rows */
    Value row1[] = { {VT_INT, .u.i = 1}, {VT_STRING, .u.s = "First Map"} };
    Value row2[] = { {VT_INT, .u.i = 2}, {VT_STRING, .u.s = "Second Map"} };
    Value row3[] = { {VT_INT, .u.i = 3}, {VT_STRING, .u.s = "Third Map"} };
    assert(insert_row(maps, row1, 2));
    assert(insert_row(maps, row2, 2));
    assert(insert_row(maps, row3, 2));

    /* Attempt to insert with wrong column count */
    Value bad_row[] = { {VT_INT, .u.i = 4} };
    assert(!insert_row(maps, bad_row, 1));

    /* Attempt to insert with type mismatch */
    Value bad_type[] = { {VT_STRING, .u.s = "Bad"}, {VT_STRING, .u.s = "Map"} };
    assert(!insert_row(maps, bad_type, 2));

    /* Select rows where id = 2 */
    Value sel_val = {VT_INT, .u.i = 2};
    Row **results = NULL;
    size_t res_count = select_rows(maps, "id", &sel_val, &results);
    assert(res_count == 1);
    assert(results != NULL);
    assert(strcmp(results[0]->values[1].u.s, "Second Map") == 0);
    free(results);

    /* Select rows where name = "Third Map" */
    Value sel_name = {VT_STRING, .u.s = "Third Map"};
    res_count = select_rows(maps, "name", &sel_name, &results);
    assert(res_count == 1);
    assert(strcmp(results[0]->values[0].u.s, "3") == 0); /* id as string */
    free(results);

    /* Select with non-existent column */
    res_count = select_rows(maps, "nonexistent", &sel_val, &results);
    assert(res_count == 0);
    assert(results == NULL);

    /* Clean up */
    free_table(maps);

    printf("All tests passed.\n");
    return 0;
}
