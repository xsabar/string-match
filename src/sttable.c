#include <string.h>
#include <stdlib.h>
#include "sttable.h"
#include "trie.h"
#include "internal/sttable_list.h"
#include "internal/sttable_array.h"
#include "internal/sttable_hasht.h"
#include "internal/sttable_dbarr.h"

sttable_t* sttable_create(STTableType type) {
    sttable_t *tbl = (sttable_t *)malloc(sizeof(sttable_t));
    memset(tbl, 0, sizeof(sttable_t));
    tbl->type = type;
    if (type == STTABLE_TYPE_ARRAY) {
        tbl->ast.size = STTABLE_DEFAULT_ARRAY_SIZE;
        tbl->ast.stt = (int *)calloc(STTABLE_DEFAULT_ARRAY_SIZE, sizeof(int));
        memset(tbl->ast.stt, -1, STTABLE_DEFAULT_ARRAY_SIZE * sizeof(int));
    } else if (type == STTABLE_TYPE_HASHT) {
        tbl->hst.base = STTABLE_HASHT_CAP_BASE;
        tbl->hst.cap = 1 << tbl->hst.base;
        tbl->hst.thrd = STTABLE_HASHT_DEFAULT_THRD;
        tbl->hst.lists = (stlist_t *)malloc(sizeof(stlist_t) * tbl->hst.cap);
        memset(tbl->hst.lists, 0, sizeof(stlist_t) * tbl->hst.cap);
        // 当hash桶达到负载上限时，状态转移节点数组也同时达到负载上限
        tbl->hst.nodes = (stlist_node_t *)malloc(sizeof(stlist_node_t) * tbl->hst.thrd);
        memset(tbl->hst.nodes, 0, sizeof(stlist_node_t) * tbl->hst.thrd);
    } else if (type == STTABLE_TYPE_DBARR) {
        tbl->dst.bsize = DEFAULT_STATE_NUM;
        tbl->dst.base = (int *)malloc(sizeof(int) * DEFAULT_STATE_NUM);
        memset(tbl->dst.base, 0, sizeof(int) * DEFAULT_STATE_NUM);
        tbl->dst.tsize = STTABLE_DBARR_DEFAULT_SIZE;
        tbl->dst.target = (int *)malloc(sizeof(int) * STTABLE_DBARR_DEFAULT_SIZE);
        memset(tbl->dst.target, 0, sizeof(int) * STTABLE_DBARR_DEFAULT_SIZE);
    } else {}
    return tbl;
}

void sttable_destroy(sttable_t *tbl) {
    if (tbl != NULL) {
        if (tbl->type == STTABLE_TYPE_ARRAY) {
            free(tbl->ast.stt);
        } else if (tbl->type == STTABLE_TYPE_HASHT) {
            free(tbl->hst.lists);
            free(tbl->hst.nodes);
        } else if (tbl->type == STTABLE_TYPE_DBARR) {
            free(tbl->dst.base);
            free(tbl->dst.target);
        } else {}
        free(tbl);
    }
}

int sttable_set(sttable_t *tbl, int fid, char c, int tid) {
    switch (tbl->type) {
        case STTABLE_TYPE_ARRAY:
            return sttable_array_set(&tbl->ast, fid, c, tid);
        case STTABLE_TYPE_HASHT:
            return sttable_hasht_set(&tbl->hst, fid, c, tid);
        case STTABLE_TYPE_LIST:
            return 0;
        case STTABLE_TYPE_DBARR:
            return sttable_dbarr_set(&tbl->dst, fid, c, tid);
        default:
            return -1;
    }
}

int sttable_get(sttable_t *tbl, int id, char c) {
    switch (tbl->type) {
        case STTABLE_TYPE_ARRAY: 
            return sttable_array_get(&tbl->ast, id, c);
        case STTABLE_TYPE_HASHT:
            return sttable_hasht_get(&tbl->hst, id, c);
        case STTABLE_TYPE_LIST:
            return sttable_list_get(&tbl->lst, id, c);
        case STTABLE_TYPE_DBARR:
            return sttable_dbarr_get(&tbl->dst, id, c);
        default:
            return -1;
    }
}

void sttable_copy(sttable_t *tbl, int fid, int tid) {
    memcpy(&tbl->ast.stt[tid * CHARSET_SIZE], &tbl->ast.stt[fid * CHARSET_SIZE], sizeof(int) * CHARSET_SIZE);
}