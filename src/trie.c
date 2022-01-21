#include <stdlib.h>
#include <string.h>
#include "trie.h"

Trie* trie_create(STTableType sttype) {
    Trie* trie = (Trie*)malloc(sizeof(Trie));
    memset(trie, 0, sizeof(Trie));
    // 初始化状态表
    trie->size = DEFAULT_STATE_NUM;
    trie->state_num = 1;
    trie->sttbl = sttable_create(sttype);
    trie->states = (TrieState*)calloc(DEFAULT_STATE_NUM, sizeof(TrieState));
    memset(trie->states, 0, sizeof(TrieState) * DEFAULT_STATE_NUM);
    return trie;
}

Trie* trie_create_ex(const char **patterns, int pnum, STTableType sttype) {
    Trie* trie = trie_create(sttype);
    for (int i = 0; i < pnum; i++) {
        trie_insert(trie, patterns[i], strlen(patterns[i]));
    }
    return trie;
}

void trie_destroy(Trie *trie) {
    free(trie->states);
    sttable_destroy(trie->sttbl);
    free(trie->bfs_states);
    free(trie);
}

static int trie_expand(Trie *trie);

static int trie_set_fin_state(Trie *trie, TrieState *state, const char *p, int plen);

static TrieState* trie_insert_new_state(Trie *trie, int act_state_id, int new_state_id, char c);

static TrieState* _trie_insert(Trie *trie, const char *p, int plen, int start, int stop, int step);

TrieState* trie_insert(Trie *trie, const char *p, int plen) {
    return _trie_insert(trie, p, plen, 0, plen, 1);
}

TrieState* trie_insert_reverse(Trie *trie, const char *p, int plen) {
    return _trie_insert(trie, p, plen, plen - 1, -1, -1);
}

void trie_set_trans(Trie *trie, int from_id, int to_id, char c) {
    sttable_set(trie->sttbl, from_id, c, to_id);
}

int trie_get_trans(const Trie *trie, int state_id, char c) {
    return sttable_get(trie->sttbl, state_id, c);
}

void trie_make_bfs(Trie* trie) {
    if (trie->bfs_states != NULL) {
        return;
    }
    int top = 0;
    int next = 1;
    TrieState **states = (TrieState **)malloc(sizeof(TrieState *) * trie->state_num);
    memset(states, 0, sizeof(TrieState *) * trie->state_num);
    states[0] = &trie->states[0];
    while (next < trie->state_num) {
        TrieState *state = states[top]->first;
        while (state != NULL) {
            states[next++] = state;
            state = state->next;
        }
        ++top;
    }
    trie->bfs_states = states;
}

void trie_search(const Trie *trie, const char *s, int slen, match_result_t* result) {
    const TrieState* state = NULL;
    for (int i = 0; i < slen; i++) {
        int state_id = 0;
        int j = i;
        while (j < slen && (state_id = trie_get_trans(trie, state_id, s[j])) != -1) {
            state = &trie->states[state_id];
            if (state->is_fin) {
                if (match_result_append(result, state->depth, i) != 0) {
                    return;
                }
            }
            j++;
        }
    }
}

/**
 * @brief trie树分配更多的存储空间
 * 
 * @param trie Trie指针
 * @return int -1:内存不足，扩展失败 0:成功
 */
static int trie_expand(Trie *trie) {
    int size = trie->size * 2;
    TrieState *states = (TrieState*)realloc(trie->states, size * sizeof(TrieState));
    if (NULL == states) {
        return -1;
    }
    memset(states + trie->size, 0, trie->size * sizeof(TrieState)); // 初始化新分配空间
    // 地址发生变化，更新状态指针
    if (states != trie->states) {
        int64_t shift = (int64_t)states - (int64_t)trie->states;
        for (int i = 0; i < trie->state_num; i++) {
            if (states[i].parent != NULL) {
                states[i].parent = (TrieState *)((char *)states[i].parent + shift);
            }
            if (states[i].first != NULL) {
                states[i].first = (TrieState *)((char *)states[i].first + shift);
            }
            if (states[i].next != NULL) {
                states[i].next = (TrieState *)((char *)states[i].next + shift);
            }
        }
    }
    trie->states = states;
    trie->size = size;
    return 0;
}

/**
 * @brief 插入新状态
 * 
 * @param trie 树指针
 * @param act_state_id 激活状态
 * @param new_state_id 转移状态
 * @param c            转移字符
 * @return TrieState* 转移状态指针
 */
static TrieState* trie_insert_new_state(Trie *trie, int act_state_id, int new_state_id, char c) {
    TrieState *act_state = &trie->states[act_state_id];
    TrieState *new_state = &trie->states[new_state_id];
    new_state->id = new_state_id;
    new_state->depth = act_state->depth + 1;
    new_state->c = c;
    new_state->parent = act_state;
    new_state->next = act_state->first;
    act_state->first = new_state;
    trie_set_trans(trie, act_state_id, new_state_id, c);
    // 更新树深度
    if (trie->depth < new_state->depth) {
        trie->depth = new_state->depth;
    }
    return new_state;
}

/**
 * @brief 插入模式串
 * 
 * @param trie 树指针
 * @param p    模式串
 * @param plen 模式串长度
 * @param start 开始位置
 * @param stop  结束位置
 * @param step  步长
 * @return TrieState* 状态指针
 */
static TrieState* _trie_insert(Trie *trie, const char *p, int plen, int start, int stop, int step) {
    if (plen <= 0) {
        return NULL;
    }
    int act_state_id = 0;
    int new_state_id = 0;
    for (int i = start; i != stop; i += step) {
        new_state_id = trie_get_trans(trie, act_state_id, p[i]);
        if (new_state_id == -1) {
            if (trie->size <= trie->state_num) {
                // 内存扩展可能会使地址发生变化
                if (trie_expand(trie) != 0) {
                    return NULL;
                }
            }
            new_state_id = trie->state_num++;
            trie_insert_new_state(trie, act_state_id, new_state_id, p[i]);
        } 
        act_state_id = new_state_id;
    }
    TrieState *state = &trie->states[act_state_id];
    if (!state->is_fin) {
        state->is_fin = 1;
        state->fid = trie->fin_state_num;
        ++trie->fin_state_num;
    }
    return &trie->states[act_state_id];
}