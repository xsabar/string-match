#include <string.h>
#include <stdlib.h>
#include "smio.h"

match_result_t* match_result_create(int size) {
    match_result_t *result = (match_result_t *)malloc(sizeof(match_result_t));
    match_item_t *items = (match_item_t *)calloc(size, sizeof(match_item_t));
    match_result_init(result, items, size);
    return result;
}

void match_result_destroy(match_result_t *result) {
    free(result->items);
    free(result);
}

void match_result_init(match_result_t *result, match_item_t* items, int size) {
    result->num = 0;
    result->size = size;
    result->items = items;
    memset(items, 0, sizeof(match_item_t) * size);
}

int match_result_append(match_result_t *result, const char *p, int plen, int pos) {
    if (result->size <= result->num) {
        return -1;
    }
    match_item_t* item = &result->items[result->num++];
    item->len = plen;
    item->pattern = p;
    item->pos = pos;
    return 0;
}