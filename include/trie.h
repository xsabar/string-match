#ifndef _TRIE_H
#define _TRIE_H

#include "smio.h"
#include "sttable.h"

/**
 * @brief trie树节点
 */
typedef struct _trie_state_s {
    char c;        // 转移字符
    int8_t is_fin; // 终止状态标记
    short depth;   // 在树中的深度
    int id;        // 状态id（状态表索引）
    // 左孩子-右兄弟表示法
    int first;     // 子节点id
    int next;      // 兄弟节点id
} TrieState;

/**
 * @brief trie树
 */
typedef struct _trie_s {
    int size;          // 状态表大小
    int state_num;     // 状态数
    TrieState *states; // 状态表
    int fin_state_num; // 终止状态数（模式串数）
    int depth;         // 树深度
    sttable_t *sttbl;  // 状态转移表
} Trie;

/**
 * @brief 创建trie树
 * 
 * @return Trie* 树指针
 */
Trie* trie_create(STTableType sttype);

/**
 * @brief 创建trie树
 * 
 * @param patterns 模式串集合
 * @param pnum     模式串数量
 * @return Trie*   树指针
 */
Trie* trie_create_ex(const char **patterns, int pnum, STTableType sttype);

/**
 * @brief 销毁trie树
 * @param trie 树指针
 */
void trie_destroy(Trie *trie);

/**
 * @brief 插入模式串
 * 
 * @param trie 树指针
 * @param p    模式串
 * @param plen 模式串长度
 * @return TrieState* 终止状态指针
 */
TrieState* trie_insert(Trie *trie, const char *p, int plen);

/**
 * @brief 插入反转模式串
 * 
 * @param trie 树指针
 * @param p    模式串
 * @param plen 模式串长度
 * @return TrieState* 终止状态指针
 */
TrieState* trie_insert_reverse(Trie *trie, const char *p, int plen);

/**
 * @brief 设置状态转移
 * 
 * @param trie    树指针
 * @param from_id 源状态ID
 * @param to_id   目标状态ID
 * @param c       转移字符
 */
void trie_set_trans(Trie *trie, int from_id, int to_id, char c);

/**
 * @brief 状态转移
 * 
 * @param trie     树指针
 * @param state_id 状态ID
 * @param c        转移字符
 * @return int     转移状态ID，-1表示不存在
 */
int trie_get_trans(const Trie *trie, int state_id, char c);

/**
 * @brief trie树广度优先遍历
 * 
 * @param trie  树指针
 * @param parents 父节点数组
 */
TrieState** trie_make_bfs(Trie* trie, int **parents);

/**
 * @brief trie树多模匹配
 * 
 * @param trie 树指针
 * @param s 字符串
 * @param slen 字符串长度
 * @param result 匹配结果
 */
void trie_search(const Trie *trie, const char *s, int slen, match_result_t* result);

#endif