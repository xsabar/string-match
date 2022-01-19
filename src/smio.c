#include <string.h>
#include <stdlib.h>
#include "smio.h"

match_result_t* match_result_create(int cap) {
    match_result_t *result = (match_result_t *)malloc(sizeof(match_result_t));
    match_item_t *items = (match_item_t *)calloc(cap, sizeof(match_item_t));
    match_result_init(result, items, cap);
    return result;
}

void match_result_destroy(match_result_t *result) {
    free(result->items);
    free(result);
}

void match_result_init(match_result_t *result, match_item_t* items, int cap) {
    result->size = 0;
    result->cap = cap;
    result->items = items;
    memset(items, 0, sizeof(match_item_t) * cap);
}

int match_result_append(match_result_t *result, int plen, int pos) {
    if (result->size >= result->cap) {
        return -1;
    }
    match_item_t* item = &result->items[result->size++];
    item->len = plen;
    item->pos = pos;
    return 0;
}