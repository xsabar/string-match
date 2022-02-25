#include <string.h>
#include <stdlib.h>
#include "sttable.h"
#include "trie.h"
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

/**
 * @brief 数组设置状态转移
 * 
 * @param tbl 表指针
 * @param fid 源状态ID
 * @param c   转移字符
 * @param tid 目标状态ID
 * @return int 0:成功 -1:失败
 */
static int _sttable_array_set(struct _sttable_array_s *tbl, int fid, char c, int tid) {
    int index = fid * CHARSET_SIZE + c;
    if (tid * CHARSET_SIZE >= tbl->size) {
        int size = tbl->size * 2;
        int *stt = (int*)realloc(tbl->stt, size * sizeof(int));
        if (stt == NULL) {
            return -1;
        }
        memset(stt + tbl->size, -1, tbl->size * sizeof(int));
        tbl->size = size;
        tbl->stt = stt;
    }
    tbl->stt[index] = tid;
    return 0;
}

/**
 * @brief 数组获取状态转移
 * 
 * @param tbl 表指针
 * @param id  源状态id
 * @param c   转移字符
 * @return int 目标状态id
 */
static int _sttable_array_get(struct _sttable_array_s *tbl, int id, char c) {
    return tbl->stt[id * CHARSET_SIZE + c];
}

/**
 * @brief 计算hash值
 * 
 * @param id 源状态id
 * @param c  转移字符
 * @param base hash桶基数
 * @return int hash值
 */
inline static int _sttable_hasht_hash(int id, char c, int base) {
    int hash = id * 31 + c;
    return (hash ^ (hash >> base)) & ((1 << base) - 1);
}

/**
 * @brief 获取状态转移节点
 * 
 * @param tbl 表指针
 * @param id  源状态
 * @param c   转移字符
 * @param h   hash值
 * @return stlist_node_t* 目标节点
 */
static stlist_node_t* _sttable_hasht_get_node(struct _sttable_hasht_s *tbl, int id, char c, int h) {
    stlist_node_t *node = tbl->lists[h].first;
    while (node != NULL) {
        if (node->fid == id && node->c == c) {
            return node;
        }
        node = node->next;
    }
    return node;
}

/**
 * @brief rehash
 * 
 * @param tbl 表指针
 */
static void _sttable_hasht_rehash(struct _sttable_hasht_s *tbl) {
    int lnum = 0;
    int size = tbl->size;
    int base = tbl->base;
    for (int i = 0; i < size; i++) {
        stlist_node_t *node = &tbl->nodes[i];
        int h = _sttable_hasht_hash(node->fid, node->c, base);
        stlist_t *list = &tbl->lists[h];
        if (list->first == NULL) {
            ++lnum;
        }
        node->next = list->first;
        list->first = node;
    }
    tbl->lnum = lnum;
}

/**
 * @brief 扩展状态转移表
 * 
 * @param tbl 表指针
 * @return int 0:成功 -1:失败
 */
static int _sttable_hasht_extend(struct _sttable_hasht_s *tbl) {
    int cap = tbl->cap * 2;
    int thrd = tbl->thrd * 2;
    stlist_t *lists = (stlist_t *)malloc(sizeof(stlist_t) * cap);
    if (lists == NULL) {
        return -1;
    }
    stlist_node_t *nodes = (stlist_node_t *)realloc(tbl->nodes, sizeof(stlist_node_t) * thrd);
    if (nodes == NULL) {
        free(lists);
        return -1;
    }
    free(tbl->lists);
    memset(lists, 0, sizeof(stlist_t) * cap);
    // tbl->size=tbl->thrd
    memset(nodes + tbl->size, 0, sizeof(stlist_node_t) * tbl->size);
    tbl->cap = cap;
    tbl->thrd = thrd;
    ++tbl->base;
    tbl->lists = lists;
    tbl->nodes = nodes;
    _sttable_hasht_rehash(tbl);
    return 0;
}

/**
 * @brief 散列表设置状态转移
 * 
 * @param tbl 表指针
 * @param fid 源状态
 * @param c   转移字符
 * @param tid 目标状态
 * @return int 0:成功 -1:失败
 */
static int _sttable_hasht_set(struct _sttable_hasht_s *tbl, int fid, char c, int tid) {
    int h = _sttable_hasht_hash(fid, c, tbl->base);
    stlist_node_t *node = _sttable_hasht_get_node(tbl, fid, c, h);
    if (node != NULL) {
        node->tid = tid;
        return -1;
    }
    if (tbl->size >= tbl->thrd) {
        if (_sttable_hasht_extend(tbl) != 0) {
            return -1;
        }
        // 更新哈希值
        h = _sttable_hasht_hash(fid, c, tbl->base);
    }
    stlist_t *list = &tbl->lists[h];
    node = &tbl->nodes[tbl->size++];
    node->fid = fid;
    node->c = c;
    node->tid = tid;
    if (list->first == NULL) {
        ++tbl->lnum;
    }
    node->next = list->first;
    list->first = node;
    return 0;
}

/**
 * @brief 散列表获取状态转移
 * 
 * @param tbl 表指针
 * @param id  源状态
 * @param c   转移字符
 * @return int 目标状态
 */
static int _sttable_hasht_get(struct _sttable_hasht_s *tbl, int id, char c) {
    int h = _sttable_hasht_hash(id, c, tbl->base);
    stlist_node_t *node = _sttable_hasht_get_node(tbl, id, c, h);
    return node != NULL ? node->tid : -1;
}

static int _sttable_list_get(struct _sttable_list_s *tbl, int id, char c) {
    int child = tbl->trie->states[id].first;
    TrieState *state = NULL;
    while (child != 0) {
        state = &tbl->trie->states[child];
        if (state->c == c) {
            return child;
        }
        child = state->next;
    }
    return -1;
}

int sttable_set(sttable_t *tbl, int fid, char c, int tid) {
    switch (tbl->type) {
        case STTABLE_TYPE_ARRAY:
            return _sttable_array_set(&tbl->ast, fid, c, tid);
        case STTABLE_TYPE_HASHT:
            return _sttable_hasht_set(&tbl->hst, fid, c, tid);
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
            return _sttable_array_get(&tbl->ast, id, c);
        case STTABLE_TYPE_HASHT:
            return _sttable_hasht_get(&tbl->hst, id, c);
        case STTABLE_TYPE_LIST:
            return _sttable_list_get(&tbl->lst, id, c);
        case STTABLE_TYPE_DBARR:
            return sttable_dbarr_get(&tbl->dst, id, c);
        default:
            return -1;
    }
}

void sttable_copy(sttable_t *tbl, int fid, int tid) {
    memcpy(&tbl->ast.stt[tid * CHARSET_SIZE], &tbl->ast.stt[fid * CHARSET_SIZE], sizeof(int) * CHARSET_SIZE);
}