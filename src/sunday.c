#include "xssm.h"

/**
 * @brief 预处理，生成失配移动距离表
 * 
 * @param shift 移动距离表
 * @param p     模式串
 * @param plen  模式串长度
 */
static void build_shift(int shift[], const char *p, int plen) {
	for (int i = 0; i < CHARSET_SIZE; i++)
		shift[i] = plen + 1;
	for (int i = 0; i < plen; i++) {
		shift[p[i]] = plen - i;
	}
}

void sunday_search(const char *s, const char *p, int slen, int plen) {
	int shift[CHARSET_SIZE];
	build_shift(shift, p, plen);
	int i = 0; //窗口位置
	int j = 0; //窗口内字符位置
	while(i <= slen - plen) {
		for(j = 0; j < plen && s[i + j] == p[j]; j++);
		if(j == plen) {
			printf("%d ", i);
		}
		//可能会越界访问s[slen]，不过并不会影响结果
		i += shift[s[i + plen]];
	}
}