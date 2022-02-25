#include "trie.h"

static int sttable_list_get(struct _sttable_list_s *tbl, int id, char c) {
    int child = tbl->trie->states[id].first;
    TrieState *state = NULL;
    while (child != 0) {
        state = &tbl->trie->states[child];
        if (state->c == c) {
            return child;
        }
        child = state->next;
    }
    return -1;
}