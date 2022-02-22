#ifndef _AC_H
#define _AC_H

#include "trie.h"

typedef enum {
    AC_LEVEL_PART, // 不完全AC自动机
    AC_LEVEL_FULL  // 完全AC自动机
} ACLevel;

typedef struct {
    Trie *trie;
    ACLevel level;
    int *suff; // 状态回溯表，当前状态的最长后缀模式串对应的状态
    int *next; // 不完全自动机，失配状态跳转表
} AC;

/**
 * @brief 创建AC自动机
 * 
 * @param ACLevel 自动机等级
 * @return AC* 
 */
AC* ac_create(ACLevel level);

/**
 * @brief 创建AC自动机
 * 
 * @param patterns 模式串集合
 * @param pnum     模式串个数
 * @param ACLevel  自动机等级
 * @return AC* 
 */
AC* ac_create_ex(const char *patterns[], int pnum, ACLevel level);

/**
 * @brief 销毁AC自动机
 * 
 * @param ac 
 */
void ac_destroy(AC *ac);

/**
 * @brief 插入模式串
 * 
 * @param ac   自动机指针
 * @param p    模式串
 * @param plen 模式串长度
 * @return int 终止状态id
 */
int ac_insert(AC *ac, const char *p, int plen);

/**
 * @brief 构建AC自动机
 * 
 * @param ac 
 */
void ac_build(AC *ac);

/**
 * @brief AC自动机字符串匹配
 * 
 * @param ac 自动机指针
 * @param s  字符串
 * @param slen 字符串长度
 * @param result 匹配结果
 */
void ac_search(const AC *ac, const char *s, int slen, match_result_t *result);

#endif