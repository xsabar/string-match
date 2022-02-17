#ifndef _ORACLE_H
#define _ORACLE_H

#include "trie.h"

#define ORC_DEFAULT_NODE_NUM DEFAULT_STATE_NUM

typedef struct _orc_slist_node_s {
    int len;   // 长度
    char *str; // 模式串
    struct _orc_slist_node_s *next;
} orc_slist_node_t;

typedef struct {
    orc_slist_node_t *first;
} orc_slist_t;

typedef struct {
    Trie *trie;  // trie自动机
    int min_len; // 最小模式串长度
    int nsize;   // 字符串节点数组大小
    int pnum;    // 模式串数量
    int *fids;   // 终止状态ID数组
    orc_slist_t *lists;      // 终止状态对应的字符串链表
    orc_slist_node_t *nodes; // 字符串节点数组
    STTableType sttype;
} Oracle;

/**
 * @brief 创建oracle自动机
 * 
 * @return Oracle* 
 */
Oracle* oracle_create(STTableType sttype);

/**
 * @brief 从模式串集合中创建oracle自动机
 * 
 * @param patterns 模式串集合
 * @param pnum     模式串数量
 * @return Oracle* 
 */
Oracle* oracle_create_ex(const char *patterns[], int pnum, STTableType sttype);

/**
 * @brief destroy oracle automation
 * 
 * @param orc oracle自动机指针
 * @return Oracle*
 */
void oracle_destroy(Oracle *orc);

/**
 * @brief 插入模式串
 * 
 * @param orc  自动机指针
 * @param p    模式串
 * @param plen 模式串长度
 * @return int 0:成功 -1:失败
 */
int oracle_insert(Oracle *orc, const char *p, int plen);

/**
 * @brief 构建oracle自动机
 * 
 * @param orc 自动机指针
 */
void oracle_build(Oracle *orc);

/**
 * @brief set backward oracle match
 * 
 * @param orc    oracle自动机指针
 * @param s      字符串
 * @param slen   字符串长度
 * @param result 匹配结果指针
 */
void oracle_search(const Oracle *orc, const char *s, int slen, match_result_t *result);

#endif