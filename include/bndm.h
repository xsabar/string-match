#ifndef _BNDM_H
#define _BNDM_H

#include "smio.h"
#include "bit_array.h"

#define BNDM_MAX_PATTERN_NUM 128
#define BNDM_MAX_PATTERN_LEN 16

typedef struct {
    int len;
    char str[BNDM_MAX_PATTERN_LEN];
} _bndm_pattern_t;

typedef struct {
    int pnum;    // 模式串数量
    int min_len; // 最小模式串长度
    bit_array_t int_mask;           // 初始状态位掩码
    bit_array_t fin_mask;           // 终止状态位掩码
    bit_array_t mask[CHARSET_SIZE]; // 字符位掩码表
    _bndm_pattern_t patterns[BNDM_MAX_PATTERN_NUM]; // 模式串数组
} BndmNFA;

/**
 * @brief 创建空的BNDM NFA
 * 
 * @return BndmNFA* 
 */
BndmNFA* bndm_nfa_create();

/**
 * @brief 从模式串集合创建BNDM NFA
 * 
 * @param patterns 模式串集合
 * @param pnum     模式串数量
 * @return BndmNFA* 
 */
BndmNFA* bndm_nfa_create_ex(const char **patterns, int pnum);

/**
 * @brief 销毁BNDM NFA
 * 
 * @param nfa 
 */
void bndm_nfa_destroy(BndmNFA *nfa);

/**
 * @brief 插入模式串
 * 
 * @param nfa   BndmNFA指针
 * @param p     字符串
 * @param plen  字符串长度
 * @return int  0:成功 -1:失败
 */
int bndm_nfa_insert(BndmNFA *nfa, const char *p, int plen);

/**
 * @brief 构建BndmNFA
 * 
 * @param nfa 
 */
void bndm_nfa_build(BndmNFA *nfa);

/**
 * @brief 在bndm nfa上搜索
 * 
 * @param nfa    bndm nfa指针
 * @param s      字符串
 * @param slen   字符串长度
 * @param result 匹配结果
 */
void bndm_nfa_search(const BndmNFA *nfa, const char *s, int slen, match_result_t *result);

#endif