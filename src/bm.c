#include "xssm.h"

static void pre_bmbc(int bmbc[], const char* p, int plen) {
	memset(bmbc, -1, CHARSET_SIZE * 4);
	for (int i = 0; i < plen - 1; i++) {
		bmbc[p[i]] = i;
	}
}

static void suffix(int suff[], const char* p, int plen) {
	memset(suff, 0, plen * 4);
	suff[plen - 1] = plen;
	int j = 0;
	for (int i = plen - 2; i >= 0; i--) {
		for (j = i; j >= 0 && p[j] == p[plen - 1 - i + j]; j--);
		suff[i] = i - j;
	}
}

static void pre_bmgs(int bmgs[], const char* p, int plen) {
	int suff[plen];
	suffix(suff, p, plen);
	for (int i = 0; i < plen; i++)
		bmgs[i] = plen;
	//模式串前缀和已匹配后缀部分match
	int j = 0;
	for (int i = plen - 2; i >= 0; i--) {
		//好后缀是前缀
		if(suff[i] == i + 1) {
			for (; j < plen - i - 1; j++) {
				bmgs[j] = plen - 1 - i;
			}
		}
	}
	//模式串存在与已匹配后缀match的子串
	for (int i = 0; i < plen - 1; i++) {
		bmgs[plen - 1 - suff[i]] = plen - 1 - i;
	}
}

void bm_search(const char* s, const char* p, int slen, int plen) {
	int bmbc[CHARSET_SIZE];
	int bmgs[plen];
	pre_bmbc(bmbc, p, plen);
	pre_bmgs(bmgs, p, plen);
	int pos = 0; // 窗口位置
	int i = 0;   // 窗口内字符位置
	int shift;   // 窗口最大安全移动距离
	while (pos <= slen - plen) {
		for (i = plen - 1; i >= 0 && s[pos + i] == p[i]; i--);
		if (i == -1) {
			printf("%d ", pos);
			shift = bmgs[0];
		} else {
			shift = i - bmbc[s[pos + i]];
			if (shift < bmgs[i]) {
				shift = bmgs[i];
			}
		}
		pos += shift;
	}
}