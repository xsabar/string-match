#ifndef _HORSPOOL_H
#define _HORSPOOL_H

#include "trie.h"

typedef struct {
    Trie* trie;     // trie树
    int size;       // 位移表大小
    int base;       // size = 1 << base
    int *shift;     // 位移表
    int min_len;    // 模式串最小长度
    int block_size; // 字符块大小
} Horspool;

/**
 * @brief 创建
 * 
 * @param block_size 块大小
 * @return Horspool* 
 */
Horspool* horspool_create(int block_size);

/**
 * @brief 基于模式串集合创建
 * 
 * @param patterns 模式串集合
 * @param pnum     模式串数量
 * @param block_size 块大小
 * @return Horspool* 
 */
Horspool* horspool_create_ex(const char **patterns, int pnum, int block_size);

/**
 * @brief 销毁
 * 
 * @param hsp 
 */
void horspool_destroy(Horspool *hsp);

/**
 * @brief 插入模式串
 * 
 * @param hsp  Horspool指针
 * @param p    模式串
 * @param plen 模式串长度
 */
void horspool_insert(Horspool *hsp, const char *p, int plen);

/**
 * @brief 构建Horspool
 * 
 * @param hsp 
 */
void horspool_build(Horspool *hsp);

/**
 * @brief 多模式串horspool匹配算法
 * 
 * @param hsp    Horspool指针
 * @param s      字符串
 * @param slen   字符串长度
 * @param result 匹配结果
 */
void horspool_trie_search(const Horspool *hsp, const char *s, int slen, match_result_t *result);

#endif