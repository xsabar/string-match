#include <stdlib.h>
#include <string.h>
#include "horspool.h"
#include "xssm.h"

Horspool* horspool_create(int block_size) {
	Horspool *hsp = (Horspool *)malloc(sizeof(Horspool));
	memset(hsp, 0, sizeof(Horspool));
	hsp->trie = trie_create(STTABLE_TYPE_DBARR);
	hsp->min_len = INT32_MAX;
	hsp->block_size = block_size;
	return hsp;
}

Horspool* horspool_create_ex(const char **patterns, int pnum, int block_size) {
	Horspool *hsp = horspool_create(block_size);
	for (int i = 0; i < pnum; i++) {
		horspool_insert(hsp, patterns[i], strlen(patterns[i]));
	}
	horspool_build(hsp);
	return hsp;
}

void horspool_destroy(Horspool *hsp) {
	trie_destroy(hsp->trie);
	free(hsp->shift);
	free(hsp);
}

void horspool_insert(Horspool *hsp, const char *p, int plen) {
	if (plen > 0) {
		trie_insert_reverse(hsp->trie, p, plen);
		if (hsp->min_len > plen) {
			hsp->min_len = plen;
		}
		hsp->size += plen;
	}
}

/**
 * @brief 计算字符串哈希值
 * 
 * @param str  字符串
 * @param len  字符串长度
 * @param base 基数
 * @return int 哈希值
 */
static int horspool_hash(const char *str, int len, int base) {
    uint64_t hash = 0;
    for (int i = 0; i < len; i++) {
        hash = hash * 31 + str[i];
    }
    return (hash ^ (hash >> base)) & ((1 << base) - 1);
}

static void horspool_build_init(Horspool *hsp) {
	if (hsp->block_size >= hsp->min_len) {
		hsp->block_size = hsp->min_len;
	}
    while ((hsp->size >> (hsp->base + 1)) != 0) {
        ++hsp->base;
    }
    if (hsp->size > (1 << hsp->base)) {
        ++hsp->base;
    }
	hsp->size = 1 << hsp->base;
	hsp->shift = (int *)calloc(hsp->size, sizeof(int));
	for (int i = 0; i < hsp->size; i++) {
		hsp->shift[i] = hsp->min_len - hsp->block_size + 1;
	}
}

static void horspool_build_shift(Horspool *hsp, TrieState *state, const char *pattern) {
	int shift = 0;
	int pos = 0;
	for (int j = state->depth - hsp->min_len + hsp->block_size - 1; j < state->depth - 1; j++) {
		shift = state->depth - j - 1;
		pos = horspool_hash(pattern + j - hsp->block_size + 1, hsp->block_size, hsp->base);
		if (hsp->shift[pos] > shift) {
			hsp->shift[pos] = shift;
		}
	}
}

void horspool_build(Horspool *hsp) {
	horspool_build_init(hsp);
	TrieState *states = hsp->trie->states;
	int dfs[hsp->trie->depth + 1];
	char pattern[hsp->trie->depth];
	TrieState *state = NULL;
	int top = 0;
	memset(dfs, 0, sizeof(dfs));
	// 深度优先搜索
	int state_id = states[dfs[top]].first;
	while (state_id != 0) {
		dfs[++top] = state_id;
		state = &states[state_id];
		pattern[hsp->trie->depth - top - 1] = state->c;
		if (state->is_fin) {
			horspool_build_shift(hsp, state, pattern + hsp->trie->depth - top);
		}
		if (state->first == 0) {
			// 到达叶节点，遍历叶节点的兄弟节点分支
			--top; // 叶节点出栈
			state_id = state->next;
			while (top != -1 && state_id == 0) {
				state_id = states[dfs[top]].next;
				--top;
			}
		} else { 
			// 非叶节点，继续插入子节点
			state_id = state->first;
		}
	}
}

void horspool_trie_search(const Horspool *hsp, const char *s, int slen, match_result_t *result) {
	Trie *trie = hsp->trie;
	for (int i = hsp->min_len - 1, shift = 0; i < slen; i += shift) {
		for (int j = i, state_id = 0; j >= 0; j--) {
			state_id = trie_get_trans(trie, state_id, s[j]);
			if (state_id == -1) {
				break;
			}
			TrieState *state = &trie->states[state_id];
			if (state->is_fin) {
				match_result_append(result, i - j + 1, j);
			}
		}
		int hash = horspool_hash(s + i - hsp->block_size + 1, hsp->block_size, hsp->base);
		shift = hsp->shift[hash];
	}
}

/**
 * @brief 预处理，生成失配移动距离表
 * 
 * @param shift 移动距离表
 * @param p     模式串
 * @param plen  模式串长度
 */
static void build_shift(int shift[], const char *p, int plen) {
	for (int i = 0; i < CHARSET_SIZE; i++)
		shift[i] = plen;
	for (int i = 0; i < plen - 1; i++) {
		shift[p[i]] = plen - 1 - i;
	}
}

void horspool_search(const char *s, const char *p, int slen, int plen) {
	int shift[CHARSET_SIZE];
	build_shift(shift, p, plen);
	int i = 0; // 窗口位置
	int j = 0; // 窗口内字符位置
	while (i <= slen - plen) {
		for (j = plen - 1; j >= 0 && s[i + j] == p[j]; j--);
		if (j == -1) {
			printf("%d ", i);
		}
		i += shift[s[i + plen - 1]];
	}
}