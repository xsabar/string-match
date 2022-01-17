#include <string.h>
#include <stdlib.h>
#include "wum.h"

Wum* wum_create(block_size) {
    Wum *wum = (Wum *)malloc(sizeof(Wum));
    memset(wum, 0, sizeof(Wum));
    wum->min_len = INT32_MAX;
    wum->block_size = block_size;
    wum->nsize = WUM_DEFAULT_PATTERN_NUM;
    wum->nodes = (wum_slist_node_t *)calloc(WUM_DEFAULT_PATTERN_NUM, sizeof(wum_slist_node_t));
    memset(wum->nodes, 0, sizeof(wum_slist_node_t) * wum->nsize);
    return wum;
}

Wum* wum_create_ex(const char **patterns, int pnum, int block_size) {
    Wum *wum = wum_create(block_size);
    for (int i = 0; i < pnum; i++) {
        wum_insert(wum, patterns[i], strlen(patterns[i]));
    }
    wum_build(wum);
    return wum;
}

void wum_destroy(Wum *wum) {
    free(wum->htbl.lists);
    free(wum->stbl.shift);
    for (int i = 0; i < wum->pnum; i++) {
        free(wum->nodes[i].str);
    }
    free(wum->nodes);
}

int wum_insert(Wum *wum, const char *p, int plen) {
    if (plen <= 0) {
        return 0;
    }
    if (wum->pnum >= wum->nsize) {
        int size = wum->nsize * 2;
        wum_slist_node_t *nodes = (wum_slist_node_t *)realloc(wum->nodes, sizeof(wum_slist_node_t) * size);
        if (nodes == NULL) {
            return -1;
        }
        memset(nodes + wum->nsize, 0, sizeof(wum_slist_node_t) * wum->nsize);
        wum->nodes = nodes;
        wum->nsize = size;
    }
    wum_slist_node_t *node = &wum->nodes[wum->pnum++];
    node->len = plen;
    node->next = NULL;
    node->str = (char *)malloc(plen + 1);
    strncpy(node->str, p, plen + 1);
    if (wum->min_len > plen) {
        wum->min_len = plen;
    }
    wum->stbl.size += plen; // 后续通过该值计算shift表大小
    return 0;
}

/**
 * @brief 计算字符串哈希值
 * 
 * @param str  字符串
 * @param len  字符串长度
 * @param base 基数
 * @return int 哈希值
 */
static int wum_htable_hash(const char *str, int len, int base) {
    uint64_t hash = 0;
    for (int i = 0; i < len; i++) {
        hash = hash * 31 + str[i];
    }
    return (hash ^ (hash >> base)) & ((1 << base) - 1);
}

/**
 * @brief 计算数值基数
 * 
 * @param value 数值
 * @return int  基数
 */
static int wum_calc_base(int value) {
    int base = 0;
    while ((value >> (base + 1)) != 0) {
        ++base;
    }
    if (value > (1 << base)) {
        ++base;
    }
    return base;
}

/**
 * @brief 对象插入哈希表
 * 
 * @param wum  Wum对象指针
 * @param node 字符串节点
 */
static void wum_htable_set(Wum *wum, wum_slist_node_t *node) {
    wum_htable_t *ht = &wum->htbl;
    int hash = wum_htable_hash(node->str + node->len - wum->block_size, wum->block_size, ht->base);
    wum_slist_t *list = &ht->lists[hash];
    wum_slist_node_t *same = list->first;
    while (same != NULL) {
        if (same->len == node->len && (memcmp(same->str, node->str, same->len) == 0)) {
            break;
        }
        same = same->next;
    }
    if (same == NULL) {
        node->next = list->first;
        list->first = node;
    }
}

/**
 * @brief 根据字符串更新位移表
 * 
 * @param wum  Wum对象指针
 * @param node 字符串节点
 */
static void wum_shift_set(Wum *wum, wum_slist_node_t *node) {
    wum_shift_t *st = &wum->stbl;
    for (int i = node->len - wum->min_len + wum->block_size - 1; i < node->len; i++) {
        int shift = node->len - 1 - i;
        int hash = wum_htable_hash(node->str + i - wum->block_size + 1, wum->block_size, st->base);
        if (st->shift[hash] > shift) {
            st->shift[hash] = shift;
        }
    }
}

void wum_build(Wum *wum) {
    if (wum->block_size > wum->min_len) {
        wum->block_size = wum->min_len;
    }
    // 初始化哈希表，假设链表平均长度为2，装载因子0.75
    int cap = wum->pnum * 2 / 3;
    int base = wum_calc_base(cap);
    wum_htable_t *ht = &wum->htbl;
    ht->cap = 1 << base;
    ht->base = base;
    ht->lists = (wum_slist_t *)calloc(ht->cap, sizeof(wum_slist_t));
    memset(ht->lists, 0, sizeof(wum_slist_t) * ht->cap);
    // 初始化位移表
    if (wum->block_size < wum->min_len) {
        wum_shift_t *st = &wum->stbl;
        cap = st->size - (wum->block_size - 1) * wum->pnum;
        base = wum_calc_base(cap);
        st->base = base;
        st->size = 1 << base;
        st->shift = (int *)calloc(st->size, sizeof(int));
        for (int i = 0; i < st->size; i++) {
            st->shift[i] = wum->min_len - wum->block_size + 1;
        }
    }
    for (int i = 0; i < wum->pnum; i++) {
        wum_htable_set(wum, &wum->nodes[i]);
        if (wum->block_size < wum->min_len) {
            wum_shift_set(wum, &wum->nodes[i]);
        }
    }
}

void wum_search(const Wum *wum, const char *s, int slen, match_result_t *result) {
    int hash = 0;
    int min_len = wum->min_len;
    int block_size = wum->block_size;
    for (int i = wum->min_len - 1, shift = 0; i < slen; i += shift) {
        if (block_size < min_len) {
            hash = wum_htable_hash(s + i - block_size + 1, block_size, wum->stbl.base);
            shift = wum->stbl.shift[hash];
            if (shift > 0) {
                continue;
            }
        }
        hash = wum_htable_hash(s + i - block_size + 1, block_size, wum->htbl.base);
        wum_slist_t *list = &wum->htbl.lists[hash];
        wum_slist_node_t *node = list->first;
        while (node != NULL) {
            if (node->len - 1 <= i && (memcmp(node->str, s + i - node->len + 1, node->len) == 0)) {
                match_result_append(result, node->str, node->len, i - node->len + 1);
            }
            node = node->next;
        }
        shift = 1;
    }
}