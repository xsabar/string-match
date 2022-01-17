#ifndef _SHIFT_H
#define _SHIFT_H

#include "smio.h"
#include "bit_array.h"

#define SHIFT_MAX_PATTERN_NUM 128
#define SHIFT_MAX_PATTERN_LEN 16

typedef struct {
    int len;
    char str[SHIFT_MAX_PATTERN_LEN];
} _snfa_pattern_t;

typedef struct {
    int pnum;    // 模式串数量
    int max_len; // 最大模式串长度
    bit_array_t int_mask;           // 初始状态位掩码
    bit_array_t fin_mask;           // 终止状态位掩码
    bit_array_t mask[CHARSET_SIZE]; // 字符位掩码表
    _snfa_pattern_t patterns[SHIFT_MAX_PATTERN_NUM]; // 模式串数组
} ShiftNFA;

/**
 * @brief 创建
 * 
 * @return ShiftNFA* 
 */
ShiftNFA* shift_nfa_create();

/**
 * @brief 从模式串集合中创建
 * 
 * @param patterns 
 * @param pnum 
 * @return ShiftNFA* 
 */
ShiftNFA* shift_nfa_create_ex(const char **patterns, int pnum);

/**
 * @brief 销毁
 * 
 * @param snfa 
 */
void shift_nfa_destroy(ShiftNFA* snfa);

/**
 * @brief 插入模式串
 * 
 * @param snfa 
 * @param p 
 * @param plen 
 * @return int 0:成功 -1:失败
 */
int shift_nfa_insert(ShiftNFA *snfa, const char *p, int plen);

/**
 * @brief 构建ShiftNFA
 * 
 * @param snfa 
 */
void shift_nfa_build(ShiftNFA *snfa);

/**
 * @brief 多模式串下shift and匹配算法
 * 
 * @param snfa   ShiftNFA指针
 * @param s      字符串
 * @param slen   字符串长度
 * @param result 匹配结果
 */
void shift_nfa_search(const ShiftNFA *snfa, const char *s, int slen, match_result_t* result);

#endif