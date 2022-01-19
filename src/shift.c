#include <stdlib.h>
#include <string.h>
#include "shift.h"
#include "xssm.h"

ShiftNFA* shift_nfa_create() {
	ShiftNFA *snfa = (ShiftNFA *)malloc(sizeof(ShiftNFA));
	memset(snfa, 0, sizeof(ShiftNFA));
	bit_array_reset(&snfa->int_mask);
	bit_array_reset(&snfa->fin_mask);
	for (int i = 0; i < CHARSET_SIZE; i++) {
		bit_array_reset(&snfa->mask[i]);
	}
	return snfa;
}

ShiftNFA* shift_nfa_create_ex(const char **patterns, int pnum) {
	ShiftNFA *snfa = shift_nfa_create();
	for (int i = 0; i < pnum; i++) {
		shift_nfa_insert(snfa, patterns[i], strlen(patterns[i]));
	}
	shift_nfa_build(snfa);
	return snfa;
}

void shift_nfa_destroy(ShiftNFA *snfa) {
	free(snfa);
}

int shift_nfa_insert(ShiftNFA *snfa, const char *p, int plen) {
	if (plen > SHIFT_MAX_PATTERN_LEN || snfa->pnum >= SHIFT_MAX_PATTERN_NUM) {
		return -1;
	}
	int max_len = snfa->max_len;
	if (max_len < plen) {
		max_len = plen;
	}
	if (max_len * (snfa->pnum + 1) > BIT_ARRAY_MAX_BITS) {
		return -1;
	}
	snfa->max_len = max_len;
	snfa->patterns[snfa->pnum].len = plen;
	strncpy(snfa->patterns[snfa->pnum].str, p, SHIFT_MAX_PATTERN_LEN);
	++snfa->pnum;
	return 0;
}

void shift_nfa_build(ShiftNFA *snfa) {
	int pos = 0;
	const _snfa_pattern_t *pattern = NULL;
	for (int i = 0; i < snfa->pnum; i++) {
		pattern = &snfa->patterns[i];
		pos = snfa->max_len * i;
		bit_array_set(&snfa->int_mask, pos);
		bit_array_set(&snfa->fin_mask, pos + pattern->len - 1);
		for (int j = 0; j < pattern->len; j++) {
			bit_array_set(&snfa->mask[pattern->str[j]], pos + j);
		}
	}
}

void shift_nfa_search(const ShiftNFA *snfa, const char *s, int slen, match_result_t* result) {
	bit_array_t status;
	bit_array_t fin_status;
	bit_array_reset(&status);
	bit_array_reset(&fin_status);
	int pos = 0;
	for (int i = 0; i < slen; i++) {
		bit_array_lshift(&status);
		bit_array_or(&status, &snfa->int_mask);
		bit_array_and(&status, &snfa->mask[s[i]]);
		bit_array_copy(&fin_status, &status);
		bit_array_and(&fin_status, &snfa->fin_mask);
		while ((pos = bit_array_pop(&fin_status)) != -1) {
			const _snfa_pattern_t *pattern = &snfa->patterns[pos / snfa->max_len];
			match_result_append(result, pattern->len, i - pattern->len + 1);
		}
	}
}

void shift_and_search(const char* s, const char* p, int slen, int plen) {
	//构建字符掩码表
	//字符掩码表示字符在模式串中每个位置是否出现，0-未出现，1-出现
	uint64_t mask[CHARSET_SIZE];
	for (int i = 0; i < CHARSET_SIZE; i++) {
		mask[i] = 0;
	}
	for (int i = 0; i < plen; i++) {
		mask[p[i]] |= 1 << i;
	}
	//当前状态，每个二进制位用于表示前缀p[0-i]是否匹配
	uint64_t status = 0;
	uint64_t target = 1 << (plen - 1);
	for (int i = 0; i < slen; i++) {
		status = (status << 1 | 1) & mask[s[i]];
		if ((status & target) == target) {
			printf("%d ", i - plen + 1);
		}
	}
}

void shift_or_search(const char* s, const char* p, int slen, int plen) {
	uint64_t mask[CHARSET_SIZE];
	for (int i = 0; i < CHARSET_SIZE; i++) {
		mask[i] = ~0;
	}
	for (int i = 0; i < plen; i++) {
		mask[p[i]] &= ~(1 << i);
	}
	uint64_t target = ~(1 << (plen - 1));
	uint64_t status = ~0;
	for (int i = 0; i < slen; i++) {
		status = (status << 1) | mask[s[i]];
		if ((status | target) == target) {
			printf("%d ", i - plen + 1);
		}
	}
}