#ifndef _HTABLE_H
#define _HTABLE_H

#define HTABLE_CAP_BASE 4 // base=4,cap=2^4=16
#define HTABLE_DEFAULT_THRD  (1 << HTABLE_CAP_BASE) * 3 / 4 // (cap * load_factor)

/**
 * @brief 字符串节点
 */
typedef struct _ht_slist_node_s {
    int len;   // 字符串长度
    char *str; // 字符串
    struct _ht_slist_node_s *next; // 指向下个节点
} ht_slist_node_t;

/**
 * @brief 字符串链表
 */
typedef struct {
    ht_slist_node_t *first; // 指向首元素
} ht_slist_t;

/**
 * @brief 字符串哈希表
 */
typedef struct {
    int cap;  // 容量
    int thrd; // 扩容阈值
    int size; // 元素数量
    int lnum; // 非空链表数
    int base; // cap = 1 << base
    ht_slist_t *lists;      // 链表集合
    ht_slist_node_t *nodes; // 元素集合
} htable_t;

htable_t* htable_create();

void htable_destroy(htable_t *ht);

int htable_put(const char *str, int len);

ht_slist_node_t* htable_get(const char *str, int len);

#endif