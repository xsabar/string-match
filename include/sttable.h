#ifndef _STTABLE_H
#define _STTABLE_H

#define DEFAULT_STATE_NUM 16
#define STTABLE_DEFAULT_ARRAY_SIZE (CHARSET_SIZE * DEFAULT_STATE_NUM)
#define STTABLE_HASHT_CAP_BASE 4 // base=4,cap=2^4=16
#define STTABLE_HASHT_DEFAULT_THRD  (1 << STTABLE_HASHT_CAP_BASE) * 3 / 4 // (cap * load_factor)

/**
 * @brief 状态转移表类型
 */
typedef enum {
    STTABLE_TYPE_ARRAY, // 数组实现状态转移表
    STTABLE_TYPE_HASHT, // 哈希表实现状态转移表
} STTableType;

/**
 * @brief 状态转移链表节点
 */
typedef struct _stlist_node_s {
    int fid; // 源状态id
    char c;  // 转移字符
    int tid; // 目标状态id
    struct _stlist_node_s *next;
} stlist_node_t;

/**
 * @brief 状态转移链表
 */
typedef struct {
    stlist_node_t *first;
} stlist_t;

/**
 * @brief 状态转移哈希表
 */
struct _sttable_hasht_s {
    int cap;  // 容量
    int thrd; // 阈值
    int size; // 大小（元素数量）
    int lnum; // 非空链表数
    int base; // cap = 1 << base
    stlist_t *lists;      // 哈希表
    stlist_node_t *nodes; // 元素集合
};

struct _sttable_array_s {
    int size;
    int *stt;
};

typedef struct {
    STTableType type; // 状态转移表类型
    union {
        struct _sttable_array_s ast; // 状态转移-数组
        struct _sttable_hasht_s hst; // 状态转移-hash表
    };
} sttable_t;

/**
 * @brief 创建状态转移表
 * 
 * @param tbl 表指针
 */
sttable_t* sttable_create(STTableType type);

/**
 * @brief 销毁状态转移表
 * 
 * @param tbl 表指针
 */
void sttable_destroy(sttable_t *tbl);

/**
 * @brief 设置状态转移，如果已存在则修改，不存在则创建新转移
 * 
 * @param tbl 表指针
 * @param fid 源状态
 * @param c   转移字符
 * @param tid 目标状态
 * 
 * @return -1:失败，0:成功
 */
int sttable_set(sttable_t *tbl, int fid, char c, int tid);

/**
 * @brief 获取状态转移
 * 
 * @param tbl 表指针
 * @param id  源状态
 * @param c   转移字符
 * @return int 目标状态，-1表示不存在
 */
int sttable_get(sttable_t *tbl, int id, char c);

/**
 * @brief 拷贝状态转移，仅限状态转移数组实现
 * 
 * @param tbl 表指针
 * @param fid 源状态
 * @param tid 目标状态
 */
void sttable_copy(sttable_t *tbl, int fid, int tid);

#endif