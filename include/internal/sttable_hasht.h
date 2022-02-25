#include "trie.h"

/**
 * @brief 计算hash值
 * 
 * @param id 源状态id
 * @param c  转移字符
 * @param base hash桶基数
 * @return int hash值
 */
inline static int sttable_hasht_hash(int id, char c, int base) {
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
static stlist_node_t* sttable_hasht_get_node(struct _sttable_hasht_s *tbl, int id, char c, int h) {
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
static void sttable_hasht_rehash(struct _sttable_hasht_s *tbl) {
    int lnum = 0;
    int size = tbl->size;
    int base = tbl->base;
    for (int i = 0; i < size; i++) {
        stlist_node_t *node = &tbl->nodes[i];
        int h = sttable_hasht_hash(node->fid, node->c, base);
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
static int sttable_hasht_extend(struct _sttable_hasht_s *tbl) {
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
    sttable_hasht_rehash(tbl);
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
static int sttable_hasht_set(struct _sttable_hasht_s *tbl, int fid, char c, int tid) {
    int h = sttable_hasht_hash(fid, c, tbl->base);
    stlist_node_t *node = sttable_hasht_get_node(tbl, fid, c, h);
    if (node != NULL) {
        node->tid = tid;
        return -1;
    }
    if (tbl->size >= tbl->thrd) {
        if (sttable_hasht_extend(tbl) != 0) {
            return -1;
        }
        // 更新哈希值
        h = sttable_hasht_hash(fid, c, tbl->base);
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
static int sttable_hasht_get(struct _sttable_hasht_s *tbl, int id, char c) {
    int h = sttable_hasht_hash(id, c, tbl->base);
    stlist_node_t *node = sttable_hasht_get_node(tbl, id, c, h);
    return node != NULL ? node->tid : -1;
}