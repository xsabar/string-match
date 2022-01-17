#include "xssm.h"
#include "trie.h"

static Trie* build_oracle(const char *p, int plen) {
    char reverse[plen + 1];
    for (int i = plen - 1; i >= 0; i--) {
        reverse[i] = p[plen - 1 - i];
    }
    reverse[plen] = '\0';
    const char* patterns[] = {reverse};
    Trie* trie = trie_create_ex(patterns, 1, STTABLE_TYPE_HASHT);
    int supply[plen + 1]; // 供给数组，对应状态数
    memset(supply, 0, sizeof(supply));
    supply[0] = -1;
    for (int i = 1; i < plen; i++) {
        int sp = i;
        int state_id = 0;
        while ((sp = supply[sp]) != -1) {
            state_id = trie_get_trans(trie, sp, reverse[i]);
            if (state_id == -1) {
                trie_set_trans(trie, sp, i + 1, reverse[i]);
            } else {
                supply[i + 1] = state_id;
                break;
            }
        }
    }
    return trie;
}

void bom_search(const char *s, const char *p, int slen, int plen) {
    Trie* trie = build_oracle(p, plen);
    int i = 0; // 窗口位置
    while (i <= slen - plen) {
        int state_id = 0;
        int j = plen - 1;
        while ((state_id = trie_get_trans(trie, state_id, s[i + j])) != -1) {
            if (j == 0) {
                printf("%d ", i);
                break;
            }
            j--;
        }
        i = i + j + 1;
    }
}