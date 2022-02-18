#ifndef _DAT_H
#define _DAT_H

#include "smio.h"
#include "trie.h"

#define DAT_NODE_DEFAULT_NUM 1024
#define DAT_NODE_INCREMT_NUM 1024
#define DAT_TAIL_DEFAULT_LEN 1024
#define DAT_TAIL_INCREMT_LEN 1024
// 模式串添加结尾字符，避免一个模式串是另一个模式串的前缀
#define DAT_STOP_CHAR '#'

/**
 * @brief 双数组trie树节点结构
 */
typedef struct {
    int base;  // 转移基数
    int check; // 来源节点
} dat_node_t;

/**
 * @brief reduced-trie树字符串后缀结构
 */
typedef struct {
    int len;   // tail长度
    int pos;   // 空闲位置
    char *str; // 后缀字符串
} dat_tail_t;

/**
 * @brief 双数组trie数结构
 * fid: 源节点ID
 * c:   转移字符
 * tid: 目标节点ID
 * tid = nodes[fid].base + c
 * nodes[tid].check = fid
 */
typedef struct {
    int cap;
    dat_node_t *nodes; // trie树节点数组
    dat_tail_t tail;   // 字符串后缀
} DATrie;

/**
 * @brief 创建空的双数组trie树
 * 
 * @return DATrie* 
 */
DATrie* dat_create();

/**
 * @brief 给定模式串集合，创建trie树
 * 
 * @param patterns 模式串集合
 * @param pnum     模式串数量
 * @return DATrie* 树指针
 */
DATrie* dat_create_ex(const char **patterns, int pnum);

/**
 * @brief 销毁trie树
 * 
 * @param dat 树指针
 */
void dat_destroy(DATrie *dat);

/**
 * @brief 插入模式串
 * 
 * @param dat  树指针
 * @param p    模式串
 * @param plen 模式串长度
 */
void dat_insert(DATrie *dat, const char *p, int plen);

void dat_build(DATrie *dat);

/**
 * @brief 删除模式串
 * 
 * @param dat  树指针
 * @param p    模式串
 * @param plen 模式串长度
 */
void dat_delete(DATrie *dat, const char *p, int plen);

/**
 * @brief 搜索字符串
 * 
 * @param dat    树指针
 * @param s      字符串
 * @param slen   字符串长度
 * @param result 匹配结果
 */
void dat_search(const DATrie *dat, const char *s, int slen, match_result_t *result);

#endif