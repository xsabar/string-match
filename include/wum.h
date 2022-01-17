#ifndef _WUM_H
#define _WUM_H

#include "smio.h"

#define WUM_DEFAULT_PATTERN_NUM 16

/**
 * @brief 字符串节点
 */
typedef struct _wum_slist_node_s {
    int len;   // 字符串长度
    char *str; // 字符串
    struct _wum_slist_node_s *next; // 指向下个节点
} wum_slist_node_t;

/**
 * @brief 字符串链表
 */
typedef struct {
    wum_slist_node_t *first; // 指向首元素
} wum_slist_t;

/**
 * @brief 字符串哈希表
 * 静态哈希表，容量通过模式串数量计算
 */
typedef struct {
    int cap;  // 容量
    int size; // 元素数量
    int lnum; // 非空链表数
    int base; // cap = 1 << base
    wum_slist_t *lists;      // 链表集合
} wum_htable_t;

typedef struct {
    int size; // shift表大小
    int base; // size = 1 << base
    int *shift; // 位移表
} wum_shift_t;

/**
 * @brief WuManber结构体
 */
typedef struct {
    int nsize;      // 模式串表大小
    int pnum;       // 模式串数量
    int min_len;    // 最小模式串长度
    int block_size; // 字符块大小
    wum_shift_t  stbl; // 位移表
    wum_htable_t htbl; // 哈希表
    wum_slist_node_t *nodes; // 模式串表
} Wum;

/**
 * @brief 创建
 * 
 * @param block_size 
 * @return Wum* 
 */
Wum* wum_create(int block_size);

/**
 * @brief 从模式串集合中创建
 * 
 * @param patterns   模式串集合
 * @param pnum       模式串数量
 * @param block_size 字符块大小
 * @return Wum* 
 */
Wum* wum_create_ex(const char **patterns, int pnum, int block_size);

/**
 * @brief 销毁
 * 
 * @param wum 
 */
void wum_destroy(Wum *wum);

/**
 * @brief 插入模式串
 * 
 * @param wum  Wum对象
 * @param p    模式串
 * @param plen 模式串长度
 * @return int 0:成功 -1:失败
 */
int wum_insert(Wum *wum, const char *p, int plen);

/**
 * @brief 构建
 * 
 * @param wum 
 */
void wum_build(Wum *wum);

/**
 * @brief wumaber匹配算法
 * 
 * @param wum    Wum对象
 * @param s      字符串
 * @param slen   字符串长度
 * @param result 匹配结果
 */
void wum_search(const Wum *wum, const char *s, int slen, match_result_t *result);

#endif