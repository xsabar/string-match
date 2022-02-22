#include <stdlib.h>
#include <string.h>
#include "ac.h"

AC* ac_create(ACLevel level) {
    AC *ac = (AC *)malloc(sizeof(AC));
    if (level == AC_LEVEL_FULL) {
        ac->trie = trie_create(STTABLE_TYPE_ARRAY);
    } else {
        ac->trie = trie_create(STTABLE_TYPE_HASHT);
    }
    ac->level = level;
    ac->suff = NULL;
    ac->next = NULL;
    return ac;
}

AC* ac_create_ex(const char *patterns[], int pnum, ACLevel level) {
    AC *ac = (AC *)malloc(sizeof(AC));
    if (level == AC_LEVEL_FULL) {
        ac->trie = trie_create_ex(patterns, pnum, STTABLE_TYPE_ARRAY);
    } else {
        ac->trie = trie_create_ex(patterns, pnum, STTABLE_TYPE_HASHT);
    }
    ac->level = level;
    ac->suff = NULL;
    ac->next = NULL;
    ac_build(ac);
    return ac;
}

void ac_destroy(AC *ac) {
    trie_destroy(ac->trie);
    free(ac->suff);
    free(ac->next);
    free(ac);
}

int ac_insert(AC *ac, const char *p, int plen) {
    return trie_insert(ac->trie, p, plen);
}

/**
 * @brief 拷贝状态转移
 * 
 * @param ac    树指针
 * @param from_id 源状态ID
 * @param to_id   目标状态ID
 */
static void ac_copy_stt(AC *ac, int from_id, int to_id) {
    Trie *trie = ac->trie;
    int state_id = trie->states[from_id].first;
    TrieState *state = NULL;
    // AC自动机构造产生的新的状态转移
    sttable_copy(ac->trie->sttbl, from_id, to_id);
    // trie树自身的状态转移
    while (state_id != 0) {
        state = &trie->states[state_id];
        trie_set_trans(trie, to_id, state_id, state->c);
        state_id = state->next;
    }
}

static void ac_build_full(AC *ac) {
    Trie *trie = ac->trie;
    int *bfs_ids = trie_make_bfs(trie);
    // 初始状态满足AC条件
    memset(trie->sttbl->ast.stt, 0, sizeof(int) * CHARSET_SIZE);
    // 层次遍历使各状态依次满足AC条件
    // 每一层的状态都会暂时和下一层断开连接
    for (int i = 1; i < trie->state_num; i++) {
        int state_id = bfs_ids[i];
        TrieState *state = &trie->states[state_id];
        TrieState *parent = &trie->states[state->parent];
        // 找到当前子串的最长后缀模式串
        int k = trie_get_trans(trie, state->parent, state->c);
        if (trie->states[k].is_fin) {
            ac->suff[state_id] = k;
        } else {
            ac->suff[state_id] = ac->suff[k];
        }
        // 重新建立连接
        trie_set_trans(trie, state->parent, state_id, state->c);
        // 拷贝状态转移
        ac_copy_stt(ac, k, state_id);
    }
    free(bfs_ids);
}

static void ac_build_part(AC *ac) {
    Trie *trie = ac->trie;
    int *bfs_ids = trie_make_bfs(trie);
    for (int i = 1; i < trie->state_num; i++) {
        int state_id = bfs_ids[i];
        TrieState *state = &trie->states[state_id];
        int j = ac->next[state->parent];
        while (j != -1) {
            int k = trie_get_trans(trie, j, state->c);
            if (k != -1) {
                ac->next[state_id] = k;
                // 找到当前子串的最长后缀模式串
                if (trie->states[k].is_fin) {
                    ac->suff[state_id] = k;
                } else {
                    ac->suff[state_id] = ac->suff[k];
                }
                break;
            } else {
                j = ac->next[j];
            }
        }
    }
    free(bfs_ids);
}

void ac_build(AC *ac) {
    ac->suff = (int *)malloc(sizeof(int) * ac->trie->state_num);
    memset(ac->suff, -1, sizeof(int) * ac->trie->state_num);
    if (ac->level == AC_LEVEL_FULL) {
        ac_build_full(ac);
    } else {
        ac->next = (int *)malloc(sizeof(int) * ac->trie->state_num);
        memset(ac->next, 0, sizeof(int) * ac->trie->state_num);
        ac->next[0] = -1;
        ac_build_part(ac);
    }
}

static void ac_search_full(const AC *ac, const char *s, int slen, match_result_t *result) {
    for (int i = 0, state_id = 0; i < slen; i++) {
        state_id = trie_get_trans(ac->trie, state_id, s[i]);
        TrieState *state = &ac->trie->states[state_id];
        if (state->is_fin) {
            match_result_append(result, state->depth, i - state->depth + 1);
        }
        for (int id = ac->suff[state_id]; id != -1; id = ac->suff[id]) {
            state = &ac->trie->states[id];
            match_result_append(result, state->depth, i - state->depth + 1);
        }
    }
}

static void ac_search_part(const AC *ac, const char *s, int slen, match_result_t *result) {
    for (int i = 0, state_id = 0, target = 0; i < slen;) {
        if (state_id == -1) {
            ++i;
            state_id = 0;
        } else if ((target = trie_get_trans(ac->trie, state_id, s[i])) != -1) {
            ++i;
            state_id = target;
            TrieState *state = &ac->trie->states[target];
            if (state->is_fin) {
                match_result_append(result, state->depth, i - state->depth);
            }
            for (int id = ac->suff[target]; id != -1; id = ac->suff[id]) {
                state = &ac->trie->states[id];
                match_result_append(result, state->depth, i - state->depth);
            }
        } else {
            state_id = ac->next[state_id];
        }
    }
}

void ac_search(const AC *ac, const char *s, int slen, match_result_t *result) {
    if (ac->level == AC_LEVEL_FULL) {
        ac_search_full(ac, s, slen, result);
    } else {
        ac_search_part(ac, s, slen, result);
    }
}
