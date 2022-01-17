#include "xssm.h"
#include "trie.h"

static Trie* create_fa(const char *p, int plen) {
	const char *patterns[] = {p};
	Trie* trie = trie_create_ex(patterns, 1, STTABLE_TYPE_ARRAY);
	// 初始状态要满足FA的要求
	memset(trie->sttbl->ast.stt, 0, sizeof(int) * CHARSET_SIZE);
	for (int i = 0; i < plen; i++) {
		int state_id = trie_get_trans(trie, i, p[i]);
		trie_set_trans(trie, i, i + 1, p[i]);
		sttable_copy(trie->sttbl, state_id, i + 1);
	}
	return trie;
}

void fam_search(const char *s, const char *p, int slen, int plen) {	
	Trie* trie = create_fa(p, plen);
	int state_id = 0;
	for (int i = 0; i < slen; i++) {
		state_id = trie_get_trans(trie, state_id, s[i]);
		if (trie->states[state_id].is_fin) {
			printf("%d ", i - plen + 1);
		}
	}
}