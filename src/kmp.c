#include "xssm.h"

/**
 * @brief 生成失配转移表
 * 
 * @param p    模式串
 * @param next 转移表
 * @param plen 模式串长度
 */
static void build_next(const char *p, int next[], int plen);

/**
 * @brief 生成更高效的失配转移表
 * 
 * @param p    模式串
 * @param next 转移表
 * @param plen 模式串长度
 */
static void build_next_adv(const char *p, int next[], int plen);

void kmp_search(const char *s, const char *p, int slen, int plen) {
	int i = 0; // 文本串索引
	int j = 0; // 模式传索引
	int next[plen + 1];
	memset(next, 0, sizeof(next));
	build_next(p, next, plen);
	while (i < slen) {
		while (i < slen && j < plen) {
			if (j == -1 || s[i] == p[j]) {
				i++;
				j++;
			} else {
				j = next[j];
			}
		}
		if (j == plen) {
			printf("%d ", i - j);
			j = next[j];
		}
	}
}

static void build_next(const char *p, int next[], int plen) {
	next[0] = -1;
	int k = -1;
	int j = 0;
	while (j < plen) {
		if (k == -1 || p[j] == p[k]) {
			k++;
			j++;
			next[j] = k;
		} else {
			k = next[k];
		}
	}
}

static void build_next_adv(const char *p, int next[], int plen) {
	next[0] = -1;
	int k = -1;
	int j = 0;
	while (j < plen) {
		if (k == -1 || p[j] == p[k]) {
			k++;
			j++;
			if (p[j] != p[k]) {
				next[j] = k;
				k = next[k];
			}
			else {
				next[j] = next[k];
			}
		}
		else {
			k = next[k];
		}
	}
}
