#include <stdlib.h>
#include <string.h>
#include "oracle.h"

Oracle* oracle_create() {
    Oracle *orc = (Oracle *)malloc(sizeof(Oracle));
    memset(orc, 0, sizeof(Oracle));
    orc->min_len = INT32_MAX;
    orc->nsize = ORC_DEFAULT_NODE_NUM;
    orc->nodes = (orc_slist_node_t *)malloc(sizeof(orc_slist_node_t) * ORC_DEFAULT_NODE_NUM);
    memset(orc->nodes, 0, sizeof(orc_slist_node_t) * ORC_DEFAULT_NODE_NUM);
    return orc;
}

Oracle* oracle_create_ex(const char *patterns[], int pnum) {
    Oracle *orc = oracle_create();
    for (int i = 0; i < pnum; i++) {
        oracle_insert(orc, patterns[i], strlen(patterns[i]));
    }
    oracle_build(orc);
    return orc;
}

void oracle_destroy(Oracle *orc) {
    free(orc->lists);
    for (int i = 0; i < orc->pnum; i++) {
        free(orc->nodes[i].str);
    }
    free(orc->nodes);
    free(orc->fids);
    trie_destroy(orc->trie);
    free(orc);
}

int oracle_insert(Oracle *orc, const char *p, int plen) {
    if (plen <= 0) {
        return 0;
    }
    int nsize = orc->nsize;
    int pnum = orc->pnum;
    if (nsize <= pnum) {
        orc_slist_node_t *nodes = (orc_slist_node_t *)realloc(orc->nodes, sizeof(orc_slist_node_t) * nsize * 2);
        if (NULL == nodes) {
            return -1;
        }
        memset(nodes + nsize, 0, sizeof(orc_slist_node_t) * nsize);
        orc->nodes = nodes;
        orc->nsize = nsize * 2;
    }
    orc_slist_node_t* node = &orc->nodes[pnum];
    node->len = plen;
    node->str = (char *)malloc(plen + 1);
    strncpy(node->str, p, plen + 1);
    ++orc->pnum;
    if (orc->min_len > plen) {
        orc->min_len = plen;
    }
    return 0;
}

static void oracle_build_trie(Oracle *orc);

void oracle_build(Oracle *orc) {
    oracle_build_trie(orc);
    int state_num = orc->trie->state_num;
    int *supply = (int *) malloc(sizeof(int) * state_num);
    memset(supply, 0, sizeof(int) * state_num);
    trie_make_bfs(orc->trie);
    supply[0] = -1;
    for (int i = 1; i < state_num; i++) {
        TrieState* state = orc->trie->bfs_states[i];
        int sp = state->parent;
        int state_id = 0;
        while ((sp = supply[sp]) != -1) {
            state_id = trie_get_trans(orc->trie, sp, state->c);
            if (state_id == -1) {
                trie_set_trans(orc->trie, sp, state->id, state->c);
            } else {
                supply[state->id] = state_id;
                break;
            }
        }
    }
    free(supply);
}

void oracle_search(const Oracle *orc, const char *s, int slen, match_result_t *result) {
    // i表示窗口位置，j表示窗口内字符位置
    int min_len = orc->min_len;
    for (int i = 0, j = min_len - 1; i <= slen - min_len; i = i + j + 1, j = min_len - 1) {
        int state_id = 0;
        while ((state_id = trie_get_trans(orc->trie, state_id, s[i + j])) != -1) {
            if (j == 0) {
                TrieState *state = &orc->trie->states[state_id];
                orc_slist_node_t *node = orc->lists[orc->fids[state->id]].first;
                while (node != NULL) {
                    int pos = i + min_len - node->len;
                    if (pos >= 0 && memcmp(s + pos, node->str, node->len - min_len) == 0) {
                        match_result_append(result, node->len, pos);
                    }
                    node = node->next;
                }
                break;
            }
            j--;
        }
    }
}

static void oracle_build_trie(Oracle *orc) {
    int nfids[orc->pnum];
    char reverse[orc->min_len];
    orc->trie = trie_create(STTABLE_TYPE_HASHT);
    orc->fids = (int*)malloc(sizeof(int) * orc->min_len * orc->pnum);
    memset(orc->fids, -1, sizeof(int) * orc->min_len * orc->pnum);
    for (int i = 0; i < orc->pnum; i++) {
        orc_slist_node_t *node = &orc->nodes[i];
        // 字符串后缀反转
        for (int j = node->len - 1, k = 0; j >= node->len - orc->min_len; j--, k++) {
            reverse[k] = node->str[j];
        }
        TrieState *state = trie_insert(orc->trie, reverse, orc->min_len);
        if (orc->fids[state->id] == -1) {
            orc->fids[state->id] = orc->trie->fin_state_num - 1;
        }
        nfids[i] = orc->fids[state->id];
    }
    // 构造每个终止状态对应的字符串链表
    orc->lists = (orc_slist_t*)calloc(orc->trie->fin_state_num, sizeof(orc_slist_t));
    for (int i = 0; i < orc->pnum; i++) {
        orc_slist_node_t *node = &orc->nodes[i];
        orc_slist_t *list = &orc->lists[nfids[i]];
        node->next = list->first;
        list->first = node;
    }
}