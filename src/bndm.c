#include <stdlib.h>
#include <string.h>
#include "bndm.h"
#include "xssm.h"

BndmNFA* bndm_nfa_create() {
	BndmNFA *nfa = (BndmNFA *)malloc(sizeof(BndmNFA));
	memset(nfa, 0, sizeof(BndmNFA));
	nfa->min_len = INT32_MAX;
	bit_array_reset(&nfa->int_mask);
	bit_array_reset(&nfa->fin_mask);
	for (int i = 0; i < CHARSET_SIZE; i++) {
		bit_array_reset(&nfa->mask[i]);
	}
	return nfa;
}

BndmNFA* bndm_nfa_create_ex(const char **patterns, int pnum) {
	BndmNFA *nfa = bndm_nfa_create();
	for (int i = 0; i < pnum; i++) {
		bndm_nfa_insert(nfa, patterns[i], strlen(patterns[i]));
	}
	bndm_nfa_build(nfa);
	return nfa;
}

void bndm_nfa_destroy(BndmNFA *nfa) {
	free(nfa);
}

int bndm_nfa_insert(BndmNFA *nfa, const char *p, int plen) {
	if (plen > BNDM_MAX_PATTERN_LEN || nfa->pnum >= BNDM_MAX_PATTERN_NUM) {
		return -1;
	}
	int min_len = nfa->min_len;
	if (min_len > plen) {
		min_len = plen;
	}
	if (min_len * (nfa->pnum + 1) > BIT_ARRAY_MAX_BITS) {
		return -1;
	}
	nfa->min_len = min_len;
	nfa->patterns[nfa->pnum].len = plen;
	strncpy(nfa->patterns[nfa->pnum].str, p, BNDM_MAX_PATTERN_LEN);
	++nfa->pnum;
	return 0;
}

void bndm_nfa_build(BndmNFA *nfa) {
	int pos = 0;
	const _bndm_pattern_t *pattern = NULL;
	for (int i = 0; i < nfa->pnum; i++) {
		pattern = &nfa->patterns[i];
		pos = nfa->min_len * i;
		bit_array_set(&nfa->fin_mask, pos + nfa->min_len - 1);
		for (int j = 0; j < nfa->min_len; j++) {
			bit_array_set(&nfa->int_mask, pos + j);
			bit_array_set(&nfa->mask[pattern->str[pattern->len - 1 - j]], pos + j);
		}
	}
}

void bndm_nfa_search(const BndmNFA *nfa, const char *s, int slen, match_result_t *result) {
	bit_array_t status;
	bit_array_t fin_status;
	bit_array_reset(&status);
	bit_array_reset(&fin_status);
	for (int i = 0, shift = 0; i <= slen - nfa->min_len; i += shift) {
		shift = nfa->min_len;
		bit_array_copy(&status, &nfa->int_mask);
		for (int j = nfa->min_len - 1; j >= 0; j--) {
			bit_array_and(&status, &nfa->mask[s[i + j]]);
			if (bit_array_empty(&status)) {
				break;
			}
			bit_array_copy(&fin_status, &status);
			bit_array_and(&fin_status, &nfa->fin_mask);
			if (!bit_array_empty(&fin_status)) {
				if (j != 0) {
					shift = j;
				} else {
					int pos = 0;
					while ((pos = bit_array_pop(&fin_status)) != -1) {
						const _bndm_pattern_t *pattern = &nfa->patterns[pos / nfa->min_len];
						int start_pos = i + nfa->min_len - pattern->len;
						if (start_pos >= 0 && (memcmp(pattern->str, s + start_pos, pattern->len - nfa->min_len) == 0)) {
							match_result_append(result, pattern->len, start_pos);
						}
					}
				}
			}
			bit_array_lshift(&status);
		}
	}
}

/**
 * @brief 生成NFA位运算掩码表
 * 
 * @param p    模式串
 * @param plen 模式串长度
 * @param mask 掩码表
 */
static void build_mask(const char *p, int plen, int mask[]) {
	int i,j;
	memset(mask, 0, CHARSET_SIZE * sizeof(int));
	for (i = plen - 1, j = 1; i >= 0; i--) {
		mask[p[i]] |= j;
		j = j << 1;
	}
}

void bndm_search(const char* s, const char* p, int slen, int plen) {
	int mask[CHARSET_SIZE]; // 字符掩码
	build_mask(p, plen, mask);
	int pos = 0;    // 窗口位置
	int target = 1 << (plen - 1);
	while (pos <= slen - plen) {
		int j = plen - 1; // 窗口内字符位置
		int shift = plen;
		int status = (1 << plen) - 1;
		// 逆向扫描当前窗口
		while (status != 0) {
			// s[pos+j...pos+plen-1]在模式串中是否出现
			status &= mask[s[pos + j]];
			if (status & target) {
				// 模式串前缀与已匹配串后缀一致
				if (j > 0) {
					shift = j;
				} else {
					printf("%d ", pos);
				}
			}
			status <<= 1;
			j = j - 1;
		}
		pos = pos + shift;
	}
}
